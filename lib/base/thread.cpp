
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "thread.h"

#include "base.h"
#include "debug.h"
#include "lock.h"
#include "log.h"
#include "scopedMutex.h"
#include "executionListener.h"

#include <errno.h>
#include <pthread.h>

using namespace eqBase;
using namespace std;

/* 
 * EQ_WIN32_SDP_JOIN_WAR: When using SDP connections on Win32, the join() of the
 * receiver thread blocks indefinitely, even after the thread has exited. This
 * workaround uses a monitor to implement the join functionality independently
 * of phtreads.
 */

Lock                            Thread::_listenerLock;
std::vector<ExecutionListener*> Thread::_listeners;
pthread_key_t                   Thread::_cleanupKey=Thread::_createCleanupKey();

#ifdef EQ_CHECK_THREADSAFETY
static pthread_t getThreadIdZero()
{
    pthread_t id;
    memset( &id, 0, sizeof( pthread_t ));
    return id;
}

pthread_t                       eqBase::threadIdZero = getThreadIdZero();
#endif

pthread_key_t Thread::_createCleanupKey()
{
    const int error = pthread_key_create(&_cleanupKey, Thread::_notifyStopping);
    if( error )
    {
        EQERROR
            << "Can't create thread-specific key for thread cleanup handler: " 
            << strerror( error ) << std::endl;
        EQASSERT( !error );
    }
    return _cleanupKey;
}

Thread::Thread()
        : _state(STATE_STOPPED)
#ifdef EQ_WIN32_SDP_JOIN_WAR
        , _retVal( 0 )
#endif
{
    memset( &_threadID, 0, sizeof( pthread_t ));
    _syncChild.set();
}

Thread::~Thread()
{
}

void* Thread::runChild( void* arg )
{
    Thread* thread = static_cast<Thread*>(arg);
    thread->_runChild();
    return NULL; // not reached
}

void Thread::_runChild()
{
#ifdef EQ_WIN32_SDP_JOIN_WAR
    _running = true;
#endif
    _threadID = pthread_self(); // XXX remove, set during create already?

    if( !init( ))
    {
        EQINFO << "Thread failed to initialise" << endl;
        _state = STATE_STOPPED;
        _syncChild.unset();
        pthread_exit( NULL );
    }

    _state    = STATE_RUNNING;
    EQINFO << "Thread successfully initialised" << endl;
    pthread_setspecific( _cleanupKey, this ); // install cleanup handler
    _notifyStarted();
    _syncChild.unset(); // sync w/ parent

    void* result = run();
    this->exit( result );
    EQUNREACHABLE;
}

void Thread::_notifyStarted()
{
    ScopedMutex mutex( _listenerLock );

    EQINFO << "Calling " << _listeners.size() << " thread started listeners"
           << endl;
    for( vector<ExecutionListener*>::const_iterator iter = _listeners.begin();
         iter != _listeners.end(); ++iter )
        
        (*iter)->notifyExecutionStarted();
}

void Thread::_notifyStopping( void* )
{
    pthread_setspecific( _cleanupKey, NULL );

    ScopedMutex mutex( _listenerLock );
    EQINFO << "Calling " << _listeners.size() << " thread stopping listeners"
           <<endl;
    for( vector<ExecutionListener*>::const_iterator iter = _listeners.begin();
         iter != _listeners.end(); ++iter )
        
        (*iter)->notifyExecutionStopping();
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
        const int error = pthread_create( &_threadID, &attributes,
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

    pthread_cancel( _threadID );
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
    const int error = pthread_join( _threadID, &_retVal);
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
    return pthread_equal( pthread_self(), _threadID );
}

void Thread::addListener( ExecutionListener* listener )
{
    ScopedMutex mutex( _listenerLock );
    _listeners.push_back( listener );
}

std::ostream& eqBase::operator << ( std::ostream& os, const Thread* thread )
{
#ifdef WIN32
    os << "Thread " << thread->_threadID.p;
#else
    os << "Thread " << thread->_threadID;
#endif
    return os;
}
