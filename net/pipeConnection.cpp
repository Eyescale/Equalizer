
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "pipeConnection.h"
#include "connectionDescription.h"
#include "serverPriv.h"

#include <eq/base/log.h>
#include <eq/base/thread.h>

#include <dlfcn.h>
#include <errno.h>

using namespace eqNet;
using namespace eqNet::priv;
using namespace std;

PipeConnection::PipeConnection()
        : eqBase::Thread( eqBase::Thread::FORK ),
          _pipes(NULL)
{
}

PipeConnection::~PipeConnection()
{
    close();
}

//----------------------------------------------------------------------
// connect
//----------------------------------------------------------------------
bool PipeConnection::connect( const ConnectionDescription &description )
{
    if( _state != STATE_CLOSED )
        return false;

    if( description.parameters.PIPE.entryFunc == NULL )
    {
        WARN << "No entry function defined for pipe connection" <<endl;
        return false;
    }

    _state = STATE_CONNECTING;

    _createPipes();

    // fork child process
    _entryFunc = description.parameters.PIPE.entryFunc;
    if( !start( ))
    {
        WARN << "Could not fork child process" <<endl;
        close();
        return false;
    }

    _setupParent();
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
    INFO << "Parent readFD " << _readFD << " writeFD " << _writeFD << endl;

    // cleanup
    delete [] _pipes;
    _pipes = NULL;

    // done...
    _state = STATE_CONNECTED;
}

int PipeConnection::run()
{
    // close unneeded pipe ends
    ::close( _pipes[0] );
    ::close( _pipes[3] );

    // assign file descriptors
    _readFD  = _pipes[2];
    _writeFD = _pipes[1];
    INFO << "Child  readFD " << _readFD << " writeFD " << _writeFD << endl;

    // cleanup
    delete [] _pipes;
    _pipes = NULL;

    // done... execute entry function
    _state = STATE_CONNECTED;

    // Note: right now all possible entry functions are hardcoded due to
    // security considerations.
    int result = EXIT_FAILURE;

    if( strcmp( _entryFunc, "Server::run" ) == 0 )
    {
        result = priv::Server::run( this );
    }
    else if( strcmp( _entryFunc, "testPipeServer" ) == 0 )
    {
#ifdef sgi
        void* dlHandle = dlopen( 0, RTLD_LAZY );
        void* func = dlsym( dlHandle, _entryFunc );
#else
        void* func = dlsym( RTLD_DEFAULT, _entryFunc );
#endif
        INFO << "Entry function '" << _entryFunc << "', addr: " << func << endl;
        typedef int (*EntryFunc)( Connection* connection );
        result = ((EntryFunc)func)( this );
    }
    // else if ....

    close();
    delete this;
    exit( result );
}
