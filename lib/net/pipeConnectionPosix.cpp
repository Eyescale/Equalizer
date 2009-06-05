
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
