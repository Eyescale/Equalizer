
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

using namespace eqNet;
using namespace eqBase;
using namespace std;

SocketConnection::SocketConnection()
{
    _description =  new ConnectionDescription;
    _description->type = CONNECTIONTYPE_TCPIP;
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
    EQASSERT( _description->type == CONNECTIONTYPE_TCPIP );
    if( _state != STATE_CLOSED )
        return false;

    if( _description->TCPIP.port == 0 )
        return false;

    _state = STATE_CONNECTING;

    if( !_createSocket( ))
        return false;

    // TODO: execute launch command

    sockaddr_in socketAddress;
    _parseAddress( socketAddress );

    const bool connected = (::connect( _readFD, (sockaddr*)&socketAddress, 
            sizeof(socketAddress)) == 0);

    if( !connected )
    {
        EQWARN << "Could not connect to '" << _description->hostname << ":"
             << _description->TCPIP.port << "': " << EQ_SOCKET_ERROR << endl;
        close();
        return false;
    }
 
    _state = STATE_CONNECTED;
    return true;
}

void SocketConnection::_tuneSocket( const Socket fd )
{
    const int on         = 1;
    setsockopt( fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof( on ));
    setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof( on ));
}

void SocketConnection::close()
{
    if( !(_state == STATE_CONNECTED || _state == STATE_LISTENING ))
        return;

    _state   = STATE_CLOSED;
    EQASSERT( _readFD > 0 ); 

    const bool closed = ( ::close(_readFD) == 0 );
    if( !closed )
        EQWARN << "Could not close socket: " << EQ_SOCKET_ERROR << endl;

    _readFD  = INVALID_SOCKET;
    _writeFD = INVALID_SOCKET;
}

void SocketConnection::_parseAddress( sockaddr_in& socketAddress )
{
    socketAddress.sin_family      = AF_INET;
    socketAddress.sin_addr.s_addr = htonl( INADDR_ANY );
    socketAddress.sin_port        = htons( _description->TCPIP.port );
    memset( &(socketAddress.sin_zero), 0, 8 ); // zero the rest

    if( !_description->hostname.empty( ))
    {
        hostent *hptr = gethostbyname( _description->hostname.c_str() );
        if( hptr )
            memcpy(&socketAddress.sin_addr.s_addr, hptr->h_addr,hptr->h_length);
    }

    EQINFO << "Address " << inet_ntoa( socketAddress.sin_addr )
           << ":" << socketAddress.sin_port << endl;
}

//----------------------------------------------------------------------
// accept
//----------------------------------------------------------------------
RefPtr<Connection> SocketConnection::accept()
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

    SocketConnection* newConnection = new SocketConnection;

    newConnection->_readFD      = fd;
    newConnection->_writeFD     = fd;
    newConnection->_state       = STATE_CONNECTED;
    newConnection->_description->type         = CONNECTIONTYPE_TCPIP;
    newConnection->_description->bandwidthKBS = _description->bandwidthKBS;
    newConnection->_description->hostname     = inet_ntoa( newAddress.sin_addr);
    newConnection->_description->TCPIP.port   = ntohs( newAddress.sin_port );

    EQINFO << "accepted connection from " << inet_ntoa(newAddress.sin_addr) 
           << ":" << newAddress.sin_port <<endl;

    return newConnection;
}
