
/* Copyright (c) 2005-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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
#pragma message ("Thread affinity is not supported by WIN32")
#endif

#ifdef Linux
#  include <sys/prctl.h>
#  include <sched.h>
#endif

#ifdef CO_USE_HWLOC
#  include <hwloc.h>
#endif

namespace co
{
namespace base
{

struct ThreadIDPrivate
{
    pthread_t pthread;
};

void _notifyStopping( void* arg );

Thread::Thread()
        : _state( STATE_STOPPED )
{
}

Thread::Thread( const Thread& from )
        : _state( STATE_STOPPED )
{
}

Thread::~Thread()
{
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
    _id._data->pthread = pthread_self();


    if( !init( ))
    {
        EQWARN << "Thread " << className( this ) << " failed to initialize"
               << std::endl;
        _state = STATE_STOPPED;
        pthread_exit( 0 );
        EQUNREACHABLE;
    }

    _state = STATE_RUNNING;
    EQINFO << "Thread " << className( this ) << " successfully initialized"
           << std::endl;

    run();
    EQINFO << "Thread " << className( this ) << " finished" << std::endl;
    this->exit();

    EQUNREACHABLE;
}

bool Thread::start()
{
    if( _state != STATE_STOPPED )
        return false;

    _state = STATE_STARTING;

    pthread_attr_t attributes;
    pthread_attr_init( &attributes );
    pthread_attr_setscope( &attributes, PTHREAD_SCOPE_SYSTEM );

    int nTries = 10;
    while( nTries-- )
    {
        const int error = pthread_create( &_id._data->pthread, &attributes,
                                          runChild, this );

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
    pthread_detach( _id._data->pthread );
    _state.waitNE( STATE_STARTING );
    return (_state != STATE_STOPPED);
}

void Thread::exit()
{
    EQASSERTINFO( isCurrent(), "Thread::exit not called from child thread" );
    EQINFO << "Exiting thread " << className( this ) << std::endl;
    Log::instance().forceFlush();
    Log::instance().exit();

    _state = STATE_STOPPING;
    pthread_exit( 0 );
    EQUNREACHABLE;
}

void Thread::cancel()
{
    EQASSERTINFO( !isCurrent(), "Thread::cancel called from child thread" );

    EQINFO << "Canceling thread " << className( this ) << std::endl;
    _state = STATE_STOPPING;

    pthread_cancel( _id._data->pthread );
}

bool Thread::join()
{
    if( _state == STATE_STOPPED )
        return false;
    if( isCurrent( )) // can't join self
        return false;

    _state.waitNE( STATE_RUNNING );
    _state = STATE_STOPPED;

    EQINFO << "Joined thread " << className( this ) << std::endl;
    return true;
}

bool Thread::isCurrent() const
{
    return pthread_equal( pthread_self(), _id._data->pthread );
}

ThreadID Thread::getSelfThreadID()
{
    ThreadID threadID;
    threadID._data->pthread = pthread_self();
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
        RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
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

void Thread::setAffinity(const int32_t affIndex)
{
	EQINFO << "Setting thread affinity " << std::endl;

#ifdef CO_USE_HWLOC

	hwloc_topology_t topology; // HW topology
	hwloc_topology_init(&topology); // Allocate & initialize the topology
	hwloc_topology_load(topology); 	// Perform HW topology detection

	cpu_set_t cpuMask; // CPU mask
	CPU_ZERO(&cpuMask); // Sets the mask to 0's

	/* Sets the affinity to a specific CPU or "socket" with all of its cores */
	if( affIndex <= CPU )
	{
		const int cpuIndex = affIndex - CPU;

		// Make sure that this cpuID is existing in the topology
		if ( hwloc_get_obj_by_type(topology, HWLOC_OBJ_SOCKET, cpuIndex) != 0 )
		{
			// Getting the CPU #cpuIndex (subtree node)
			const hwloc_obj_t cpuObj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_SOCKET, cpuIndex);

			// Get the number of PUs shipped on
			const int numCores = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_PU);

			// Core object
			hwloc_obj_t coreObj;

			/* Now, we are ready for the loop */
			for (int iCore = 0; iCore < numCores; iCore++)
			{
				// Get the PUs inside iSocket according to their index "pointed by socketObj"
				coreObj = hwloc_get_obj_below_by_type(topology, HWLOC_OBJ_SOCKET, cpuIndex, HWLOC_OBJ_PU, iCore);

				// If this core "PU" is under this CPU "socket" in this topology
				if( hwloc_obj_is_in_subtree(topology, coreObj, cpuObj) )
				{
					// Add core "PU" to the mask set
					CPU_SET(iCore, &cpuMask);
				}
			}
			EQINFO << "Thread is set to CPU #" << cpuIndex << std::endl;
		}
		else
		{
			// Warning if the CPU is not in the topology tree
			EQWARN << "CPU ID " << cpuIndex << " is not existing in the topology \n" << std::endl;
		}
	}
	/* Sets the affinity to a specific core or "PU" of your machine */
	else if (affIndex >= CORE)
	{
		const int32_t coreIndex = (affIndex - CORE);
		CPU_SET(coreIndex, &cpuMask);
		EQINFO << "Thread is set to core #" << coreIndex << std::endl;
	}
	else {
		EQWARN << "Thread affinity is not set" << std::endl;
	}

#  ifdef Linux
	// Sets the affinity according to the generated cpuMask
	sched_setaffinity(0, sizeof(cpuMask), &cpuMask);
#  else
	EQWARN << "Thread affinity is not supported for this platform" << std::endl;
	EQWARN << "Thread affinity is not set " << std::endl;
#  endif
#else
	EQWARN << "HWLoc library is not supported " << std::endl;
	EQWARN << "Thread affinity is not set " << std::endl;
#endif
}

std::ostream& operator << ( std::ostream& os, const Thread* thread )
{
    os << "Thread " << thread->_id << " state " 
       << ( thread->_state == Thread::STATE_STOPPED  ? "stopped"  :
            thread->_state == Thread::STATE_STARTING ? "starting" :
            thread->_state == Thread::STATE_RUNNING  ? "running"  :
            thread->_state == Thread::STATE_STOPPING ? "stopping" : "unknown" );

#ifdef PTW32_VERSION
    os << " called from " << pthread_self().p;
#else
    os << " called from " << pthread_self();
#endif

    return os;
}
}
}
