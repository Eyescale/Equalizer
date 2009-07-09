
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

using namespace eq::base;

namespace eq
{
namespace net
{
SocketConnection::SocketConnection( const ConnectionType type )
{
    EQASSERT( type == CONNECTIONTYPE_TCPIP || type == CONNECTIONTYPE_SDP );
    _description =  new ConnectionDescription;
    _description->type = type;
    _description->bandwidth = (type == CONNECTIONTYPE_TCPIP) ?
                                  102400 : 819200;
    EQVERB << "New SocketConnection @" << (void*)this << std::endl;
}

SocketConnection::~SocketConnection()
{
    close();
}

//----------------------------------------------------------------------
// connect
//----------------------------------------------------------------------
bool SocketConnection::connect()
{
    EQASSERT( _description->type == CONNECTIONTYPE_TCPIP ||
              _description->type == CONNECTIONTYPE_SDP );
    if( _state != STATE_CLOSED )
        return false;

    if( _description->TCPIP.port == 0 )
        return false;

    _state = STATE_CONNECTING;
    _fireStateChanged();

    if( !_createSocket( ))
        return false;

    sockaddr_in socketAddress;
    if( !_parseAddress( socketAddress ))
    {
        EQWARN << "Can't parse connection parameters" << std::endl;
        close();
        return false;
    }

    if( socketAddress.sin_addr.s_addr == 0 )
    {
        EQWARN << "Refuse to connect to 0.0.0.0" << std::endl;
        close();
        return false;
    }

    const bool connected = (::connect( _readFD, (sockaddr*)&socketAddress, 
            sizeof(socketAddress)) == 0);

    if( !connected )
    {
        EQWARN << "Could not connect to '" << _description->getHostname() << ":"
             << _description->TCPIP.port << "': " << EQ_SOCKET_ERROR << std::endl;
        close();
        return false;
    }
 
    _state = STATE_CONNECTED;
    _fireStateChanged();
    return true;
}

void SocketConnection::close()
{
    if( !(_state == STATE_CONNECTED || _state == STATE_LISTENING ))
        return;

    _state = STATE_CLOSED;
    EQASSERT( _readFD > 0 ); 

    const bool closed = ( ::close(_readFD) == 0 );
    if( !closed )
        EQWARN << "Could not close socket: " << EQ_SOCKET_ERROR << std::endl;

    _readFD  = INVALID_SOCKET;
    _writeFD = INVALID_SOCKET;
    _fireStateChanged();
}

//----------------------------------------------------------------------
// Async IO handles
//----------------------------------------------------------------------
void SocketConnection::_initAIOAccept(){ /* NOP */ }
void SocketConnection::_exitAIOAccept(){ /* NOP */ }
void SocketConnection::_initAIORead(){ /* NOP */ }
void SocketConnection::_exitAIORead(){ /* NOP */ }

//----------------------------------------------------------------------
// accept
//----------------------------------------------------------------------
void SocketConnection::acceptNB(){ /* NOP */ }
 
ConnectionPtr SocketConnection::acceptSync()
{
    if( _state != STATE_LISTENING )
        return 0;

    sockaddr_in newAddress;
    socklen_t   newAddressLen = sizeof( newAddress );

    Socket    fd;
    unsigned  nTries = 1000;
    do
        fd = ::accept( _readFD, (sockaddr*)&newAddress, &newAddressLen );
    while( fd == INVALID_SOCKET && errno == EINTR && --nTries );

    if( fd == INVALID_SOCKET )
    {
        EQWARN << "accept failed: " << EQ_SOCKET_ERROR << std::endl;
        return 0;
    }

    _tuneSocket( fd );

    SocketConnection* newConnection = new SocketConnection( _description->type);

    newConnection->_readFD      = fd;
    newConnection->_writeFD     = fd;
    newConnection->_state       = STATE_CONNECTED;
    newConnection->_description->bandwidth = _description->bandwidth;
    newConnection->_description->setHostname( inet_ntoa( newAddress.sin_addr ));
    newConnection->_description->TCPIP.port   = ntohs( newAddress.sin_port );

    EQVERB << "accepted connection from " << inet_ntoa(newAddress.sin_addr) 
           << ":" << ntohs( newAddress.sin_port ) << std::endl;

    return newConnection;
}
}
}
