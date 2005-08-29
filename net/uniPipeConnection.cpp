
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "uniPipeConnection.h"
#include "connectionDescription.h"

#include <eq/base/log.h>
#include <eq/base/thread.h>

#include <dlfcn.h>
#include <errno.h>

using namespace eqNet;
using namespace std;

UniPipeConnection::UniPipeConnection()
{
    _pipe[0] = -1;
    _pipe[1] = -1;
}

UniPipeConnection::~UniPipeConnection()
{
    close();
}

//----------------------------------------------------------------------
// connect
//----------------------------------------------------------------------
bool UniPipeConnection::connect( const ConnectionDescription &description )
{
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

    INFO << "readFD " << _readFD << " writeFD " << _writeFD << endl;
    _state = STATE_CONNECTED;
    return true;
}

bool UniPipeConnection::_createPipe()
{
    if( ::pipe( _pipe ) == -1 )
    {
        ERROR << "Could not create pipe: " << strerror( errno );
        close();
        return false;
    }
    return true;
}

void UniPipeConnection::close()
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
