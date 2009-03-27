
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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
using namespace std;

namespace eq
{
namespace net
{
SocketConnection::SocketConnection( const ConnectionType type )
{
    EQASSERT( type == CONNECTIONTYPE_TCPIP || type == CONNECTIONTYPE_SDP );
    _description =  new ConnectionDescription;
    _description->type = type;
    EQVERB << "New SocketConnection @" << (void*)this << endl;
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
        EQWARN << "Can't parse connection parameters" << endl;
        close();
        return false;
    }

    if( socketAddress.sin_addr.s_addr == 0 )
    {
        EQWARN << "Refuse to connect to 0.0.0.0" << endl;
        close();
        return false;
    }

    const bool connected = (::connect( _readFD, (sockaddr*)&socketAddress, 
            sizeof(socketAddress)) == 0);

    if( !connected )
    {
        EQWARN << "Could not connect to '" << _description->getHostname() << ":"
             << _description->TCPIP.port << "': " << EQ_SOCKET_ERROR << endl;
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
        EQWARN << "Could not close socket: " << EQ_SOCKET_ERROR << endl;

    _readFD  = INVALID_SOCKET;
    _writeFD = INVALID_SOCKET;
    _fireStateChanged();
}

//----------------------------------------------------------------------
// accept
//----------------------------------------------------------------------
ConnectionPtr SocketConnection::accept()
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
        EQWARN << "accept failed: " << EQ_SOCKET_ERROR << endl;
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
           << ":" << ntohs( newAddress.sin_port ) <<endl;

    return newConnection;
}
}
}
