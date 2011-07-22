
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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
#define EQ_NUMCONN_IB 32
#pragma comment ( lib, "ibal.lib" )


#include "Connection.h"

#define KEY_MSG_SIZE ( sizeof "0000:000000:000000:00000000:0000000000000000" )
#define KEY_PRINT_FMT "%04x:%06x:%06x:%08x:%016I64x"
#define KEY_SCAN_FMT "%x:%x:%x:%x:%x"
namespace co

IBConnection::IBConnection( )
{ 
    _description->type = CONNECTIONTYPE_IB;
    _description->bandwidth = 819200;

    _completionQueues.resize( EQ_NUMCONN_IB );
    _interfaces.resize( EQ_NUMCONN_IB );

    for ( int i = 0; i < EQ_NUMCONN_IB; i++ )
    {
        _interfaces[i] = new IBInterface();
        _completionQueues[i] = new IBCompletionQueue();
    }

    numWrite = 0;

}

IBConnection::~IBConnection()
{
    
    close();

    for ( int i = 0; i < EQ_NUMCONN_IB; i++ )
    {
        if ( _interfaces[i] )
            delete _interfaces[i];
        _interfaces[i] = 0;
        
        if ( _completionQueues[i] )
           delete _completionQueues[i];
        _completionQueues[i] = 0;
    }
}

eq::net::Connection::Notifier IBConnection::getNotifier() const 
{
    if( isListening( ) )
        return _socketConnection->getNotifier();

    return _readEvent;
}

bool IBConnection::connect()
{
    EQASSERT( _description->type == CONNECTIONTYPE_IB );

    if( _state != STATE_CLOSED )
        return false;
    
    _state = STATE_CONNECTING;
    _fireStateChanged();

    _socketConnection = new SocketConnection();

    ConnectionDescriptionPtr description = 
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

    EQINFO << "Connected " << _description << std::endl;
    return true;
}

void IBConnection::close()
{
    if( !(_state == STATE_CONNECTED || _state == STATE_LISTENING ))
        return;

    if( _socketConnection.isValid( ))
        _socketConnection->close();
    _socketConnection = 0;

    for ( int i = 0; i < EQ_NUMCONN_IB; i++ )
    {
        _interfaces[i]->close();
        _completionQueues[i]->close();
    }
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
    }

    return newConnection;
}

bool IBConnection::_preRegister( )
{
    if( !_adapter.open() )
    {
        EQWARN << "Can't open IB adapter" << std::endl;
        close();
        return false;
    }
    _readEvent = CreateEvent( 0, true, false, 0 );

    for ( int i = 0; i < EQ_NUMCONN_IB; i++ )
    {
        if( !_completionQueues[i]->create( this, &_adapter, 
             _interfaces[i]->getNumberBlockMemory() ))
        {
            EQWARN << "Can't create completion queue" << std::endl;
            close();
            return false;
        }
        
        if( !_interfaces[i]->create( &_adapter, 
                                     _completionQueues[i],
                                     this ))
        {
            close();
            return false;
        }
    }
    
    return true;
}
bool IBConnection::_establish( bool isServer )
{
    for ( int i = 0; i < EQ_NUMCONN_IB; i++ )
        if ( !_establish( isServer, i ))
        {
            _socketConnection->close();
            _socketConnection = 0;
            return false;
        }
    _socketConnection->close();
    _socketConnection = 0;
    _state = STATE_CONNECTED;
    _fireStateChanged();

    return true;
}
bool IBConnection::_establish( bool isServer, uint32_t index )
{

    uint32_t numberBlockMemory = 
            _interfaces[index]->getNumberBlockMemory();
    int psn;
    for (int i = 0 ; i < numberBlockMemory ; i++)
    {
        struct IBDest myDest = _interfaces[index]->getMyDest( i );

        // Create connection between client and server.
        // We do it by exchanging data over a TCP socket connection.
       /*
        EQINFO << "local address:  LID " << myDest.lid 
               << " QPN "                << myDest.qpn 
               << " PSN "                << myDest.psn 
               << " RKey "               << myDest.rkey
               << " VAddr "              << myDest.vaddr 
               << std::endl;
        */
        struct IBDest remDest;

        bool rc;
        rc = !isServer ? _clientExchDest( &myDest, &remDest):
                         _serverExchDest( &myDest, &remDest);
        if ( !rc )
        {
            close();
            return false;
        }
    /*    EQINFO << "dest address:  LID " << remDest.lid 
               << " QPN "               << remDest.qpn 
               << " PSN "               << remDest.psn 
               << " RKey "              << remDest.rkey
               << " VAddr "             << remDest.vaddr 
               << std::endl;
     */
        _interfaces[index]->setDestInfo( &remDest, i );
        
        // An additional handshake is required *after* moving qp to RTR.
        // Arbitrarily reuse exch_dest for this purpose.
        rc = !isServer ? _clientExchDest( &myDest, &remDest):
                         _serverExchDest( &myDest, &remDest);
    }
    _interfaces[index]->modiyQueuePairAttribute( );
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


void IBConnection::addEvent()
{
    eq::base::ScopedMutex mutex( _mutex );
    
    _comptEvent++;

    if( _comptEvent  > 0 ) 
        SetEvent( _readEvent );
}
void IBConnection::removeEvent()
{
    eq::base::ScopedMutex mutex( _mutex );
    _comptEvent--;
    if( _comptEvent  < 1 ) 
    {
        ResetEvent( _readEvent );
    }
        
}

void IBConnection::incReadInterface()
{
    numRead = ( numRead + 1 ) % EQ_NUMCONN_IB;
}

void IBConnection::incWriteInterface()
{
    numWrite = ( numWrite + 1 ) % EQ_NUMCONN_IB;
}

//----------------------------------------------------------------------
// read
//----------------------------------------------------------------------
void IBConnection::readNB( void* buffer, const uint64_t bytes ){ /* NOB */ }

int64_t IBConnection::readSync( void* buffer, const uint64_t bytes,
                                const bool ignored )
{
    EQ_TS_THREAD( _recvThread );
    int64_t result = _interfaces[ numRead ]->readSync( buffer, bytes );
    if ( result < 0 ) 
        close();

    return result;
}

int64_t IBConnection::write( const void* buffer, const uint64_t bytes)
{
    if( _state != STATE_CONNECTED )
        return -1;
    
    int64_t result = _interfaces[ numWrite ]->postRdmaWrite( buffer, bytes );

    if ( result < 0 ) 
        close();

    return result;
}

}
#endif //EQ_INFINIBAND
