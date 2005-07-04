
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "thread.h"
#include "base.h"
#include "lock.h"

#include <errno.h>
#include <pthread.h>
#include <strings.h>
#include <sys/wait.h>

using namespace eqBase;
using namespace std;

Thread::Thread( const Type type )
        : _type(type),
          _threadState(STATE_STOPPED)
{
    bzero( &_threadID, sizeof( ThreadID ));
    _lock = new Lock( type );
    _lock->set();
}

Thread::~Thread()
{
    delete _lock;
}

void* Thread::runChild( void* arg )
{
    Thread* thread = static_cast<Thread*>(arg);
    ASSERT( thread );
    thread->_runChild();
    return NULL; // not reached
}

void Thread::_runChild()
{
    _threadID      = _getLocalThreadID();
    _threadState   = STATE_RUNNING;
    ssize_t result = 0;

    if( init( ))
    {
        _lock->unset(); // sync w/ parent
        result = run();
        _threadState = STATE_STOPPING;
    }
    else
    {
        _threadState = STATE_STOPPED;
        _lock->unset();
    }

    switch( _type )
    {
        case PTHREAD:
            pthread_exit( (void*)result );
            break;

        case FORK:
            exit( result );
    }
}

bool Thread::start()
{
    if( _threadState != STATE_STOPPED )
        return false;

    _threadState = STATE_STARTING;

    switch( _type )
    {
        case PTHREAD:
        {
            pthread_t      pthread;
            pthread_attr_t attributes;
            pthread_attr_init( &attributes );
            pthread_attr_setscope( &attributes, PTHREAD_SCOPE_SYSTEM );

            int nTries = 10;
            while( nTries-- )
            {
                const int error = pthread_create( &pthread, &attributes,
                                                  runChild, this );
                if( error == 0 ) // succeeded
                    break;
                if( error != EAGAIN || nTries==0 )
                {
                    WARN << "Could not create thread: " << strerror( error )
                         << endl;
                    return false;
                }
            }
        } break;

        case FORK:
        {
            const int result = fork();
            switch( result )
            {
                case 0: // child
                    INFO << "Child running" << endl;
                    _runChild(); 
                    return true; // not reached
            
                case -1: // error
                    WARN << "Could not fork child process:" 
                         << strerror( errno ) << endl;
                    return false;

                default: // parent
                    INFO << "Parent running" << endl;
                    _threadID.fork = result;
                    break;
            }
        } break;
    }

    _lock->set(); // sync with child's entry func
    // TODO: check if thread's initialised correctly. needs shmem for FORK.
    _threadState = STATE_RUNNING;
    return true;
}

bool Thread::join( ssize_t* retVal )
{
    if( _threadState == STATE_STOPPED )
        return false;

    switch( _type )
    {
        case PTHREAD:
        {
            // WAR: according to the man page, passing NULL as value_ptr should
            // be ok, but apparently it crashes on Darwin.
            ssize_t dummy;
            if( !retVal )
                retVal = &dummy;

            const int error = pthread_join( _threadID.pthread, (void**)retVal);
            if( error != 0 )
            {
                WARN << "Error joining the thread: " << strerror(error) << endl;
                return false;
            }
            _threadState = STATE_STOPPED;
        } return true;

        case FORK:
            while( true )
            {
                int status;
                pid_t result = waitpid( _threadID.fork, &status, 0 );
                if( result == _threadID.fork )
                {
                    if( WIFEXITED( status ))
                    {
                        if( retVal )
                            *retVal = WEXITSTATUS( status );
                        _threadState = STATE_STOPPED;
                        return true;
                    }
                    return false;
                }
                
                switch( errno )
                {
                    case EINTR:
                        break; // try again

                    default:
                        WARN << "Error joining the process: " 
                             << strerror(errno) << endl;
                        return false;
                }
            }
            return false;
    }
    return false;
}

Thread::ThreadID Thread::_getLocalThreadID()
{
    ThreadID threadID;

    switch( _type )
    {
        case PTHREAD:
            threadID.pthread = pthread_self();
            break;

        case FORK:
            threadID.fork = getpid();
            break;
    }

    return threadID;
}
