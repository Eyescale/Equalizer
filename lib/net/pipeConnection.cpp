
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
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
{
    _pipe[0] = -1;
    _pipe[1] = -1;
    _description = new ConnectionDescription;
    _description->type = Connection::TYPE_PIPE;
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

    if( !_createPipe( ))
    {
        close();
        return false;
    }

    _readFD  = _pipe[0];
    _writeFD = _pipe[1];

    EQINFO << "readFD " << _readFD << " writeFD " << _writeFD << endl;
    _state       = STATE_CONNECTED;
    return true;
}

bool PipeConnection::_createPipe()
{
    if( ::pipe( _pipe ) == -1 )
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

    _state = STATE_CLOSED;
}
