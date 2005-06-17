
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "pipeConnection.h"

#include "server.h"

#include <eq/base/log.h>

#include <dlfcn.h>
#include <sys/errno.h>

using namespace eqNet;
using namespace std;

PipeConnection::~PipeConnection()
{
    close();
}

//----------------------------------------------------------------------
// connect
//----------------------------------------------------------------------
bool PipeConnection::connect()
{
    if( _state != STATE_CLOSED )
        return false;

    if( _description.launchCommand == NULL )
    {
        WARN << "No launch command function defined for pipe connection" <<endl;
        return false;
    }

    _state = STATE_CONNECTING;

    _createPipes();

    // fork child process
    pid_t pid = fork();
    switch( pid )
    {
        case 0: // child
            INFO << "Child running" << endl;
            _runChild(); // never returns
            return true;
            
        case -1: // error
            WARN << "Could not fork child process:" << strerror( errno ) <<endl;
            close();
            return false;

        default: // parent
            INFO << "Parent running" << endl;
            _setupParent();
            break;
    }
    return true;
}

// Create two pairs of pipes, since they are unidirectional
void PipeConnection::_createPipes()
{
    _pipes = new int[4];

    if( pipe( &_pipes[0] ) == -1 || pipe( &_pipes[2] ) == -1 )
    {
        string error = strerror( errno );
        throw connection_error("Could not create pipe: " + error);
    }
}

void PipeConnection::close()
{
    if( _readFD != -1 )
    {
        ::close(_readFD);
        _readFD  = -1;
    }

    if( _writeFD != -1 )
    {
        ::close(_writeFD);
        _writeFD = -1;
    }

    if( _pipes != NULL )
    {
        delete [] _pipes;
        _pipes = NULL;
    }
   
    _state   = STATE_CLOSED;
}

void PipeConnection::_setupParent()
{
    // close unneeded pipe ends
    ::close( _pipes[1] );
    ::close( _pipes[2] );

    // assign file descriptors
    _readFD  = _pipes[0];
    _writeFD = _pipes[3];
    INFO << "parent readFD " << _readFD << " writeFD " << _writeFD << endl;

    // cleanup
    delete [] _pipes;
    _pipes = NULL;

    // done...
    _state = STATE_CONNECTED;
}

void PipeConnection::_runChild()
{
    // close unneeded pipe ends
    ::close( _pipes[0] );
    ::close( _pipes[3] );

    // assign file descriptors
    _readFD  = _pipes[2];
    _writeFD = _pipes[1];
    INFO << "child  readFD " << _readFD << " writeFD " << _writeFD << endl;

    // cleanup
    delete [] _pipes;
    _pipes = NULL;

    // done... execute entry function
    _state = STATE_CONNECTED;

    // Note: right now all possible entry functions are hardcoded due to
    // security considerations.
    if( strcmp( _description.launchCommand, "Server::run" ) == 0 )
    {
        const int result = Server::run( this );
        exit( result );
    }
    else if( strcmp( _description.launchCommand, "testPipeServer" ) == 0 )
    {
        void *func = dlsym( RTLD_DEFAULT, _description.launchCommand );
        typedef int (*entryFunc)( Connection* connection );
        const int result = ((entryFunc)func)( this );
        exit( result );
    }
    // else if ....

    exit( EXIT_SUCCESS );
}
