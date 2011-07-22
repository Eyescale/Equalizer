
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include "node.h"
#ifdef _WIN32
#  include "namedPipeConnection.h"
#endif

#include <co/base/log.h>
#include <co/base/thread.h>

namespace co
{

PipeConnection::PipeConnection()
{
    _description->type = CONNECTIONTYPE_PIPE;
    _description->bandwidth = 1024000;
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
    _sibling = new PipeConnection;
    _sibling->_sibling = this;

    if( !_createPipes( ))
    {
        close();
        return false;
    }

    _state = STATE_CONNECTED;
    _sibling->_state = STATE_CONNECTED;

    _fireStateChanged();
    return true;
}

#ifdef _WIN32

Connection::Notifier PipeConnection::getNotifier() const 
{ 
    if( !_namedPipe )
        return 0;
    return _namedPipe->getNotifier(); 
}

bool PipeConnection::_createPipes()
{
    std::stringstream pipeName;
    pipeName << "\\\\.\\pipe\\Collage." << co::base::UUID( true );

    _namedPipe = new NamedPipeConnection;
    _namedPipe->getDescription()->setFilename( pipeName.str() );
    if( !_namedPipe->listen( ))
        return false;
    _namedPipe->acceptNB();

    _sibling->_namedPipe = new NamedPipeConnection;
    _sibling->_namedPipe->getDescription()->setFilename( pipeName.str() );
    if( !_sibling->_namedPipe->connect( ))
    {
        _sibling->_namedPipe = 0;
        return false;
    }

    _namedPipe = _namedPipe->acceptSync();
    _state = STATE_CONNECTED;
    _sibling->_state = STATE_CONNECTED;
    return true;
}

void PipeConnection::close()
{
    if( _state == STATE_CLOSED )
        return;

    _namedPipe->close();
    _namedPipe = 0;
    _sibling = 0;

    _state = STATE_CLOSED;
    _fireStateChanged();
}

void PipeConnection::readNB( void* buffer, const uint64_t bytes )
{
    if( _state == STATE_CLOSED )
        return;
    _namedPipe->readNB( buffer, bytes );
}

int64_t PipeConnection::readSync( void* buffer, const uint64_t bytes,
                                       const bool ignored )
{
    if( _state == STATE_CLOSED )
        return -1;

    const int64_t bytesRead = _namedPipe->readSync( buffer, bytes, ignored );

    if( bytesRead == -1 )
        close();
    
    return bytesRead;
}

int64_t PipeConnection::write( const void* buffer, const uint64_t bytes )
{
    if( _state != STATE_CONNECTED )
        return -1;

    return _namedPipe->write( buffer, bytes );
}

#else // !_WIN32

bool PipeConnection::_createPipes()
{
    int pipeFDs[2];
    if( ::pipe( pipeFDs ) == -1 )
    {
        EQERROR << "Could not create pipe: " << strerror( errno );
        close();
        return false;
    }

    _readFD  = pipeFDs[0];
    _sibling->_writeFD = pipeFDs[1];

    if( ::pipe( pipeFDs ) == -1 )
    {
        EQERROR << "Could not create pipe: " << strerror( errno );
        close();
        return false;
    }

    _sibling->_readFD  = pipeFDs[0];
    _writeFD = pipeFDs[1];
    return true;
}

void PipeConnection::close()
{
    if( _state == STATE_CLOSED )
        return;

    if( _writeFD > 0 )
    {
        ::close( _writeFD );
        _writeFD = 0;
    }
    if( _readFD > 0 )
    {
        ::close( _readFD );
        _readFD  = 0;
    }
    _state = STATE_CLOSED;
    _sibling = 0;
    _fireStateChanged();
}
#endif // else _WIN32

}
