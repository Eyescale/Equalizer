
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "pipeConnection.h"
#include "connectionDescription.h"

#include <eq/base/log.h>
#include <eq/base/thread.h>

#include <errno.h>

using namespace std;

namespace eq
{
namespace net
{

PipeConnection::PipeConnection()
{
    _description = new ConnectionDescription;
    _description->type = CONNECTIONTYPE_PIPE;
    EQVERB << "New PipeConnection @" << (void*)this << endl;
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
    EQASSERT( _description->type == CONNECTIONTYPE_PIPE );

    if( _state != STATE_CLOSED )
        return false;

    _state = STATE_CONNECTING;

    if( !_createPipe( ))
    {
        close();
        return false;
    }

    EQVERB << "readFD " << _readFD << " writeFD " << _writeFD << endl;
    _state = STATE_CONNECTED;
    _fireStateChanged();
    return true;
}

bool PipeConnection::_createPipe()
{
    int pipeFDs[2];
    if( ::pipe( pipeFDs ) == -1 )
    {
        EQERROR << "Could not create pipe: " << strerror( errno );
        close();
        return false;
    }

    _readFD  = pipeFDs[0];
    _writeFD = pipeFDs[1];
    return true;
}

void PipeConnection::close()
{
    if( _readFD > 0 )
    {
        ::close(_readFD);
        _readFD  = 0;
    }

    if( _writeFD > 0 )
    {
        ::close(_writeFD);
        _writeFD = 0;
    }

    _state = STATE_CLOSED;
    _fireStateChanged();
}

}
}
