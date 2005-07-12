
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "socketConnection.h"
#include "connectionDescription.h"

#include <eq/base/log.h>

#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>

using namespace eqNet;
using namespace eqNet::priv;
using namespace std;


SocketConnection::SocketConnection()
{
}

SocketConnection::~SocketConnection()
{
    close();
}

//----------------------------------------------------------------------
// connect
//----------------------------------------------------------------------
bool SocketConnection::connect( const ConnectionDescription &description )
{
    if( _state != STATE_CLOSED )
        return false;

    _state = STATE_CONNECTING;

    _createSocket();

    // TODO: execute launch command

    sockaddr_in socketAddress;
    _parseAddress( description, socketAddress );

    const bool connected = (::connect( _readFD, (sockaddr*)&socketAddress, 
            sizeof(socketAddress)) == 0);

    if( connected )
        _state = STATE_CONNECTED;
    else
    {
        const char *hostname = description.parameters.TCPIP.hostname ? 
            description.parameters.TCPIP.hostname : "null";
        WARN << "Could not connect to '" << hostname << ":" 
             << description.parameters.TCPIP.port << "': " << strerror( errno ) 
             << endl;

        close();
    }
    
    return connected;
}

void SocketConnection::_createSocket()
{
    const int fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    if( fd == -1 )
    {
        string error = strerror( errno );
        throw connection_error("Could not create socket: " + error);
    }

    const int on = 1;
    setsockopt( fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on) );

    _readFD  = fd;
    _writeFD = fd; // TCP/IP sockets are bidirectional
}

void SocketConnection::close()
{
    if( _readFD == -1 )
        return;

    const bool closed = ( ::close(_readFD) == 0 );
    if( !closed )
        WARN << "Could not close socket: " << strerror( errno ) << endl;

    _readFD  = -1;
    _writeFD = -1;
    _state   = STATE_CLOSED;
}

void SocketConnection::_parseAddress( const ConnectionDescription &description, 
    sockaddr_in& socketAddress )
{
    socketAddress.sin_family = AF_INET;
    socketAddress.sin_addr.s_addr = htonl( INADDR_ANY );
    socketAddress.sin_port = htons( description.parameters.TCPIP.port );

    if( description.parameters.TCPIP.hostname != NULL )
    {
        hostent *hptr = gethostbyname( description.parameters.TCPIP.hostname );
        if( hptr )
            memcpy(&socketAddress.sin_addr.s_addr, hptr->h_addr,hptr->h_length);
    }
}

//----------------------------------------------------------------------
// listen
//----------------------------------------------------------------------
bool SocketConnection::listen(ConnectionDescription &description)
{
    if( _state != STATE_CLOSED )
        return false;

    _state = STATE_CONNECTING;

    _createSocket();

    sockaddr_in socketAddress;
    const size_t size = sizeof( sockaddr_in ); 

    _parseAddress( description, socketAddress ); //TODO restrict IP

    const bool bound = (::bind(_readFD, (sockaddr *)&socketAddress, size) == 0);

    if( !bound )
    {
        WARN << "Could not bind socket: " << strerror( errno ) << endl;
        close();
        return false;
    }

    const bool listening = (::listen( _readFD, 10 ) == 0);
        
    if( listening )
        _state = STATE_LISTENING;
    else
    {
        WARN << "Could not listen on socket: " << strerror( errno ) << endl;
        close();
    }

    return listening;
}

//----------------------------------------------------------------------
// accept
//----------------------------------------------------------------------
Connection* SocketConnection::accept()
{
    if( _state != STATE_LISTENING )
        return NULL;

    sockaddr_in newAddress;
    socklen_t   newAddressLen = sizeof( newAddress );
    int         fd;

    do
        fd = ::accept( _readFD, (sockaddr*)&newAddress, &newAddressLen );
    while( fd == -1 && errno == EINTR );

    if( fd == -1 )
    {
        WARN << "accept failed: " << strerror( errno ) << endl;
        return NULL;
    }

//     ConnectionDescription description;
//     char                  address[15+1+5+1];

//     description.protocol      = Network::PROTO_TCPIP;
//     description.bandwidthKBS  = _description.bandwidthKBS;
//     description.parameters.TCPIP.address = address;

//     sprintf( address, "%s:%d", inet_ntoa(newAddress.sin_addr),
//         newAddress.sin_port );

    SocketConnection* newConnection = new SocketConnection();
    newConnection->_readFD  = fd;
    newConnection->_writeFD = fd;
    newConnection->_state   = STATE_CONNECTED;
    //TODO: newConnection->launchCommand

//     INFO << "accepted connection from "
//          << newConnection->_description.parameters.TCPIP.address << endl;

    return newConnection;
}

ushort SocketConnection::getPort() const
{
    sockaddr_in address;
    socklen_t used = sizeof(address);
    getsockname( _readFD, (struct sockaddr *) &address, &used ); 
    return ntohs(address.sin_port);
}
