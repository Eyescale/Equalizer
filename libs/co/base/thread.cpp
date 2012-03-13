
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com> 
 *               2012, Marwan Abdellah <marwan.abdellah@epfl.ch>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "thread.h"

#include "os.h"
#include "debug.h"
#include "lock.h"
#include "log.h"
#include "monitor.h"
#include "rng.h"
#include "scopedMutex.h"
#include "sleep.h"

#include <co/base/lock.h>

#include <errno.h>
#include <pthread.h>
#include <algorithm>

// Experimental Win32 thread pinning
#ifdef _WIN32
//#  define EQ_WIN32_THREAD_AFFINITY
#  pragma message ("Thread affinity  not supported on WIN32")
#endif

#ifdef Linux
#  include <sys/prctl.h>
#endif

#ifdef CO_USE_HWLOC
#  include <hwloc.h>
#endif

namespace co
{
namespace base
{
namespace
{
/** The current state of a thread. */
enum ThreadState
{
    STATE_STOPPED,
    STATE_STARTING, // start() in progress
    STATE_RUNNING,
    STATE_STOPPING  // child no longer active, join() not yet called
};
}
namespace detail
{
class ThreadID
{
public:
    pthread_t pthread;
};

class Thread
{
public:
    Thread() : state( STATE_STOPPED ) {}

    base::ThreadID id;
    Monitor< ThreadState > state;
};
}

void _notifyStopping( void* arg );

Thread::Thread()
        : _impl( new detail::Thread )
{
}

Thread::Thread( const Thread& from )
        : _impl( new detail::Thread )
{
}

Thread::~Thread()
{
    delete _impl;
}

bool Thread::isStopped() const
{
    return ( _impl->state == STATE_STOPPED );
}

bool Thread::isRunning() const
{
    return ( _impl->state == STATE_RUNNING );
}

void* Thread::runChild( void* arg )
{
    Thread* thread = static_cast<Thread*>(arg);
    thread->_runChild();
    return 0; // not reached
}

void Thread::_runChild()
{
    setName( className( this ));
    pinCurrentThread();
    _impl->id._impl->pthread = pthread_self();

    if( !init( ))
    {
        EQWARN << "Thread " << className( this ) << " failed to initialize"
               << std::endl;
        _impl->state = STATE_STOPPED;
        pthread_exit( 0 );
        EQUNREACHABLE;
    }

    _impl->state = STATE_RUNNING;
    EQINFO << "Thread " << className( this ) << " successfully initialized"
           << std::endl;

    run();
    EQINFO << "Thread " << className( this ) << " finished" << std::endl;
    this->exit();

    EQUNREACHABLE;
}

bool Thread::start()
{
    if( _impl->state != STATE_STOPPED )
        return false;

    _impl->state = STATE_STARTING;

    pthread_attr_t attributes;
    pthread_attr_init( &attributes );
    pthread_attr_setscope( &attributes, PTHREAD_SCOPE_SYSTEM );

    int nTries = 10;
    while( nTries-- )
    {
        const int error = pthread_create( &_impl->id._impl->pthread,
                                          &attributes, runChild, this );
        if( error == 0 ) // succeeded
        {
            EQVERB << "Created pthread " << this << std::endl;
            break;
        }
        if( error != EAGAIN || nTries == 0 )
        {
            EQWARN << "Could not create thread: " << strerror( error )
                   << std::endl;
            return false;
        }
        sleep( 1 ); // Give EAGAIN some time to recover
    }

    // avoid memleak, we don't use pthread_join
    pthread_detach( _impl->id._impl->pthread );
    _impl->state.waitNE( STATE_STARTING );
    return (_impl->state != STATE_STOPPED);
}

void Thread::exit()
{
    EQASSERTINFO( isCurrent(), "Thread::exit not called from child thread" );
    EQINFO << "Exiting thread " << className( this ) << std::endl;
    Log::instance().forceFlush();
    Log::instance().exit();

    _impl->state = STATE_STOPPING;
    pthread_exit( 0 );
    EQUNREACHABLE;
}

void Thread::cancel()
{
    EQASSERTINFO( !isCurrent(), "Thread::cancel called from child thread" );

    EQINFO << "Canceling thread " << className( this ) << std::endl;
    _impl->state = STATE_STOPPING;

    pthread_cancel( _impl->id._impl->pthread );
}

bool Thread::join()
{
    if( _impl->state == STATE_STOPPED )
        return false;
    if( isCurrent( )) // can't join self
        return false;

    _impl->state.waitNE( STATE_RUNNING );
    _impl->state = STATE_STOPPED;

    EQINFO << "Joined thread " << className( this ) << std::endl;
    return true;
}

bool Thread::isCurrent() const
{
    return pthread_equal( pthread_self(), _impl->id._impl->pthread );
}

ThreadID Thread::getSelfThreadID()
{
    ThreadID threadID;
    threadID._impl->pthread = pthread_self();
    return threadID;
}

void Thread::yield()
{
#ifdef _MSC_VER
    ::Sleep( 0 ); // sleeps thread
    // or ::SwitchToThread() ? // switches to another waiting thread, if exists
#elif defined (Darwin)
    ::pthread_yield_np();
#else
    ::sched_yield();
#endif
}

void Thread::pinCurrentThread()
{
#ifdef EQ_WIN32_THREAD_AFFINITY
    static Lock lock;
    ScopedMutex<> mutex( lock );

    static DWORD_PTR processMask = 0;
    static DWORD_PTR processor   = 0;
    if( processMask == 0 )
    {
        // Get available processors
        DWORD_PTR systemMask;
        if( GetProcessAffinityMask( GetCurrentProcess(), &processMask, 
            &systemMask ) == 0 )
        {
            EQWARN << "Can't get usable processor mask" << std::endl;
            return;
        }
        EQINFO << "Available processors 0x" << hex << processMask << dec <<endl;

        // Choose random starting processor: Multiple Eq apps on the same node
        // would otherwise use the same processor for the same thread
        unsigned nProcessors = 0;
        for( DWORD_PTR i = 1; i != 0; i <<= 1 )
        {
            if( processMask & i )
                ++nProcessors;
        }
        EQINFO << nProcessors << " available processors" << std::endl;

        unsigned chance = RNG().get< unsigned >();
        processor = 1 << (chance % nProcessors);
        EQINFO << "Starting with processor " << processor << std::endl;
    }
    EQASSERT( processMask != 0 );

    while( true )
    {
        processor <<= 1;
        if( processor == 0 ) // wrap around
            processor = 1;

        if( processor & processMask ) // processor is available
        {
            if( SetThreadAffinityMask( GetCurrentThread(), processor ) == 0 )
                EQWARN << "Can't set thread processor" << std::endl;
            EQINFO << "Pinned thread to processor 0x" << hex << processor << dec
                   << std::endl;
            return;
        }
    }
#endif
}

#ifdef _WIN32
#ifndef MS_VC_EXCEPTION
#  define MS_VC_EXCEPTION 0x406D1388
#endif

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

#endif

void Thread::setName( const std::string& name )
{
    Log::instance().setThreadName( name );

#ifdef _MSC_VER
#  ifndef NDEBUG
    ::Sleep(10);

    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = name.c_str();
    info.dwThreadID = GetCurrentThreadId();
    info.dwFlags = 0;

    __try
    {
        RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR),
                        (ULONG_PTR*)&info );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
#  endif
#elif __MAC_OS_X_VERSION_MIN_REQUIRED >= 1060
    pthread_setname_np( name.c_str( ));
#elif defined(Linux)
    prctl( PR_SET_NAME, name.c_str(), 0, 0, 0 );
#else
    // Not implemented
    EQVERB << "Thread::setName( " << name << " ) not implemented" << std::endl;
#endif
}

#ifdef CO_USE_HWLOC
static hwloc_bitmap_t _getCpuSet( const int32_t affinity,
                                  hwloc_topology_t topology )
{
    hwloc_bitmap_t cpuSet = hwloc_bitmap_alloc(); // HWloc CPU set
    hwloc_bitmap_zero( cpuSet ); // Initialize to zeros

    if( affinity >= Thread::CORE )
    {
        const int32_t coreIndex = affinity - Thread::CORE;
        if( hwloc_get_obj_by_type( topology, HWLOC_OBJ_CORE, coreIndex ) == 0 )
        {
            EQWARN << "Core " << coreIndex << " does not exist in the topology"
                   << std::endl;
            return cpuSet;
        }

        // Getting the core object #coreIndex
        const hwloc_obj_t coreObj = hwloc_get_obj_by_type( topology,
                                                           HWLOC_OBJ_CORE,
                                                           coreIndex );
        // Get the CPU set associated with the specified core
        cpuSet = coreObj->allowed_cpuset;
        return cpuSet;
    }

    if( affinity >= 0 )
        return cpuSet;

    // Sets the affinity to a specific CPU or "socket"
    EQASSERT( affinity >= Thread::SOCKET && affinity < Thread::SOCKET_MAX );
    const int32_t socketIndex = affinity - Thread::SOCKET;

    if( hwloc_get_obj_by_type( topology, HWLOC_OBJ_SOCKET, socketIndex ) == 0 )
    {
        EQWARN << "Socket " << socketIndex << " does not exist in the topology"
               << std::endl;
        return cpuSet;
    }

    // Getting the CPU object #cpuIndex (subtree node)
    const hwloc_obj_t socketObj = hwloc_get_obj_by_type( topology,
                                                         HWLOC_OBJ_SOCKET,
                                                         socketIndex );
    // Get the CPU set associated with the specified socket
    cpuSet = socketObj->allowed_cpuset;
    return cpuSet;
}
#endif

void Thread::setAffinity(const int32_t affinity)
{
#ifdef CO_USE_HWLOC
    hwloc_topology_t topology;
    hwloc_topology_init( &topology ); // Allocate & initialize the topology
    hwloc_topology_load( topology );  // Perform HW topology detection
    const hwloc_bitmap_t cpuSet = _getCpuSet( affinity, topology );
    const int result = hwloc_set_cpubind( topology, cpuSet,
                                                HWLOC_CPUBIND_THREAD );
    char* cpuSetString;
    hwloc_bitmap_asprintf( &cpuSetString, cpuSet );

    if( affinityFlag == 0 )
    {
        EQINFO << "Bound thread to cpu set "  << cpuSetString << std::endl;
    }
    else
    {
        EQWARN << "Error binding thread to cpu set " << cpuSetString
               << std::endl;
    }
    ::free( cpuSetString );

    hwloc_topology_destroy(topology);

#else
    EQWARN << "Thread::setAffinity not implemented, hwloc library missing"
           << std::endl;
#endif
}

#if 0
std::ostream& operator << ( std::ostream& os, const Thread* thread )
{
    os << "Thread " << thread->_impl->id << " state " 
       << ( thread->_impl->state == Thread::STATE_STOPPED  ? "stopped"  :
            thread->_impl->state == Thread::STATE_STARTING ? "starting" :
            thread->_impl->state == Thread::STATE_RUNNING  ? "running"  :
            thread->_impl->state == Thread::STATE_STOPPING ? "stopping" :
            "unknown" );

#ifdef PTW32_VERSION
    os << " called from " << pthread_self().p;
#else
    os << " called from " << pthread_self();
#endif

    return os;
}
#endif

}
}
