/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com> 
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

#include "rspConnection.h"

#include "connection.h"
#include "connectionDescription.h"


#define SELF_INTERRUPT 42

#ifdef WIN32
#  define SELECT_TIMEOUT WAIT_TIMEOUT
#  define SELECT_ERROR   WAIT_FAILED
#else
#  define SELECT_TIMEOUT  0
#  define SELECT_ERROR   -1
#endif

namespace eq
{
namespace net
{

namespace
{
static const size_t _maxBuffer = 1454*1000;
static const size_t _numberBuffer = 4;
static const size_t _maxDatagram = _maxBuffer /  ( UDPConnection::getMTU() - 
                                     sizeof( RSPConnection::DatagramData ) )+1;
static const size_t _maxNack = ( UDPConnection::getMTU() - 
                                sizeof( RSPConnection::DatagramNack ) ) / 
                                              sizeof( RSPConnection::RepeatID );
}

RSPConnection::RSPConnection()
        : _countAcceptChildren( 0 )
        , _currentReadSync( 0 )
        , _thread ( 0 )
        , _connection( 0 )
        , _parentConnection( 0 )
        , _maxLengthDatagramData( 0 )
        , _lastSequenceIDAck( -1 )
        , _timeEvent( 999999999 )
{
    _description->type = CONNECTIONTYPE_MCIP_RSP;
    _description->bandwidth = 102400;

    for ( uint8_t i = 0; i < _numberBuffer; i++ )
    {
        DataReceive* receive = new DataReceive();
        receive->sequenceID = 0;
        receive->ackSend    = true;
        receive->allRead    = true;
        receive->posRead    = 0;
        receive->totalSize  = 0;
        receive->boolBuffer.resize( _maxDatagram );

        memset( receive->boolBuffer.getData() , false, 
                receive->boolBuffer.getSize() );

        receive->dataBuffer.resize( _maxBuffer );
        
        _buffer.push_back( receive );
    }
    bufferReceive = _buffer[0];
}

void RSPConnection::close()
{
    if ( _connection )
        _connection->close();
    _connection = 0;

    if ( _thread )
        _thread->join();
}

RSPConnection::~RSPConnection()
{
    close();
}

//----------------------------------------------------------------------
// Async IO handles
//----------------------------------------------------------------------
#ifdef WIN32
void RSPConnection::_initAIORead()
{
    _hEvent = CreateEvent( 0, FALSE, FALSE, 0 );
    EQASSERT( _hEvent );

    if( !_hEvent )
        EQERROR << "Can't create event for AIO notification: " 
                << base::sysError << std::endl;
}

void RSPConnection::_initAIOAccept()
{
    _initAIORead();
}

void RSPConnection::_exitAIOAccept()
{
    _exitAIORead();
}
void RSPConnection::_exitAIORead()
{
    if( _hEvent )
    {
        CloseHandle( _hEvent );
        _hEvent = 0;
    }
}
#else
void RSPConnection::_initAIOAccept(){ /* NOP */ }
void RSPConnection::_exitAIOAccept(){ /* NOP */ }
void RSPConnection::_initAIORead(){ /* NOP */ }
void RSPConnection::_exitAIORead(){ /* NOP */ }
#endif

bool RSPConnection::connect()
{
    return listen();
}

bool RSPConnection::listen()
{
    EQASSERT( _description->type == CONNECTIONTYPE_MCIP_RSP );

    if( _state != STATE_CLOSED )
        return false;
    
    _state = STATE_CONNECTING;
    _fireStateChanged();

    // init an udp Connection
    _connection = new UDPConnection();
    ConnectionDescription* description = 
        new ConnectionDescription( *_description );
    description->type = CONNECTIONTYPE_UDP;
    _connection->setDescription( description );
    _description->type = CONNECTIONTYPE_MCIP_RSP;
    _maxLengthDatagramData = _connection->getMTU() - 
                         sizeof( DatagramData );

    // connect UDP multicast
    if ( !_connection->connect() )
    {
        EQWARN << "can't connect RSP transmission " << std::endl;
        return false;
    }

    _parentConnection = 0;
    // init a thread for manage the protocole communication

    _thread = new Thread( this );

#ifdef WIN32
    _initAIOAccept();
    _writeEndEvent = CreateEvent( 0, FALSE, FALSE, 0 );
#else
    _selfPipeWriteEventEnd = new PipeConnection;
    if( !_selfPipeWriteEventEnd->connect( ))
    {
        EQERROR << "Could not create connection" << std::endl;
        return false;
    }
    

    _countNbAckInWrite =  0;
    _writeEndEvent.events = POLLIN;
    _writeEndEvent.fd = _selfPipeWriteEventEnd->getNotifier();
    _writeEndEvent.revents = 0;

    _selfPipeHEvent = new PipeConnection;
    if( !_selfPipeHEvent->connect( ))
    {
        EQERROR << "Could not create connection" << std::endl;
        return false;
    }

    _hEvent.events = POLLIN;
    _hEvent.fd = _selfPipeHEvent->getNotifier();
    _readFD = _hEvent.fd;
    _hEvent.revents = 0;

    _udpEvent.events = POLLIN; // | POLLPRI;
    _udpEvent.fd = _connection->getNotifier();
    EQASSERT( _udpEvent.fd > 0 );
    _udpEvent.revents = 0;

#endif
    _countNbAckInWrite =  0;

    _myID = _rng.get<uint32_t>();
    _bufRead.resize( _connection->getMTU() );
    
    _thread->start();
     const DatagramNode newnode ={ NEWNODE, _getID() };
     _connection->write( &newnode, sizeof( DatagramNode ) );

    _state = STATE_LISTENING;
    _fireStateChanged();
    
    return true;
}

void RSPConnection::acceptNB()
{
    EQASSERT( _state == STATE_LISTENING );

    if ( _childrensConnection.size() > _countAcceptChildren )
#ifdef WIN32
        SetEvent( _hEvent );
    else
        ResetEvent( _hEvent );
#else
    {
        _selfPipeHEvent->recvNB( &_selfCommand, sizeof( _selfCommand ));
        const char c = SELF_INTERRUPT;
        _selfPipeHEvent->send( &c, 1, true );
    }
    else 
    {
        _selfPipeHEvent->recvNB( &_selfCommand, sizeof( _selfCommand ));	
    }

#endif
}

ConnectionPtr RSPConnection::acceptSync()
{
    CHECK_THREAD( _recvThread );
    if( _state != STATE_LISTENING )
        return 0;

    if ( _countAcceptChildren == _childrensConnection.size() )
#ifdef WIN32
        WaitForSingleObject( getNotifier(), INFINITE );
#else
        poll( &_hEvent, 1, -1 );
#endif
    EQASSERT ( _countAcceptChildren < _childrensConnection.size() )
    RSPConnection* newConnection = 
                        _childrensConnection[ _countAcceptChildren ];

    ConnectionPtr  connection( newConnection ); // to keep ref-counting correct

    newConnection->_initAIORead();
    newConnection->_parentConnection = this;
    newConnection->_connection = _connection;
    newConnection->_state       = STATE_CONNECTED;
    newConnection->_description = _description;

#ifndef WIN32
    _selfPipeHEvent->recvSync( 0, 0 );

    newConnection->_selfPipeHEvent = new PipeConnection;
    if( !newConnection->_selfPipeHEvent->connect( ))
    {
        EQERROR << "Could not create connection" << std::endl;
        return false;
    }
    
    newConnection->_hEvent.events = POLLIN;
    newConnection->_hEvent.fd = newConnection->_selfPipeHEvent->getNotifier();
    newConnection->_readFD = newConnection->_hEvent.fd;
    newConnection->_hEvent.revents = 0;

#endif     
    _countAcceptChildren++;

    const DatagramCountConnection countNode = 
                        { COUNTNODE, _getID(), _childrensConnection.size() };
    _connection->write( &countNode, sizeof( DatagramCountConnection ) );
    
    EQINFO << "accepted connection " << (void*)newConnection
           << std::endl;

    return connection;
}

void RSPConnection::readNB( void* buffer, const uint64_t bytes )
{
    EQASSERT( _state == STATE_CONNECTED );

    base::ScopedMutex mutex( _mutexRead );
    
    for ( std::vector< DataReceive* >::iterator i = _buffer.begin();
              i != _buffer.end(); ++i )
    {
        if ( (*i)->ackSend && !(*i)->allRead )
        {
#ifdef WIN32
            SetEvent( _hEvent );
#else
            const char c = SELF_INTERRUPT;
            _selfPipeHEvent->send( &c, 1, true );
#endif
            return;
        }
    }

#ifdef WIN32
    ResetEvent( _hEvent );
#else
    _selfPipeHEvent->recvNB( &_selfCommand, sizeof( _selfCommand ));
#endif

}

eq::net::RSPConnection::DataReceive* RSPConnection::_findReceiverRead()
{
    DataReceive* receiver = _currentReadSync;
    base::ScopedMutex mutex( _mutexRead );
    if ( !receiver )
    {
        for ( std::vector< DataReceive* >::iterator i = _buffer.begin();
                  i != _buffer.end(); ++i )
        {
            if( (*i)->ackSend && !(*i)->allRead )
                return *i;
            
        }
    }

    return receiver;
}
int64_t RSPConnection::readSync( void* buffer, const uint64_t bytes )
{
    uint32_t size =   EQ_MIN( bytes, _maxBuffer );
    DataReceive* receiver = 0;
    
    while( !receiver )
    {
        receiver = _findReceiverRead();
        
        if ( receiver )
            break;

#ifdef WIN32
        WaitForSingleObject( getNotifier(), INFINITE );
#else
        poll( &_hEvent, 1, -1 );
#endif
    }
#ifndef WIN32
    _selfPipeHEvent->recvSync( 0, 0 );
#endif
    return _readSync( receiver, buffer, size );
   
}

void RSPConnection::_run()
{
    bool doReadNb = true;
    while ( true )
    {
        if ( doReadNb )
            _connection->readNB( _bufRead.getData(), _connection->getMTU() );
#ifdef WIN32
        WORD ret = WaitForSingleObject( _connection->getNotifier(), _timeEvent );
#else
        int ret = poll( &_udpEvent, 1, _timeEvent );
#endif
        switch ( ret )
        {
        case SELECT_TIMEOUT:
        {
            doReadNb = false;
            for ( std::vector< RSPConnection* >::iterator i = 
                   _childrensConnection.begin() ;
                  i != _childrensConnection.end(); ++i )
            {
                RSPConnection* connection = *i;
                if ( !connection->_ackReceive )
                {
                    connection->_countTimeOut++;
                    // send a ack request
                    if ( connection->_countTimeOut % 5 == 0)
                    {   
                        // send a datagram Ack Request    
                        _sendAckRequest();
                        // may the connection is dead
                        if ( connection->_countTimeOut >= 100 )
                        {
                            //const DatagramNode newnode = 
                            //                  { EXITNODE, clientRSP->writerId };
                            //_connection->write( &newnode, sizeof(DatagramNode));
                            //clientsRSP.erase( i );
                        }
                        break;
                    }
                }
            }
            break;
        }
        default:
        {
            doReadNb = true;
            _read();
        }
        }
    }
}

void* RSPConnection::Thread::run()
{
    _connection->_run();
    return 0;
}

int64_t RSPConnection::_readSync( DataReceive* receive, 
                                  void* buffer, 
                                  const uint64_t bytes )
{
    const uint64_t size = EQ_MIN( bytes, receive->totalSize - 
                                         receive->posRead );
    const uint8_t* data = receive->dataBuffer.getData()+ receive->posRead;
    memcpy( buffer, data, size );
    
    receive->posRead += size;
    
    // if all data in the buffer has been taken
    if ( receive->posRead == receive->totalSize )
    {
        base::ScopedMutex mutex( _mutexRead );
        memset( receive->boolBuffer.getData(), 
                false, receive->boolBuffer.getSize() * sizeof( bool ));
        receive->allRead = true;
        _currentReadSync = 0;

    }

    return bytes;

}


void RSPConnection::_read( )
{
    base::ScopedMutex mutex( _mutexRead );
    // read datagram 
    if ( _connection->readSync( _bufRead.getData(), 
                                _connection->getMTU() ) == -1 )
        return;
                                
    
    // read datagram type
    const uint8_t* type = reinterpret_cast<uint8_t*>( _bufRead.getData() ); 
    switch ( *type )
    {
    case DATA: 
    { 
        const DatagramData* datagram = reinterpret_cast< const DatagramData* >
                                       ( _bufRead.getData() );

        if ( _myID == datagram->writerID )
            return;
        RSPConnection* connection = _findConnectionWithWriterID( 
                                                        datagram->writerID );
        // find connection destination
        for ( uint32_t i = 0 ; i < _childrensConnection.size(); i++ )
        {
            connection = _childrensConnection[i];
            
            if ( connection->_writerID != datagram->writerID )
                continue;
            
            // if the buffer has ever been found during previous read or last
            // ack data sequence
            if ( connection->bufferReceive )
                break;

            // find the data correspondig buffer 
            // maybe not need this because if we arriving here, there are one
            // situation
            // 1: ack send and we haven't found a new free buffer
            connection->bufferReceive = 
                connection->_findReceiverWithSequenceID( datagram->sequenceID );

            // if we found the buffer receiver 
            if ( connection->bufferReceive )
                break;

            // Found a free buffer that all data
            for ( std::vector< DataReceive* >::iterator k = 
                               connection->_buffer.begin();
                  k != connection->_buffer.end(); ++k )
            {
                if( (*k)->allRead )
                {
                    connection->bufferReceive = *k;
                    break;
                }
            }
        }
        
        if ( !connection )
            return;

        if ( !connection->bufferReceive )
            return;
        
        DataReceive* receive = connection->bufferReceive;

                
        // if it's the first datagram 
        if ( receive->ackSend )
        {

            if ( datagram->sequenceID == receive->sequenceID )
                return;

            // if it's a datagram repetition for an other connection, we have
            // to ignore it
            if ( ( connection->_lastSequenceIDAck == datagram->sequenceID ) &&
                    connection->_lastSequenceIDAck != -1 )
                return;

            receive->sequenceID = datagram->sequenceID;
            receive->posRead   = 0;
            receive->totalSize = 0;
            receive->ackSend   = false;
        }

        uint64_t index = datagram->dataID;

        // if it's a repetition and we have the data then we ignore it
        if ( !receive->boolBuffer.getData()[ index ] )
        {
            uint16_t length = datagram->length; 
            datagram++;
            const uint8_t* data = reinterpret_cast< const uint8_t* >
                                                         ( datagram );
            receive->boolBuffer.getData()[ index ] = true;
            receive->totalSize += length;
            const uint64_t pos = ( index ) * ( _connection->getMTU() - 
                                         sizeof( DatagramData ) );
            memcpy( receive->dataBuffer.getData() + pos,
                    data, length );
            return;
        }
        break;
    }
    case ACK: 
    {
        const DatagramAck* ack = reinterpret_cast< const DatagramAck* >( type );
        
        // if the ack is not for me, we ignore it. the case is when the 
        // connection receive it's own data
        if ( ack->writerID != _myID )
            return;

        // sometimes we repeat for a detected timeout that a connection
        // has respond for it
        if ( ack->sequenceID != _sequenceIDWrite )
            return;

        RSPConnection* connection = 0;

        // find connection destination and if we have not receive an ack from,
        // we update the ack data.
        for ( std::vector< RSPConnection* >::iterator i = 
                                    _childrensConnection.begin();
              i != _childrensConnection.end(); ++i )
        {
            connection = *i;
            if ( connection->_writerID == ack->readerID )
            {
                if ( connection->_ackReceive )
                    return;

                connection->_ackReceive = true;
                _countNbAckInWrite++;
                // if all connections have send ack 
                if ( _countNbAckInWrite == _childrensConnection.size() )
                {
#ifdef WIN32
                    _timeEvent = INFINITE;
                    SetEvent( _writeEndEvent );
#else
                    const char c = SELF_INTERRUPT;
                    _selfPipeWriteEventEnd->send( &c, 1, true );
#endif
                    // reset counter timeout
                    connection->_countTimeOut = 0;
                }
                break;
            }
        }
        break;
    }
    case NACK:
    {
        const DatagramNack* nack = reinterpret_cast< const DatagramNack* >
                                                         ( _bufRead.getData() );
        
        if ( nack->readerID == _myID )
            return;
        
        // TO DO Write Error
        //EQASSERT( nack->idSequence == _idSequenceWrite );
        // understand why we have a nack for bad sequence
        if ( nack->sequenceID != _sequenceIDWrite )
            return;
        
        RSPConnection* connection = _findConnectionWithWriterID( 
                                                        nack->readerID );

        if ( connection )
        {
            connection->_countTimeOut = 0;
        }
        else
        {
            // it's an unknow connection 
            // TO DO we may be add this connection
        }

        uint8_t countRepeatID = nack->countRepeatID;
        nack++;
        const RepeatID* repeatID = reinterpret_cast< const RepeatID* >( nack );
        for ( uint8_t j = 0; j < countRepeatID; j++ )
        {
            uint32_t start = repeatID->start;
            uint32_t end   = repeatID->end;
            // repeat datagram data
            for ( uint32_t i = start; i <= end; i++ )
                _sendDatagram( i );
            repeatID++;
        }

        // Send a ack request
        const DatagramAckRequest ackRequest = { ACKREQ, _myID, 
                                      _numberDatagram -1, 
                                      _sequenceIDWrite };
        _connection->write( &ackRequest, sizeof( ackRequest ) );
        break;
    }
    case ACKREQ: // The writer ask for a ack data
    {
        const DatagramAckRequest* ackRequest = 
                  reinterpret_cast< const DatagramAckRequest* >
                                                         ( _bufRead.getData() );
        
        // if it's my own request
        if ( ackRequest->writerID == _getID() )
            return;

        DataReceive* receive = 0;

        // find the connection and send a ack if all data hve been receive or
        // send a nack for lost data
        RSPConnection* connection = 0;
        for ( std::vector< RSPConnection* >::iterator m = 
                                                   _childrensConnection.begin();
              m != _childrensConnection.end(); ++m )
        {
            if( (*m)->_writerID != ackRequest->writerID )
                continue;
            
            connection = *m;

            // find the correspondig buffer
            receive = connection->_findReceiverWithSequenceID( 
                                                       ackRequest->sequenceID );
            
            // Why no receiver found ?
            // 1 : all datagram data has not been receive ( timeout )
            // 2 : all receiver are full and not ready for receive datagramData
            //    We ask for resend all the datagrams
            if ( !receive )
            {
                RepeatID repeatID = { 0, ackRequest->lastDatagramID }; 
                sendNackDatagram ( connection->_writerID,
                                   ackRequest->sequenceID,
                                   1, &repeatID );
                return;
            }
            
            // Repeat ack
            if ( receive->ackSend )
            {
                const DatagramAck ack = { ACK, _myID, ackRequest->writerID, 
                                    ackRequest->sequenceID };
                _connection->write( &ack, sizeof( ack ) );
                return;
            }
        }

        if ( !receive )
            return;
        
        // find all lost datagramData
        receive->boolBuffer.resize( ackRequest->lastDatagramID + 1);
        eq::base::Buffer< RepeatID > bufferRepeatID;
        for ( uint32_t i = 0; i < receive->boolBuffer.getSize(); i++)
        {
            // size max datagram = mtu
            if ( _maxNack <= bufferRepeatID.getSize() )
                break;

            if ( receive->boolBuffer.getData()[i] )
                continue;
            
            RepeatID repeatID = { i, receive->boolBuffer.getSize()-1 };

            // OPT: Send all NACK packets at once
            for( ; i < receive->boolBuffer.getSize(); ++i )
            {
                if( !receive->boolBuffer.getData()[i] )
                    continue;
                repeatID.end = i-1;
                break;
            }
            bufferRepeatID.append( repeatID );
        }
        
        // send datagram NACK
        if ( bufferRepeatID.getSize() > 0 )
        {
            sendNackDatagram ( connection->_writerID,
                               ackRequest->sequenceID,
                               bufferRepeatID.getSize(),
                               bufferRepeatID.getData() );
            return;
        }


        // the receiver is ready and can be read by ReadSync
        receive->ackSend = true;
        receive->allRead = false;
        connection->bufferReceive = 0;
            
        // Found a free buffer for the next receive
        for ( std::vector< DataReceive* >::iterator k = 
                           connection->_buffer.begin();
              k != connection->_buffer.end(); ++k )
        {
            if( (*k)->allRead )
            {
                connection->bufferReceive = *k;
                break;
            }
        }

        // ack data
        DatagramAck ack = { ACK, _myID, connection->_writerID, 
                            receive->sequenceID };
        _connection->write( &ack, sizeof( ack ) );
        connection->_lastSequenceIDAck = receive->sequenceID;
#ifdef WIN32
        SetEvent( connection->_hEvent );
#else
        const char c = SELF_INTERRUPT;
        connection->_selfPipeHEvent->send( &c, 1, true );
#endif
        break;
    }
    case NEWNODE:
    {
        
        base::ScopedMutex mutexConn( _mutexConnection );
        const DatagramNode* node = reinterpret_cast< const DatagramNode* >
                                                       (  _bufRead.getData()  );

        if ( node->connectionID != _myID )
        {
            for ( std::vector< RSPConnection* >::iterator i = 
                   _childrensConnection.begin() ; 
                   i != _childrensConnection.end(); ++i )
            {
                if ( (*i)->_writerID == node->connectionID )
                    return;
            }
            _addNewConnection( node->connectionID );
#ifdef WIN32
            SetEvent( _hEvent );
#else
            const char c = SELF_INTERRUPT;
            _selfPipeHEvent->send( &c, 1, true );
#endif
        }
        break;
    }
    case EXITNODE:
    {
        
        // later
        const DatagramNode* node = reinterpret_cast< const DatagramNode* >
                                                         ( _bufRead.getData() );
        EQWARN << "remove Connection " << node->connectionID << std::endl;
        if ( node->connectionID != _myID )
        {
            const DatagramCountConnection countNode = 
                          { COUNTNODE, _getID(), _childrensConnection.size() };
            _connection->write( &countNode, sizeof( DatagramCountConnection ) );
            return;
        }
        for ( std::vector< RSPConnection* >::iterator i = 
                   _childrensConnection.begin() ;
              i != _childrensConnection.end(); ++i )
        {
            if ( (*i)->_writerID == node->connectionID )
            {
                _childrensConnection.erase( i );
                break;
            }
        }

        const DatagramCountConnection countNode = 
                           { COUNTNODE, _getID(), _childrensConnection.size() };
        _connection->write( &countNode, sizeof( DatagramCountConnection ) );
            
        break;
    }
    case COUNTNODE:
    {
        base::ScopedMutex mutexConn( _mutexConnection );
        const DatagramCountConnection* countConn = 
                reinterpret_cast< const DatagramCountConnection* >
                                                         ( _bufRead.getData() );
        
        if( countConn->clientID == _myID )
            return;
        // we know all connection
        if ( _childrensConnection.size() == countConn->nbClient )
            return;
        
        for ( std::vector< RSPConnection* >::iterator i = 
                    _childrensConnection.begin() ;
              i != _childrensConnection.end(); ++i )
        {
            if ( (*i)->_writerID == countConn->clientID )
                return;
        }
        _addNewConnection( countConn->clientID );
#ifdef WIN32
        SetEvent( _hEvent );
#else
        const char c = SELF_INTERRUPT;
        _selfPipeHEvent->send( &c, 1, true ); 
#endif
        break;
    }
    }//END switch
}

RSPConnection::DataReceive* RSPConnection::_findReceiverWithSequenceID( 
                         const uint32_t sequenceID ) const
{
    // find the correspondig buffer
    for ( std::vector< DataReceive* >::const_iterator k = _buffer.begin(); 
          k != _buffer.end(); ++k )
    {
        if( (*k)->sequenceID == sequenceID )
            return *k;
    }
    return 0;
}

RSPConnection* RSPConnection::_findConnectionWithWriterID( 
                                    const uint32_t writerID ) const
{
    for ( uint32_t i = 0 ; i < _childrensConnection.size(); i++ )
    {
        if ( _childrensConnection[i]->_writerID == writerID )
            return _childrensConnection[i];
    }
    return 0;
}
void RSPConnection::sendNackDatagram ( const uint32_t  toWriterID,
                                       const uint32_t  sequenceID,
                                       const uint8_t   countNack,
                                       const RepeatID* repeatID   ) const
{
    eq::base::Bufferb sendBuffer;
    sendBuffer.resize( _connection->getMTU() );

    // set the header
    DatagramNack* header = reinterpret_cast< DatagramNack* >
                                                ( sendBuffer.getData() );
    DatagramNack headerInit = { NACK, _myID, toWriterID, 
                                sequenceID, countNack };
    *header = headerInit;

    header++;

    memcpy( header, repeatID, countNack * sizeof( RepeatID ) );
    _connection->write( sendBuffer.getData(), _connection->getMTU() );

}
void RSPConnection::_addNewConnection( uint64_t id )
{
    RSPConnection* connection  = new RSPConnection();
    connection->_connection = 0;
    connection->_writerID   = id; 

    _childrensConnection.push_back( connection );
    EQWARN << "New connection " << id << std::endl;
}


int64_t RSPConnection::write( const void* buffer, const uint64_t bytes )
{
    base::ScopedMutex mutex( _mutexConnection );
    _countNbAckInWrite = 0;
    if ( _parentConnection )
        return _parentConnection->write( buffer, bytes );

    if ( _getCountConnection() == 0 )
        return bytes;

    uint32_t size =   EQ_MIN( bytes, _maxBuffer );

#ifdef WIN32
    ResetEvent( _writeEndEvent );
#else
    _selfPipeWriteEventEnd->recvNB( &_selfCommand, sizeof( _selfCommand ));
#endif
    _sequenceIDWrite ++;
    _dataSend = reinterpret_cast< const char* >( buffer );
    _lengthDataSend = size;
    _numberDatagram = size  / _maxLengthDatagramData;
    

    // compute number of datagram
    if ( _numberDatagram * _maxLengthDatagramData != size )
        _numberDatagram++;

    // send each datagram
    for ( uint64_t i = 0; i < _numberDatagram; i++ )
        _sendDatagram( i );
    
    // init all ack receive flag
    for ( std::vector< RSPConnection* >::iterator i = 
             _childrensConnection.begin() ;
              i != _childrensConnection.end(); ++i )
            (*i)->_ackReceive = false;
    
    // send a datagram Ack Request    
    _sendAckRequest();

    if ( _childrensConnection.size() == 0 )
        return size;

    // wait ack from all connection
    _timeEvent = 100;
#ifdef WIN32
    WaitForSingleObject( _writeEndEvent, INFINITE );
#else
    poll( &_writeEndEvent, 1, 9999999 );
    _selfPipeWriteEventEnd->recvSync( 0, 0);
#endif
        
    return size;
}

void RSPConnection::_sendDatagram( const uint64_t idDatagram )
{
    
    uint32_t posInData = _maxLengthDatagramData * idDatagram;
    uint32_t lengthData;
    if ( _lengthDataSend - posInData >= _maxLengthDatagramData )
        lengthData = _maxLengthDatagramData;
    else
        lengthData = _lengthDataSend - posInData;

    const char* data = _dataSend + posInData;

    _sendBuffer.resize( lengthData + sizeof( DatagramData ) );
    
    // set the header
    DatagramData* header = reinterpret_cast< DatagramData* >
                                                ( _sendBuffer.getData() );
    DatagramData headerInit = { DATA, _myID, idDatagram, 
                                _sequenceIDWrite, lengthData };
    *header = headerInit;
    header++;

    memcpy( header, data, lengthData );

    // send Data
    _connection->write ( _sendBuffer.getData(), _sendBuffer.getSize() );
}

void RSPConnection::_sendAckRequest()
{
    const DatagramAckRequest ackRequest = { ACKREQ, _myID, 
                                      _numberDatagram -1, 
                                      _sequenceIDWrite };
    _connection->write( &ackRequest, sizeof( DatagramAckRequest ) );
}
}
}
