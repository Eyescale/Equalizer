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

#ifndef INFINITE
#  define INFINITE -1
#endif

namespace eq
{
namespace net
{

namespace
{
static const size_t _maxBuffer = (UDPConnection::getMTU() - 
                                sizeof( RSPConnection::DatagramData )) * 50;
static const size_t _numberBuffer = 4;
static const size_t _maxDatagram = _maxBuffer /  ( UDPConnection::getMTU() - 
                                     sizeof( RSPConnection::DatagramData ) )+1;
static const size_t _maxNack = ( UDPConnection::getMTU() - 
                                sizeof( RSPConnection::DatagramNack ) ) / 
                                              sizeof( uint32_t );
static const  size_t _maxLengthDatagramData = UDPConnection::getMTU() - 
                                          sizeof( RSPConnection::DatagramData );
}

RSPConnection::RSPConnection()
        : _countAcceptChildren( 0 )
        , _writing( false )
        , _thread ( 0 )
        , _lastSequenceIDAck( -1 )
        , _countReadReceiver ( 0 )
        , _countWriteReceiver ( 0 )
{
    _description->type = CONNECTIONTYPE_RSP;
    _description->bandwidth = 102400;

    for( size_t i = 0; i < _numberBuffer; i++ )
        _buffer.push_back( new DataReceive() );

    _bufferReceive = _buffer[ _countWriteReceiver ];
    _sibling = this;
}

RSPConnection::DataReceive::DataReceive()
{
    reset();
}

void RSPConnection::DataReceive::reset() 
{

    sequenceID = 0;
    ackSend    = true;
    allRead    = true;
    posRead    = 0;
    boolBuffer.resize( _maxDatagram );

    memset( boolBuffer.getData(), false,
            boolBuffer.getSize() );

    dataBuffer.resize( _maxBuffer );
}

void RSPConnection::close()
{
    if( _state == STATE_CLOSED )
        return;

    _state = STATE_CLOSED;

    if( _thread )
    {
        const DatagramNode exitNode ={ EXITNODE, _getID() };
        _connection->write( &exitNode, sizeof( DatagramNode ) );
        _connectionSet.interrupt();
        _thread->join();
    }

    for ( std::vector< RSPConnectionPtr >::iterator i = _childs.begin();
        i != _childs.end(); i++ )
    {
        RSPConnectionPtr connection = *i;
        connection = 0;
    }

    _parent = 0;
    if( _connection.isValid( ))
    {
        _connection->close();
    }
    _connection = 0;

    for( std::vector< DataReceive* >::iterator i = _buffer.begin(); 
         i < _buffer.end(); i++ )
    {
        DataReceive* buffer = *i;
        buffer->reset();
    }
    _bufferReceive = _buffer[0];

    _fireStateChanged();
}

RSPConnection::~RSPConnection()
{
    close();

    _sibling = 0;
    _bufferReceive = 0;
    
    for( std::vector< DataReceive* >::iterator i = _buffer.begin(); 
         i < _buffer.end(); i++ )
    {
        DataReceive* buffer = *i;
        delete buffer;
    }

    _buffer.clear();
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


RSPConnection::IDConnectionType RSPConnection::_buildNewID()
{
    _myID = _rng.get<IDConnectionType>();
    _myIDShift = static_cast< IDConnectionType >( _myID ) << 
                                             ( sizeof( IDConnectionType ) * 8 );
    return _myID;
}

bool RSPConnection::listen()
{
    EQASSERT( _description->type == CONNECTIONTYPE_RSP );

    if( _state != STATE_CLOSED )
        return false;
    
    _state = STATE_CONNECTING;
    _fireStateChanged();

    // init an udp Connection
    _connection = new UDPConnection();
    ConnectionDescriptionPtr description = 
        new ConnectionDescription( *_description.get( ));
    description->type = CONNECTIONTYPE_UDP;
    _connection->setDescription( description );

    // connect UDP multicast
    if( !_connection->connect() )
    {
        EQWARN << "can't connect RSP transmission " << std::endl;
        return false;
    }

    _connectionSet.addConnection( _connection->getSibling() );

    // init a thread for manage the communication protocol 
    _thread = new Thread( this );

#ifdef WIN32
    _initAIOAccept();
#else
    _countNbAckInWrite =  0;

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
#endif
    _countNbAckInWrite = 0;

    _buildNewID();
    
    _bufRead.resize( _connection->getMTU( ));
    // wait until protcol RSP establish connection to the multicast network
    
    // start the working protocol RSP 
    if( !_thread->start( ))
    {
        close();
        return false;
    }

    _state = STATE_LISTENING;
    _fireStateChanged();


    EQINFO << "Connected on " << _description->getHostname() 
           << _description->port << " (" << _description->toString() << ")"
           << std::endl;
    return true;
}

void RSPConnection::acceptNB()
{
    EQASSERT( _state == STATE_LISTENING );
}

ConnectionPtr RSPConnection::getSibling()
{
    return _sibling.get();
}

ConnectionPtr RSPConnection::acceptSync()
{
    CHECK_THREAD( _recvThread );
    if( _state != STATE_LISTENING )
        return 0;
    {
        const base::ScopedMutex mutexConn( _mutexConnection );
        if ( _countAcceptChildren == _childs.size() )
#ifdef WIN32
            WaitForSingleObject( _hEvent, INFINITE );
#else
            poll( &_hEvent, 1, INFINITE );
#endif
    }
    EQASSERT ( _countAcceptChildren < _childs.size() )
    
    RSPConnectionPtr newConnection = 
                        _childs[ _countAcceptChildren ];
    
    ConnectionPtr  connection = newConnection->getSibling(); 

    newConnection->_initAIORead();
    newConnection->_parent      = this;
    newConnection->_connection  = 0;
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
                        { COUNTNODE, _getID(), _childs.size() };
    _connection->write( &countNode, sizeof( DatagramCountConnection ) );
    
    EQINFO << "accepted connection " << (void*)newConnection.get()
           << std::endl;
    const base::ScopedMutex mutexConn( _mutexConnection );
    if ( _childs.size() >= _countAcceptChildren )
#ifdef WIN32
        ResetEvent( _hEvent );
#else
        _selfPipeHEvent->recvNB( &_selfCommand, sizeof( _selfCommand ));
#endif
    return connection;
}


int64_t RSPConnection::readSync( void* buffer, const uint64_t bytes )
{
    const uint32_t size =   EQ_MIN( bytes, _maxBuffer );
    DataReceive* receiver = _buffer[ _countReadReceiver ];
    
    receiver->ackSend.waitEQ( true );
    receiver->allRead.waitEQ( false );
    
    return _readSync( receiver, buffer, size );
}

bool RSPConnection::Thread::init()
{
    return _connection->_init();
}

bool RSPConnection::_init()
{
    _connection->readNB( _bufRead.getData(), _connection->getMTU() );

    // send a first datgram for annunce me and discover other connection 
    const DatagramNode newnode ={ ACKREQNEWNODE, _getID() };
    _connection->write( &newnode, sizeof( DatagramNode ) );
    uint8_t countTimeOut = 0;
    while ( true )
    {
        switch ( _connectionSet.select( 100 ) )
        {
            case ConnectionSet::EVENT_TIMEOUT:
                if ( countTimeOut < 10 )
                {
                    countTimeOut++;
                    const DatagramNode ackNode ={ ACKREQNEWNODE, _getID() };
                    _connection->write( &ackNode, sizeof( DatagramNode ) );
                }
                else
                {
                    const DatagramNode confirmNode ={ CONFIRMNODE, _getID() };
                    _connection->write( &confirmNode, sizeof( DatagramNode ) );
                    _addNewConnection( _getID() );
                    //_connection->setMulticastLoop( true );
                    return true;
                }
                break;
            case ConnectionSet::EVENT_DATA:
                countTimeOut = 0;
                if ( !_handleInitData() )
                {
                    EQERROR << " Error during Read UDP Connection" 
                            << std::endl;
                    return false;
                }
                else
                    _connection->readNB( _bufRead.getData(), 
                                         _connection->getMTU());
                break;
            case ConnectionSet::EVENT_INTERRUPT:
                
                _connection->close();
                _connection = 0;
                return false;

            default: break;

        }
    }
}

bool RSPConnection::_handleInitData()
{
    // read datagram 
    if( _connection->readSync( _bufRead.getData(), 
                               _connection->getMTU() ) == -1 )
    {
        EQERROR << "Error read on Connection UDP" << std::endl;
        return false;
    }

    // read datagram type
    const uint8_t* type = reinterpret_cast<uint8_t*>( _bufRead.getData() );
    const DatagramNode* node = reinterpret_cast< const DatagramNode* >
                                                       (  _bufRead.getData()  );
    switch ( *type )
    {
    case ACKREQNEWNODE:
        return _acceptNewIDConnection( node->connectionID ) ;

    case NACKNEWNODE:
        // a connection refused my ID, ask for another ID
        if ( node->connectionID == _myID )
        {
             const DatagramNode newnode ={ ACKREQNEWNODE, _buildNewID() };
             _connection->write( &newnode, sizeof( DatagramNode ) );
        }
        return true;

    case CONFIRMNODE:
        return _addNewConnection( node->connectionID );

    case EXITNODE:
        return _acceptRemoveConnection( node->connectionID );
    default: break;
    
    }//END switch
    return true;

}

void* RSPConnection::Thread::run()
{
    _connection->_run();
    _connection = 0;
    return 0;
}

void RSPConnection::_run()
{

    //_connection->readNB( _bufRead.getData(), _connection->getMTU() );
    while ( true )
    {
        const int timeOut = _writing == true ? 100 : -1;

        switch ( _connectionSet.select( timeOut ) )
        {
        case ConnectionSet::EVENT_TIMEOUT:
        {
            for ( std::vector< RSPConnectionPtr >::iterator i = _childs.begin() ;
                  i != _childs.end(); ++i )
            {
                RSPConnectionPtr connection = *i;
                if ( connection->_ackReceive )
                    continue;

                connection->_countTimeOut++;
                // a lot of more time out
                if ( connection->_countTimeOut >= 100 )
                {
                    EQERROR << "Several timeouts from RSP communication " 
                            << std::endl;
                   // _connection = 0;
                   // return;
                }

                // send a datagram Ack Request    
                _sendAckRequest();

                break;
                
            }
            break;
        }
        case ConnectionSet::EVENT_DATA:
        {
            if ( !_handleData() )
            {
                EQERROR << " Error during Read UDP Connection" 
                        << std::endl;
               _connectionSet.interrupt();
            }
            else
               _connection->readNB( _bufRead.getData(), _connection->getMTU());
            break;
        }
        case ConnectionSet::EVENT_INTERRUPT:
        {
            _connection = 0;
            return;
        }
        default: return;
        }
    }
}

int64_t RSPConnection::_readSync( DataReceive* receive, 
                                  void* buffer, 
                                  const uint64_t bytes )
{
    const uint64_t size = EQ_MIN( bytes, receive->dataBuffer.getSize() - 
                                         receive->posRead );
    const uint8_t* data = receive->dataBuffer.getData()+ receive->posRead;
    memcpy( buffer, data, size );

    receive->posRead += size;
    
    // if all data in the buffer has been taken
    if ( receive->posRead == receive->dataBuffer.getSize() )
    {
        memset( receive->boolBuffer.getData(), 
                false, receive->boolBuffer.getSize() * sizeof( bool ));
        
        _countReadReceiver = ( _countReadReceiver + 1 ) % _numberBuffer;
        
        {
            const base::ScopedMutex mutexEvent( _mutexEvent );
            if ( !( _buffer[ _countReadReceiver ]->ackSend.get() && 
                  !_buffer[ _countReadReceiver ]->allRead.get()) )
            {
#ifdef WIN32
                ResetEvent( _hEvent );
#else
                _selfPipeHEvent->recvNB( &_selfCommand, sizeof( _selfCommand ));
#endif
            }
            receive->dataBuffer.setSize( 0 );
            receive->allRead = true;
        }
    }

    return bytes;
}

bool RSPConnection::_handleData( )
{
    // read datagram 
    if( _connection->readSync( _bufRead.getData(), 
                               _connection->getMTU() ) == -1 )
    {
        EQERROR << "Error read on Connection UDP" << std::endl;
        return false;
    }

    // read datagram type
    const uint8_t* type = reinterpret_cast<uint8_t*>( _bufRead.getData() ); 
    switch ( *type )
    {
    case DATA:

        return RSPConnection::_handleDataDatagram( 
            reinterpret_cast< const DatagramData* >( _bufRead.getData() ) );
    
    case ACK:

        return _handleAckDatagram( 
                  reinterpret_cast< const DatagramAck* >( type )) ;
    
    case NACK:

        return _handleNackDatagram( reinterpret_cast< const DatagramNack* >
                                                        ( _bufRead.getData() ));
    
    case ACKREQ: // The writer ask for a ack data
        
        return _handleAckRequDatagram(
                reinterpret_cast< const DatagramAckRequest* >( type ));
    
    case ACKREQNEWNODE:
    {
        const DatagramNode* node = reinterpret_cast< const DatagramNode* >
                                                       (  _bufRead.getData() );
        return _acceptNewIDConnection( node->connectionID );
    }
    case CONFIRMNODE:
    {
        const DatagramNode* node = reinterpret_cast< const DatagramNode* >
                                                       (  _bufRead.getData() );
        return _addNewConnection( node->connectionID );
    }
    case EXITNODE:
    {
        const DatagramNode* node = reinterpret_cast< const DatagramNode* >
                                                       (  _bufRead.getData()  );
        return _acceptRemoveConnection( node->connectionID );
    }
    case COUNTNODE:
    {

        /* datagram CONFIRMNODE 
           if it's my ID 
               NOP
           else
               build data structure DataReceive
               setEvent for accept waiting
               send a datagram CountNode
        */
        
        const DatagramCountConnection* countConn = 
                reinterpret_cast< const DatagramCountConnection* >
                                                         ( _bufRead.getData() );
        
        // we know all connection
        if ( _childs.size() == countConn->nbClient )
            return true;

        RSPConnectionPtr connection = 
                            _findConnectionWithWriterID( countConn->clientID );

        if ( !connection.isValid() )
        { 

            _addNewConnection( countConn->clientID );
        }
        break;
    }
    default: break;
    }//END switch
    return true;
}

bool RSPConnection::_handleDataDatagram( const DatagramData* datagram )
{
    uint32_t datagramWriterID   = datagram->writeSeqID >> 
                                       ( sizeof( IDConnectionType ) * 8 );
    
    uint32_t datagramSequenceID = datagram->writeSeqID  & 0xFFFF ;
    
    RSPConnectionPtr connection = 
                            _findConnectionWithWriterID( datagramWriterID );
    
    // it's an unknown connection or when we run netperf client before
    // server netperf
    // TO DO find a solution in this situation
    if ( !connection.isValid() )
    {
        EQUNREACHABLE;
        return false;
    }

    // if the buffer hasn't been found during previous read or last
    // ack data sequence.
    // find the data correspondig buffer 
    // why we haven't a receiver here:
    // 1: it's a reception data for another connection
    // 2: all buffer was not ready during last ack data
    if ( !connection->_bufferReceive )
    {
        connection->_bufferReceive = 
            connection->_findReceiverWithSequenceID( datagramSequenceID );
        // if needed set the vector length
        if ( connection->_bufferReceive )
        {
            if ( connection->_buffer[ 
                          connection->_countWriteReceiver ]->allRead.get() )
                connection->_bufferReceive = 
                     connection->_buffer[ connection->_countWriteReceiver ];
        }
        else
        {
        // we do not have a free buffer, which means that the receiver is
        // slower then our read thread. This should not happen, because now
        // we'll drop the data and will send a full NAK packet upon the 
        // Ack request, causing retransmission even though we'll drop it
        // again
            EQWARN << "Reader to slow, dropping data" << std::endl;
            return false;
        }
    }

    DataReceive* receive = connection->_bufferReceive;

    // if it's the first datagram 
    if( receive->ackSend.get() )
    {
        if ( datagramSequenceID == receive->sequenceID )
            return true;

        // if it's a datagram repetition for an other connection, we have
        // to ignore it
        if ( ( connection->_lastSequenceIDAck == datagramSequenceID ) &&
             connection->_lastSequenceIDAck != -1 )
            return true;

        receive->sequenceID = datagramSequenceID;
        receive->posRead   = 0;
        receive->dataBuffer.setSize( 0 );
        receive->ackSend   = false;
    }

    uint64_t index = datagram->dataIDlength >> 16;

    // if it's a repetition and we have the data then we ignore it
    if ( receive->boolBuffer.getData()[ index ] )
        return true;

    const uint16_t length = datagram->dataIDlength & 0xFFFF ; 
    const uint8_t* data = reinterpret_cast< const uint8_t* >( ++datagram );
    receive->boolBuffer.getData()[ index ] = true;
    if ( ( index * _maxLengthDatagramData + length ) > receive->dataBuffer.getSize() )
        receive->dataBuffer.resize( index * _maxLengthDatagramData + length );
    const uint64_t pos = ( index ) * ( _connection->getMTU() - 
                                       sizeof( DatagramData ) );
    
    memcpy( receive->dataBuffer.getData() + pos, data, length );

    // control if the previous datagrams have been received
    if ( index <= 0 ) 
        return true;
    
    if ( receive->boolBuffer.getData()[ index - 1 ] )
        return true;
    
    const uint16_t indexMax = index-1;
    uint16_t indexMin = indexMax;
    for( ; indexMin == 0; indexMin-- )
    {
        if ( !receive->boolBuffer.getData()[ indexMin ] )
            continue;
            
        indexMin++;
        break;
    }
    const uint32_t repeatID = indexMax | ( indexMin << 16 ) ; 
    
    sendNackDatagram ( connection->_writerID, datagramSequenceID, 1, &repeatID);
    return true;
}

bool RSPConnection::_handleAckDatagram( const DatagramAck* ack )
{
    // it's not a ack corresponding to my write sequence
    if ( ack->sequenceID != _sequenceIDWrite && ack->writerID != _getID() )
        return true;

    // find connection destination and if we have not receive an ack from,
    // we update the ack data.
    RSPConnectionPtr connection = _findConnectionWithWriterID( 
                                                    ack->readerID );

    if ( !connection.isValid() )
    {
        EQUNREACHABLE;
        return false;
    }

    // if I have receive ack previously from the reader
    if ( connection->_ackReceive )
        return true;

    connection->_ackReceive = true;
    _countNbAckInWrite++;

    if ( _countNbAckInWrite != _childs.size()-1 )
        return true;

    // unblock write function if all ack have been received
    _writing = false;

    // reset counter timeout
    connection->_countTimeOut = 0;

    return true;
}

bool RSPConnection::_handleNackDatagram( const DatagramNack* nack )
{
    RSPConnectionPtr connection = 
                              _findConnectionWithWriterID( nack->readerID );

    
    if ( connection->_ackReceive )
        return true;

    // it's not a nack for my write sequence. it's for an other writer 
    if ( nack->sequenceID != _sequenceIDWrite && 
         nack->writerID != _getID() )
        return true;

    if ( connection.isValid() )
    {
        connection->_countTimeOut = 0;
    }
    else
    {
        EQUNREACHABLE;
        return false;
        // it's an unknow connection 
        // TO DO we may be add this connection 
    }

    // repeat the ask datagram
    const uint8_t countRepeatID = nack->countRepeatID;
    nack++;
    const uint32_t* repeatID = reinterpret_cast< const uint32_t* >( nack );
    for ( uint8_t j = 0; j < countRepeatID; j++ )
    {
        const uint32_t start = ( *repeatID & 0xFFFF0000) >> 16;
        uint32_t end   = ( *repeatID & 0xFFFF );

        uint32_t writSeqID = _myIDShift | _sequenceIDWrite;

        // repeat datagram data
        for ( uint16_t i = start; i <= end; i++ )
            _sendDatagram( writSeqID, i );
        repeatID++;
    }

    // Send a ack request
    const DatagramAckRequest ackRequest = { ACKREQ, _myID, 
                                  _numberDatagram -1, 
                                  _sequenceIDWrite };
    _connection->write( &ackRequest, sizeof( ackRequest ) );
    return true;
}

bool RSPConnection::_handleAckRequDatagram( 
                      const DatagramAckRequest* ackRequest )
{

    RSPConnectionPtr connection = 
                            _findConnectionWithWriterID( ackRequest->writerID );

    if ( !connection.isValid() )
    {
        EQUNREACHABLE;
        return false;
    }
    // find the corresponding buffer
    DataReceive* receive = connection->_findReceiverWithSequenceID( 
                                               ackRequest->sequenceID );
    
    // Why no receiver found ?
    // 1 : all datagram data has not been receive ( timeout )
    // 2 : all receiver are full and not ready for receive datagramData
    //    We ask for resend all the datagrams
    if ( !receive )
    {
        const uint32_t repeatID = ackRequest->lastDatagramID; 
        sendNackDatagram ( connection->_writerID, ackRequest->sequenceID,
                           1, &repeatID );
        return true;
    }
    
    // Repeat ack
    if ( receive->ackSend.get() )
    {
        const DatagramAck ack = { ACK, _myID, ackRequest->writerID, 
                                  ackRequest->sequenceID };
        _connection->write( &ack, sizeof( ack ) );
        return true;
    }
    
    // find all lost datagramData
    receive->boolBuffer.resize( ackRequest->lastDatagramID + 1);
    eq::base::Buffer< uint32_t > bufferRepeatID;
    for ( uint32_t i = 0; i < receive->boolBuffer.getSize(); i++)
    {
        // size max datagram = mtu
        if ( _maxNack <= bufferRepeatID.getSize() )
            break;

        if ( receive->boolBuffer.getData()[i] )
            continue;
        
        const uint32_t start = i << 16;
        uint32_t end = receive->boolBuffer.getSize()-1;

        // OPT: Send all NACK packets at once
        for( ; i < receive->boolBuffer.getSize(); i++ )
        {
            if( !receive->boolBuffer.getData()[i] )
                continue;
            end = i-1;
            break;
        }
        const uint32_t repeatID = end | start ; 
        bufferRepeatID.append( repeatID );
    }
    
    // send datagram NACK
    if ( bufferRepeatID.getSize() > 0 )
    {
        sendNackDatagram ( connection->_writerID,
                           ackRequest->sequenceID,
                           bufferRepeatID.getSize(),
                           bufferRepeatID.getData() );
        return true;
    }
    
    // no repeat needed, we send an ack and we prepare the next buffer 
    // receive.
    connection->_bufferReceive = 0;
    
    // Found a free buffer for the next receive
    connection->_countWriteReceiver = 
                    ( connection->_countWriteReceiver + 1 ) % _numberBuffer;
    if ( connection->_buffer[ 
                          connection->_countWriteReceiver ]->allRead.get() )
        connection->_bufferReceive = 
                 connection->_buffer[ connection->_countWriteReceiver ];

    // ack data
    const DatagramAck ack = { ACK, _myID, connection->_writerID, 
                    receive->sequenceID };
    _connection->write( &ack, sizeof( ack ) );
    connection->_lastSequenceIDAck = receive->sequenceID;
    {
        const base::ScopedMutex mutexEvent( _mutexEvent );
#ifdef WIN32
        SetEvent( connection->_hEvent );
#else
        const char c = SELF_INTERRUPT;
        connection->_selfPipeHEvent->send( &c, 1, true );
#endif
        // the receiver is ready and can be read by ReadSync
        receive->ackSend = true;
        receive->allRead = false;
    }

    return true;
}

bool RSPConnection::_acceptNewIDConnection( IDConnectionType id )
{
    /* datagram ACKREQNEWNODE 
       if it's an unknow ID 
           NOP
       else
           send NACKNEWNODE
    */

    // the ID is same as mine 
    // how to detect if its my own send ?
    //   set ip_multicast_loop to false during connection period
    if ( id == _myID )
    {
        DatagramNode nodeSend = { NACKNEWNODE, _myID };
        _connection->write( &nodeSend, sizeof( DatagramNode ) );
        return true;
    }
    
    // look if the new ID exist in other connection
    RSPConnectionPtr child = _findConnectionWithWriterID( id );
    
    if ( child.isValid() )
    {
        DatagramNode nodeSend = { NACKNEWNODE, id };
        _connection->write( &nodeSend, sizeof( DatagramNode ) );
    }
    
    return true;
}

RSPConnection::DataReceive* RSPConnection::_findReceiverWithSequenceID( 
                         const IDSequenceType sequenceID ) const
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

RSPConnection::RSPConnectionPtr RSPConnection::_findConnectionWithWriterID( 
                                    const IDConnectionType writerID )
{
    //if ( writerID == _getID() )
    //    return this;

    for ( uint32_t i = 0 ; i < _childs.size(); i++ )
    {
        if ( _childs[i]->_writerID == writerID )
            return _childs[i];
    }
    return 0;
}


bool RSPConnection::_addNewConnection( IDConnectionType id )
{
    //if ( id == _myID )
    //    return true;

    if ( _findConnectionWithWriterID( id ).isValid() )
    {
        EQUNREACHABLE;
        return false;
    }

    RSPConnection* connection  = new RSPConnection();
    connection->_connection    = 0;
    connection->_writerID      = id;

    // protect the event and child size which can be use at the same time 
    // in acceptSync
    {
        const base::ScopedMutex mutexConn( _mutexConnection );
        _childs.push_back( connection );
        EQWARN << "New connection " << id << std::endl;

#ifdef WIN32
        SetEvent( _hEvent );
#else
        const char c = SELF_INTERRUPT;
        _selfPipeHEvent->send( &c, 1, true );
#endif
    }
    _sendDatagramCountNode();
    return true;
}

bool RSPConnection::_acceptRemoveConnection( const IDConnectionType id )
{
    EQWARN << "remove Connection " << id << std::endl;
    if ( id != _myID )
    {
        _sendDatagramCountNode();
        return true;
    }

    for ( std::vector< RSPConnectionPtr >::iterator i = _childs.begin(); 
          i != _childs.end(); i++ )
    {
        RSPConnectionPtr child = *i;
        if ( child->_writerID == id )
        {
            child->close();
            _childs.erase( i );
        }
    }

    _sendDatagramCountNode();
    return true;
}

int64_t RSPConnection::write( const void* buffer, const uint64_t bytes )
{
    if( _state != STATE_CONNECTED && _state != STATE_LISTENING )
        return -1;
    const uint32_t size =   EQ_MIN( bytes, _maxBuffer );
    const base::ScopedMutex mutex( _mutexConnection );

    _writing = true;
    _countNbAckInWrite = 0;

    if ( _parent.isValid() )
        return _parent->write( buffer, bytes );

    if ( _getCountConnection() == 0 )
        return bytes;
    
    

    _sequenceIDWrite ++;
    _dataSend = reinterpret_cast< const char* >( buffer );
    _lengthDataSend = size;
    _numberDatagram = size  / _maxLengthDatagramData;
    
    // compute number of datagram
    if ( _numberDatagram * _maxLengthDatagramData != size )
        _numberDatagram++;

    uint32_t writSeqID = _myIDShift | _sequenceIDWrite;

    // send each datagram
    for ( uint16_t i = 0; i < _numberDatagram; i++ )
        _sendDatagram( writSeqID, i );

    // init all ack receive flag
    for ( std::vector< RSPConnectionPtr >::iterator i = 
             _childs.begin() ;
              i != _childs.end(); ++i )
    {
        (*i)->_countTimeOut = 0;
        (*i)->_ackReceive = false;
    }

    // send a datagram Ack Request    
    _sendAckRequest();

    // wait ack from all connection
    _writing.waitEQ( false );

    return size;
}

void RSPConnection::_sendDatagramCountNode()
{
    const DatagramCountConnection countNode = 
                       { COUNTNODE, _getID(), _childs.size() -1 };
    _connection->write( &countNode, sizeof( DatagramCountConnection ) );
}

void RSPConnection::sendNackDatagram ( const IDConnectionType  toWriterID,
                                       const IDSequenceType  sequenceID,
                                       const uint8_t   countNack,
                                       const uint32_t* repeatID   )
{
    eq::base::Bufferb sendBuffer;
    sendBuffer.resize( _connection->getMTU() );

    // set the header
    DatagramNack* header = reinterpret_cast< DatagramNack* >
                                                ( sendBuffer.getData() );
    const DatagramNack headerInit = { NACK, _myID, toWriterID, 
                                sequenceID, countNack };
    *header = headerInit;

    header++;

    memcpy( header, repeatID, countNack * sizeof( uint32_t ) );
    _connection->write( sendBuffer.getData(), _connection->getMTU() );
}

void RSPConnection::_sendDatagram( const uint32_t writSeqID, 
                                   const uint16_t idDatagram )
{
    const uint32_t posInData = _maxLengthDatagramData * idDatagram;
    uint16_t lengthData;
    
    if ( _lengthDataSend - posInData >= _maxLengthDatagramData )
        lengthData = _maxLengthDatagramData;
    else
        lengthData = _lengthDataSend - posInData;
    
    const char* data = _dataSend + posInData;

    _sendBuffer.resize( lengthData + sizeof( DatagramData ) );
    
    uint32_t dataIDlength = ( idDatagram << 16 ) | lengthData;

    // set the header
    DatagramData* header = reinterpret_cast< DatagramData* >
                                                ( _sendBuffer.getData() );
    DatagramData headerInit = { DATA, writSeqID, dataIDlength };
    *header = headerInit;

    memcpy( ++header, data, lengthData );

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

