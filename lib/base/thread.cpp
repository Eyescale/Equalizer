
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "base.h"
#include "debug.h"
#include "lock.h"
#include "log.h"
#include "rng.h"
#include "scopedMutex.h"
#include "executionListener.h"

#include <eq/base/lock.h>

#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <algorithm>

using namespace std;

/* 
 * EQ_WIN32_SDP_JOIN_WAR: When using SDP connections on Win32, the join() of the
 * receiver thread blocks indefinitely, even after the thread has exited. This
 * workaround uses a monitor to implement the join functionality independently
 * of pthreads.
 */

// Experimental Win32 thread pinning
#ifdef WIN32
//#  define EQ_WIN32_THREAD_AFFINITY
#endif

namespace eq
{
namespace base
{
namespace
{
static Lock& _listenerLock()
{
    static Lock lock;
    return lock;
}

static std::vector<ExecutionListener*>& _listeners()
{
    static std::vector<ExecutionListener*> listeners;
    return listeners;
}
}

static pthread_key_t _createCleanupKey();
void                 _notifyStopping( void* arg );

static pthread_key_t _cleanupKey = _createCleanupKey();

pthread_key_t _createCleanupKey()
{
    const int error = pthread_key_create( &_cleanupKey, _notifyStopping );
    if( error )
    {
        EQERROR
            << "Can't create thread-specific key for thread cleanup handler: " 
            << strerror( error ) << std::endl;
        EQASSERT( !error );
    }
    return _cleanupKey;
}

class ThreadPrivate
{
public:
	pthread_t threadID;
};

Thread::Thread()
        : _data( new ThreadPrivate )
        , _state( STATE_STOPPED )
#ifdef EQ_WIN32_SDP_JOIN_WAR
        , _retVal( 0 )
#endif
{
    memset( &_data->threadID, 0, sizeof( pthread_t ));
    _syncChild.set();
}

Thread::~Thread()
{
	delete _data;
	_data = 0;
}

void* Thread::runChild( void* arg )
{
    Thread* thread = static_cast<Thread*>(arg);
    thread->_runChild();
    return 0; // not reached
}

void Thread::_runChild()
{
#ifdef EQ_WIN32_SDP_JOIN_WAR
    _running = true;
#endif
    pinCurrentThread();

    _data->threadID = pthread_self(); // XXX remove, set during create already?

    if( !init( ))
    {
        EQWARN << "Thread failed to initialize" << endl;
        _state = STATE_STOPPED;
        _syncChild.unset();
        pthread_exit( 0 );
    }

    _state    = STATE_RUNNING;
    EQINFO << "Thread successfully initialized" << endl;
    pthread_setspecific( _cleanupKey, this ); // install cleanup handler
    _notifyStarted();
    _syncChild.unset(); // sync w/ parent

    void* result = run();
    EQINFO << "Thread finished with result " << result << endl;
    this->exit( result );

    EQUNREACHABLE;
}

void Thread::_notifyStarted()
{
    // make copy of vector so that listeners can add/remove listeners.
    _listenerLock().set();
    const std::vector<ExecutionListener*> listeners = _listeners();
    _listenerLock().unset();

    EQVERB << "Calling " << listeners.size() << " thread started listeners"
           << endl;
    for( vector<ExecutionListener*>::const_iterator i = listeners.begin();
         i != listeners.end(); ++i )
        
        (*i)->notifyExecutionStarted();
}

void _notifyStopping( void* )
{ 
    Thread::_notifyStopping();
}

void Thread::_notifyStopping()
{
    pthread_setspecific( _cleanupKey, 0 );

    // make copy of vector so that listeners can add/remove listeners.
    _listenerLock().set();
    std::vector< ExecutionListener* > listeners = _listeners();
    _listenerLock().unset();

    // Call them in reverse order so that symmetry is kept
    for( vector< ExecutionListener* >::reverse_iterator i = listeners.rbegin();
         i != listeners.rend(); ++i )
        
        (*i)->notifyExecutionStopping();
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
        const int error = pthread_create( &_data->threadID, &attributes,
                                          runChild, this );

        if( error == 0 ) // succeeded
        {
            EQVERB << "Created pthread " << this << endl;
            break;
        }
        if( error != EAGAIN || nTries==0 )
        {
            EQWARN << "Could not create thread: " << strerror( error )
                   << endl;
            return false;
        }
    }

    _syncChild.set(); // sync with child's entry func
    _state = STATE_RUNNING;
    return true;
}

void Thread::exit( void* retVal )
{
    EQASSERTINFO( isCurrent(), "Thread::exit not called from child thread" );

    EQINFO << "Exiting thread" << endl;
    _state = STATE_STOPPING;

#ifdef EQ_WIN32_SDP_JOIN_WAR
    _running = false;
    _retVal  = retVal;
#endif

    pthread_exit( (void*)retVal );
    EQUNREACHABLE;
}

void Thread::cancel()
{
    EQASSERTINFO( !isCurrent(), "Thread::cancel called from child thread" );

    EQINFO << "Cancelling thread" << endl;
    _state = STATE_STOPPING;

    pthread_cancel( _data->threadID );
    EQUNREACHABLE;
}

bool Thread::join( void** retVal )
{
    if( _state == STATE_STOPPED )
        return false;
    if( isCurrent( )) // can't join self
        return false;

    EQVERB << "Joining thread" << endl;
#ifdef EQ_WIN32_SDP_JOIN_WAR
    _running.waitEQ( false );
#else
    void *_retVal;
    const int error = pthread_join( _data->threadID, &_retVal);
    if( error != 0 )
    {
        EQWARN << "Error joining thread: " << strerror(error) << endl;
        return false;
    }
#endif

    _state = STATE_STOPPED;
    if( retVal )
        *retVal = _retVal;
    return true;
}

bool Thread::isCurrent() const
{
    return pthread_equal( pthread_self(), _data->threadID );
}

size_t Thread::getSelfThreadID()
{
#ifdef PTW32_VERSION
    return reinterpret_cast< size_t >( pthread_self().p );
#else
    return ( size_t )( pthread_self( ));
#endif
}

void Thread::addListener( ExecutionListener* listener )
{
    ScopedMutex mutex( _listenerLock() );
    _listeners().push_back( listener );
}

bool Thread::removeListener( ExecutionListener* listener )
{
    ScopedMutex mutex( _listenerLock() );

    vector< ExecutionListener* >::iterator i = find( _listeners().begin(),
                                                     _listeners().end(),
                                                     listener );
    if( i == _listeners().end( ))
        return false;

    _listeners().erase( i );
    return true;
}

void Thread::removeAllListeners()
{
    _listenerLock().set();

    EQINFO << _listeners().size() << " thread listeners active" << endl;
    for( vector<ExecutionListener*>::const_iterator i = _listeners().begin();
         i != _listeners().end(); ++i )

        EQINFO << "    " << typeid( **i ).name() << endl;

    _listenerLock().unset();
    
    _notifyStopping();

    _listenerLock().set();
    _listeners().clear();
    _listenerLock().unset();
}


void Thread::pinCurrentThread()
{
#ifdef EQ_WIN32_THREAD_AFFINITY
    static Lock lock;
    ScopedMutex mutex( lock );

    static DWORD_PTR processMask = 0;
    static DWORD_PTR processor   = 0;
    if( processMask == 0 )
    {
        // Get available processors
        DWORD_PTR systemMask;
        if( GetProcessAffinityMask( GetCurrentProcess(), &processMask, 
            &systemMask ) == 0 )
        {
            EQWARN << "Can't get usable processor mask" << endl;
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
        EQINFO << nProcessors << " available processors" << endl;

        unsigned chance = RNG().get< unsigned >();
        processor = 1 << (chance % nProcessors);
        EQINFO << "Starting with processor " << processor << endl;
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
                EQWARN << "Can't set thread processor" << endl;
            EQINFO << "Pinned thread to processor 0x" << hex << processor << dec
                   << endl;
            return;
        }
    }
#endif
}

std::ostream& operator << ( std::ostream& os, const Thread* thread )
{
#ifdef PTW32_VERSION
    os << "Thread " << thread->_data->threadID.p;
#else
    os << "Thread " << thread->_data->threadID;
#endif
	os << " state " 
		<< ( thread->_state == Thread::STATE_STOPPED ? "stopped" :
			thread->_state == Thread::STATE_STARTING ? "starting" :
			thread->_state == Thread::STATE_RUNNING ? "running" :
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
