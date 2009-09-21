
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "ibConnection.h"

#ifdef EQ_INFINIBAND

#pragma comment ( lib, "ibal.lib" )


#include "Connection.h"
#include <Mswsock.h>

#define KEY_MSG_SIZE ( sizeof "0000:000000:000000:00000000:0000000000000000" )
#define KEY_PRINT_FMT "%04x:%06x:%06x:%08x:%016I64x"
#define KEY_SCAN_FMT "%x:%x:%x:%x:%x"
#define EQ_SOCKET_ERROR eq::base::getErrorString( GetLastError( )) << \
    "(" << GetLastError() << ")"


namespace eq
{ 
namespace net
{

IBConnection::IBConnection( ) 
{ 
    _description = new ConnectionDescription;
    _description->type = CONNECTIONTYPE_IB;
    _description->bandwidth = 819200;
}

IBConnection::~IBConnection()
{ 
}

eq::net::Connection::Notifier IBConnection::getNotifier() const 
{
    if( isListening( ) )
        return _socketConnection->getNotifier();
    else
        return _completionQueue.getReadNotifier();
}

bool IBConnection::connect()
{
    EQASSERT( _description->type == CONNECTIONTYPE_IB );

    if( _state != STATE_CLOSED )
        return false;
    
    _state = STATE_CONNECTING;
    _fireStateChanged();

    _socketConnection = new SocketConnection();

    ConnectionDescription* description = 
                                     new ConnectionDescription( *_description );
    description->type = CONNECTIONTYPE_TCPIP;

    _socketConnection->setDescription( description );

    // 1 : TCPIP Connection
    if ( !_socketConnection->connect() ||
         !_preRegister() ||
         !_establish( false ))
    {
        _socketConnection->close();     
        _socketConnection = 0;
        return false;
    }
    return true;
}

void IBConnection::close()
{
    if( !(_state == STATE_CONNECTED || _state == STATE_LISTENING ))
        return;

    if( _socketConnection.isValid( ))
        _socketConnection->close();
    _socketConnection = 0;

    _interface.close();
    _completionQueue.close();
    _adapter.close();
 
    _state = STATE_CLOSED;
    _fireStateChanged();
}

bool IBConnection::listen()
{
    EQASSERT( _description->type == CONNECTIONTYPE_IB );

    _socketConnection = new SocketConnection();  

    ConnectionDescription* description = 
            new ConnectionDescription( *_description );
    description->type = CONNECTIONTYPE_TCPIP;
    _socketConnection->setDescription( description );
    if( !_socketConnection->listen())
    {
        close();
        return false;
    }

    _state = _socketConnection->getState();
    _fireStateChanged();

    return ( _state == STATE_LISTENING );
}

void IBConnection::acceptNB()
{
    EQASSERT( _state == STATE_LISTENING );
    _socketConnection->acceptNB();
}

ConnectionPtr IBConnection::acceptSync()
{
    ConnectionPtr connection = _socketConnection->acceptSync();

    IBConnection* ibConnection = new IBConnection();
    ConnectionPtr newConnection = ibConnection;

    ibConnection->setDescription( _description );
    ibConnection->_preRegister( );  
    ibConnection->_setConnection( connection );
    if( !ibConnection->_establish( true ) )
    {
        newConnection = 0;
        return 0;
        ;
    }

    return newConnection;
}

bool IBConnection::_preRegister( )
{
    if( !_adapter.open() )
    {
        close();     
        return false;
    }
    if( !_completionQueue.create( &_adapter )) 
    {
        close();
        return false;
    }
    
    if( !_interface.create( &_adapter, &_completionQueue ))
    {
        close();
        return false;
    }
    
    return true;
}

bool IBConnection::_establish( bool isServer )
{
    struct IBDest myDest = _interface.getMyDest( 0 );

    // Create connection between client and server.
    // We do it by exchanging data over a TCP socket connection.
    EQINFO << "local address:  LID " << myDest.lid 
           << " QPN "                << myDest.qpn 
           << " PSN "                << myDest.psn 
           << " RKey "               << myDest.rkey
           << " VAddr "              << myDest.vaddr 
           << std::endl;

    struct IBDest remDest;

    bool rc;
    rc = !isServer ? _clientExchDest( &myDest, &remDest):
                     _serverExchDest( &myDest, &remDest);
    if ( !rc )
    {
        close();
        return false;
    }
    EQINFO << "dest address:  LID " << remDest.lid 
           << " QPN "               << remDest.qpn 
           << " PSN "               << remDest.psn 
           << " RKey "              << remDest.rkey
           << " VAddr "             << remDest.vaddr 
           << std::endl;

    _interface.setDestInfo( &remDest );
    _interface.modiyQueuePairAttribute( myDest.psn );
    
    // An additional handshake is required *after* moving qp to RTR.
    // Arbitrarily reuse exch_dest for this purpose.
    rc = !isServer ? _clientExchDest( &myDest, &remDest):
                     _serverExchDest( &myDest, &remDest);

    _socketConnection->close();
    _socketConnection = 0;

    if ( !rc )
    {
        close();
        return false;
    }

    _state = STATE_CONNECTED;
    _fireStateChanged();

    return true;

}

bool IBConnection::_writeKeys( const struct IBDest *myDest )
{
    char msg[KEY_MSG_SIZE];
    
    sprintf( msg, 
             KEY_PRINT_FMT, 
             cl_hton16(myDest->lid), 
             cl_hton32(myDest->qpn), 
             cl_hton32(myDest->psn), 
             cl_hton32(myDest->rkey), 
             myDest->vaddr);

    _socketConnection->send( msg, KEY_MSG_SIZE );
    return true;
}

bool IBConnection::_readKeys( struct IBDest *remDest)
{
    int parsed;
    eq::base::Buffer<void*> buf;
    buf.resize( KEY_MSG_SIZE );
    uint64_t size = KEY_MSG_SIZE;
    WSABUF wsaBuffer = { KEY_MSG_SIZE,
                         reinterpret_cast< char* >( buf.getData() ) };
    DWORD  got   = 0;
    DWORD  flags = 0;

    _socketConnection->readNB( wsaBuffer.buf, size);
    _socketConnection->readSync( wsaBuffer.buf, size);

    char* data = reinterpret_cast<char*>(buf.getData());

    parsed = sscanf( data, 
                     KEY_PRINT_FMT, 
                     &remDest->lid, 
                     &remDest->qpn,
                     &remDest->psn,
                     &remDest->rkey, 
                     &remDest->vaddr );

    remDest->lid  = cl_ntoh16(remDest->lid);
    remDest->qpn  = cl_ntoh32(remDest->qpn);
    remDest->psn  = cl_ntoh32(remDest->psn);
    remDest->rkey = cl_ntoh32(remDest->rkey);

    if ( parsed != 5 ) 
        return false;

    remDest->vaddr = ( uintptr_t ) remDest->vaddr;
    return true;
}

bool IBConnection::_serverExchDest( const struct IBDest *myDest,
                                          struct IBDest* remDest )
{
    if ( !_readKeys( remDest ))
      return false;
    return _writeKeys( myDest );
}

bool IBConnection::_clientExchDest( const struct IBDest *myDest,
                                          struct IBDest *remDest )
{
    if ( !_writeKeys( myDest) )
        return false;
    return _readKeys( remDest );
}

//----------------------------------------------------------------------
// read
//----------------------------------------------------------------------
void IBConnection::readNB( void* buffer, const uint64_t bytes )
{
    if( _state == STATE_CLOSED )
        return;
    _interface.readNB( buffer, bytes );
}

int64_t IBConnection::readSync( void* buffer, const uint64_t bytes )
{
   CHECK_THREAD( _recvThread );
    
   int64_t nbRead = _interface.readSync( buffer, bytes );
   return nbRead;
}

int64_t IBConnection::write( const void* buffer, const uint64_t bytes)
{
   int64_t numWrites = _interface.postRdmaWrite( buffer, bytes );

   if ( numWrites > 0 )
       return numWrites;

   close();

   return -1;

}
}
}
#endif //EQ_INFINIBAND
