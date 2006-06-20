
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "pipeConnection.h"
#include "connectionDescription.h"

#include <eq/base/log.h>
#include <eq/base/thread.h>

#include <dlfcn.h>
#include <errno.h>

using namespace eqNet;
using namespace std;

PipeConnection::PipeConnection()
        : _pipes(NULL)
{
    _description = new ConnectionDescription;
    _description->type = Connection::TYPE_PIPE;
    EQINFO << "New PipeConnection @" << (void*)this << endl;
}

PipeConnection::PipeConnection(const PipeConnection& conn)
        : FDConnection(conn),
          _childConnection( conn._childConnection )
{
    _description = new ConnectionDescription;
    _description->type = Connection::TYPE_PIPE;

    if( conn._pipes )
    {
        _pipes = new int[4];
        memcpy( _pipes, conn._pipes, 4*sizeof(int));
    }
}

PipeConnection::~PipeConnection()
{
    close();
}

//----------------------------------------------------------------------
// connect
//----------------------------------------------------------------------
bool PipeConnection::connect()
{
    EQASSERT( _description->type == TYPE_PIPE );

    if( _state != STATE_CLOSED )
        return false;

    _state = STATE_CONNECTING;

    if( !_createPipes( ))
    {
        close();
        return false;
    }

    PipeConnection* childConnection = new PipeConnection( *this );
    childConnection->_setupChild();
    _childConnection = childConnection;

    _setupParent();
    return true;
}

// Create two pairs of pipes, since they are unidirectional
bool PipeConnection::_createPipes()
{
    _pipes = new int[4];

    if( pipe( &_pipes[0] ) == -1 || pipe( &_pipes[2] ) == -1 )
    {
        EQERROR << "Could not create pipe: " << strerror( errno );
        close();
        return false;
    }
    return true;
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

    _childConnection = NULL;
    _state = STATE_CLOSED;
}

void PipeConnection::_setupParent()
{
    EQASSERT( _state == STATE_CONNECTING );
    // assign file descriptors
    _readFD  = _pipes[0];
    _writeFD = _pipes[3];
    _description->Pipe.fd = _pipes[1]; // i.e. the write end to parent read end
    EQINFO << "Parent readFD " << _readFD << " writeFD " << _writeFD << endl;

    // cleanup
    EQASSERT(_pipes);
    delete [] _pipes;
    _pipes = NULL;

    // done...
    _state = STATE_CONNECTED;
}

void PipeConnection::_setupChild()
{
    EQASSERT( _state == STATE_CONNECTING );
    // assign file descriptors
    _readFD  = _pipes[2];
    _writeFD = _pipes[1];
    _description->Pipe.fd = _pipes[3]; // i.e. the write end to child read end
    EQINFO << "Child  readFD " << _readFD << " writeFD " << _writeFD << endl;

    // cleanup
    EQASSERT(_pipes);
    delete [] _pipes;
    _pipes = NULL;

    // done...
    _state = STATE_CONNECTED;
}

// #ifdef sgi
//         void* dlHandle = dlopen( 0, RTLD_LAZY );
//         void* func = dlsym( dlHandle, _entryFunc );
// #else
//         void* func = dlsym( RTLD_DEFAULT, _entryFunc );
// #endif
//         INFO << "Entry function '" << _entryFunc << "', addr: " << func << endl;
//         typedef int (*EntryFunc)( Connection* connection );
//         result = ((EntryFunc)func)( this );
