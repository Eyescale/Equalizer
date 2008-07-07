 
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "process.h"

#include "base.h"
#include "lock.h"
#include "scopedMutex.h"

#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

namespace eq
{
namespace base
{
Process::Process()
        : _state(STATE_STOPPED),
          _pid(0)
{
}

Process::~Process()
{
}

void Process::_runChild()
{
    _pid   = getpid();
    if( !init( ))
    {
        EQINFO << "Process failed to initialise" << endl;
        _state = STATE_STOPPED;
        exit(0);
    }

    EQINFO << "Process successfully initialised" << endl;
    _state = STATE_RUNNING;
    int result = run();
    _state = STATE_STOPPING;
    exit( result );
}

// the signal handler for SIGCHILD
static void sigChildHandler( int /*signal*/ )
{
    //int status;
    //int pid = wait( &status );
    EQINFO << "Received SIGCHILD" << endl;
    signal( SIGCHLD, sigChildHandler );
}

bool Process::start()
{
    if( _state != STATE_STOPPED )
        return false;

    _state = STATE_STARTING;

    signal( SIGCHLD, sigChildHandler );
    const int result = fork();
    switch( result )
    {
        case 0: // child
            EQVERB << "Child running" << endl;
            _runChild(); 
            return true; // not reached
            
        case -1: // error
            EQWARN << "Could not fork child process:" << strerror( errno )
                   << endl;
            return false;

        default: // parent
            EQVERB << "Parent running" << endl;
            _pid = result;
            break;
    }

    _state = STATE_RUNNING;
    return true;
}

void Process::exit( int retVal )
{
    EQASSERTINFO( isCurrent( ), "Process::exit not called from child process" );

    EQINFO << "Exiting process" << endl;
    _state = STATE_STOPPING;

    ::exit( retVal );
    EQUNREACHABLE;
}

void Process::cancel()
{
    EQASSERTINFO( !isCurrent( ), "Process::cancel called from child process" );

    EQINFO << "Cancelling process" << endl;
    _state = STATE_STOPPING;

    kill( _pid, SIGTERM );
    EQUNREACHABLE;
}

bool Process::join( int* retVal )
{
    if( _state == STATE_STOPPED )
        return false;
    if( isCurrent( )) // can't join self
        return false;

    while( true )
    {
        int status;
        pid_t result = waitpid( _pid, &status, 0 );
        if( result == _pid )
        {
            if( WIFEXITED( status ))
            {
                if( retVal )
                    *retVal = WEXITSTATUS( status );
                _state = STATE_STOPPED;
                
                return true;
            }
            return false;
        }
                
        switch( errno )
        {
            case EINTR:
                break; // try again

            default:
                EQWARN << "Error joining the process: " << strerror(errno)
                       << endl;
                return false;
        }
    }
    return false;
}

bool Process::isCurrent() const
{
    return ( getpid() == _pid );
}

void Process::addListener( RunnableListener* listener )
{
    // TODO
}
}
}
