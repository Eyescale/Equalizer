#include "RSPConnection.h"

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
static const size_t _numberBuffer = 2;
}

RSPConnection::RSPConnection()
        : _countAcceptChildren( 0 )
        , _indexRead( -1 )
        , _thread ( 0 )
        , _connection( 0 )
        , _parentConnection( 0 )
        , _maxLengthDatagramData( 0 )
        , _currentReadSync( 0 )
        , _timeEvent( 999999999 )
{
    for ( uint8_t i = 0; i < _numberBuffer; i++ )
    {
        DataReceive* receive = new DataReceive();
        receive->_idSequence = 0;
        receive->_ackSend = true;
        receive->_allRead = true;
        receive->_posRead = 0;
        receive->_totalSize = 0;
        
        receive->_boolBuffer.resize( _maxBuffer /  ( UDPConnection::getMTU() - 
                                                  sizeof( DatagramData ) )+1 );

        memset( receive->_boolBuffer.getData() , false, 
                receive->_boolBuffer.getSize() );

        receive->_dataBuffer.resize( _maxBuffer );
        
        _buffer.push_back( receive );
    }
    bufferReceive = _buffer[0];
    _description =  new ConnectionDescription;
    _description->type = CONNECTIONTYPE_MCIP_RSP;
    _description->bandwidth = 102400;

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
    _hEvent = CreateEvent( 0, FALSE, FALSE, 0 );
    _writeEndEvent = CreateEvent( 0, FALSE, FALSE, 0 );
#else
    _selfPipeWriteEventEnd = new PipeConnection;
    if( !_selfPipeWriteEventEnd->connect( ))
    {
        EQERROR << "Could not create connection" << std::endl;
        return false;
    }
    _selfPipeWriteEventEnd->recvNB( &_selfCommand, sizeof( _selfCommand ));

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
    _selfPipeHEvent->recvNB( &_selfCommand, sizeof( _selfCommand ));

    _hEvent.events = POLLIN;
    _hEvent.fd = _selfPipeHEvent->getNotifier();
    _hEvent.revents = 0;

    _udpEvent.events = POLLIN; // | POLLPRI;

    // add self 'connection'
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
        const char c = SELF_INTERRUPT;
        _selfPipeHEvent->send( &c, 1, true );
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
#endif;
    EQASSERT ( _countAcceptChildren < _childrensConnection.size() )
    RSPConnection* newConnection = 
                        _childrensConnection[ _countAcceptChildren ];

    ConnectionPtr  connection( newConnection ); // to keep ref-counting correct

    newConnection->_initAIORead();
    newConnection->_parentConnection = this;
    newConnection->_connection = _connection;
    newConnection->_state       = STATE_CONNECTED;
    newConnection->_description = _description;
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
    for ( std::vector< DataReceive* >::iterator i = _buffer.begin();
              i != _buffer.end(); ++i )
    {
        if ( (*i)->_ackSend && !(*i)->_allRead )
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
#endif

}

eq::net::RSPConnection::DataReceive* RSPConnection::_findReceiverRead()
{
    DataReceive* receiver = _currentReadSync;

    if ( !receiver )
    {
        for ( std::vector< DataReceive* >::iterator i = _buffer.begin();
                  i != _buffer.end(); ++i )
        {
            if( (*i)->_ackSend && !(*i)->_allRead )
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
        int ret = poll( &_udpEvent, 1, -1 );
#endif
        switch ( ret )
        {
        case SELECT_TIMEOUT:
        {
            doReadNb = true;
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
                        EQWARN << "send ACK Requ : " << _idSequenceWrite << std::endl;
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

    const uint64_t size = EQ_MIN( bytes, receive->_totalSize - receive->_posRead );
    const uint8_t* data = receive->_dataBuffer.getData()+ receive->_posRead;
    memcpy( buffer, data, size );
    
    receive->_posRead += size;
    
    // if all data in the buffer has been taken
    if ( receive->_posRead == receive->_totalSize )
    {

        memset( receive->_boolBuffer.getData(), 
                false, receive->_boolBuffer.getSize() * sizeof( bool ));
        receive->_allRead = true;
        _currentReadSync = 0;

    }

    return bytes;

}


void RSPConnection::_read( )
{
    // read datagram 
    const int32_t readSize = _connection->readSync( _bufRead.getData(), 
                                                    _connection->getMTU() );
    
    // read datagram type
    const uint8_t* type = reinterpret_cast<uint8_t*>( _bufRead.getData() ); 
    switch ( *type )
    {
    case DATA: 
    { 
        const DatagramData* datagram = reinterpret_cast< const DatagramData* >
                                       ( _bufRead.getData() );

        if ( _myID == datagram->idwriter )
            return;
        RSPConnection* connection;
        // find connection destination
        for ( uint32_t i = 0 ; i < _childrensConnection.size(); i++ )
        {
            connection = _childrensConnection[i];
            
            if ( connection->_writerId != datagram->idwriter )
                continue;
            
            // if the buffer has ever been found during previous read or last
            // ack data sequence
            if ( connection->bufferReceive )
                break;

            // find the data correspondig buffer 
            // maybe not need this because if we arriving here, there are one
            // situation
            // 1: ack send and we haven't found a new free buffer
            for ( std::vector< DataReceive* >::iterator k = 
                               connection->_buffer.begin();
                  k != connection->_buffer.end(); ++k )
            {
                if( (*k)->_idSequence == datagram->idSequence )
                {
                    connection->bufferReceive = *k;
                    break;
                }
            }

            // if we found the buffer receiver 
            if ( connection->bufferReceive )
                break;

            // Found a free buffer that all data
            for ( std::vector< DataReceive* >::iterator k = 
                               connection->_buffer.begin();
                  k != connection->_buffer.end(); ++k )
            {
                if( (*k)->_allRead )
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
        
        const uint64_t sizeToRead = readSize - sizeof( DatagramData );
                
        // if it's the first datagram 
        if ( connection->bufferReceive->_ackSend )
        {
            if ( datagram->idSequence == connection->bufferReceive->_idSequence )
                return;

            connection->bufferReceive->_idSequence    = datagram->idSequence;
            connection->bufferReceive->_posRead   = 0;
            connection->bufferReceive->_totalSize = 0;
            connection->bufferReceive->_ackSend   = false;
        }

        uint64_t index = datagram->idData;

        // if it's a repetition and we have the data then we ignore it
        if ( !connection->bufferReceive->_boolBuffer.getData()[ index ] )
        {
            datagram++;
            const uint8_t* data = reinterpret_cast< const uint8_t* >
                                                         ( datagram );
            connection->bufferReceive->_boolBuffer.getData()[ index ] = true;
            connection->bufferReceive->_totalSize += sizeToRead;
            const uint64_t pos = ( index ) * ( _connection->getMTU() - 
                                         sizeof( DatagramData ) );
            memcpy( connection->bufferReceive->_dataBuffer.getData() + pos,
                    data, sizeToRead );
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
        if ( ack->idSequence != _idSequenceWrite )
            return;

        RSPConnection* connection = 0;

        // find connection destination and if we have not receive an ack from,
        // we update the ack data.
        for ( std::vector< RSPConnection* >::iterator i = 
                                    _childrensConnection.begin();
              i != _childrensConnection.end(); ++i )
        {
            connection = *i;
            if ( connection->_writerId == ack->readerID )
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
                    // reset counter for send the next sequence of datagram
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
        
        if ( nack->readerId == _myID )
            return;
        
        // TO DO Write Error
        //EQASSERT( nack->idSequence == _idSequenceWrite );
        // understand why we have a nack for bad sequence
        if ( nack->idSequence != _idSequenceWrite )
            return;
        
        RSPConnection* connection = 0;
        // find connection destination and reset counter timeout
        for ( uint32_t i = 0 ; i < _childrensConnection.size(); i++ )
        {
            connection = _childrensConnection[i];
            if ( connection->_writerId == nack->readerId )
            {
                connection->_countTimeOut = 0;
                break;
            }
        }
        uint32_t start = nack->idDataStart;
        uint32_t end = nack->idDataEnd;
        // repeat datagram data
        for (uint32_t i = start; i <= end; i++ )
        {
            connection->_countTimeOut = 0;
            _sendDatagram( i );
        }
        // Send a ack request
        const DatagramAckRequest ackRequest = { ACKREQ, _myID, 
                                      _numberDatagram -1, 
                                      _idSequenceWrite };
        _connection->write( &ackRequest, sizeof( ackRequest ) );
        connection->_countTimeOut = 0;
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
        for ( std::vector< RSPConnection* >::iterator m = _childrensConnection.begin();
              m != _childrensConnection.end(); ++m )
        {
            if( (*m)->_writerId != ackRequest->writerID )
                continue;
            
            connection = *m;

            // find the correspondig buffer
            for ( std::vector< DataReceive* >::iterator k = 
                               (*m)->_buffer.begin();
                  k != (*m)->_buffer.end(); ++k )
            {
                if( (*k)->_idSequence == ackRequest->idSequence )
                {
                    receive = *k;
                    break;
                }
            }

            if ( !receive )
                return;
            
            // Repeat ack
            if ( receive->_ackSend )
            {
                const DatagramAck ack = { ACK, _myID, ackRequest->writerID, 
                                    ackRequest->idSequence };
                _connection->write( &ack, sizeof( ack ) );
                return;
            }
            
            // it strange and must be study for understand
            if ( receive->_idSequence != ackRequest->idSequence )
                continue;

            receive->_boolBuffer.resize( ackRequest->lastDatagramid + 1);
            for ( uint32_t i = 0; i < receive->_boolBuffer.getSize(); i++)
            {
                if ( receive->_boolBuffer.getData()[i] )
                    continue;

                // OPT: Send all NACK packets at once
                for( uint32_t j = i; j < receive->_boolBuffer.getSize();
                     ++j )
                {
                    if( !receive->_boolBuffer.getData()[j] )
                        continue;

                    DatagramNack datagramNack = { NACK, _myID, 
                                          (*m)->_writerId, 
                                          i, j-1,
                                          receive->_idSequence};
                    _connection->write( &datagramNack, 
                                sizeof( DatagramNack ) );
                    EQWARN << "repeat : " << i << " to " << j-1 << std::endl;
                    return;
                }
                const DatagramNack datagramNack = { NACK, _myID, 
                                    connection->_writerId, 
                                    i, receive->_boolBuffer.getSize()-1,
                                    receive->_idSequence };
                _connection->write( &datagramNack, 
                                    sizeof( DatagramNack ) );
                EQWARN << "repeat : " << i << " to " << receive->_boolBuffer.getSize()-1 << std::endl;
                return;
            }
            
            // buffer is read
            receive->_ackSend = true;
            receive->_allRead = false;
            connection->bufferReceive = 0;
            
            // Found a free buffer for the next receive
            for ( std::vector< DataReceive* >::iterator k = 
                               connection->_buffer.begin();
                  k != connection->_buffer.end(); ++k )
            {
                if( (*k)->_allRead )
                {
                    connection->bufferReceive = *k;
                    break;
                }
            }

                    // ack data
            DatagramAck ack = { ACK, _myID, connection->_writerId, 
                                receive->_idSequence };
            _connection->write( &ack, sizeof( ack ) );
#ifdef WIN32
            SetEvent( connection->_hEvent );
#else
            const char c = SELF_INTERRUPT;
            connection->_selfPipeHEvent->send( &c, 1, true );
#endif
            break;
        }
        break;
    }
    case NEWNODE:
    {
        
        base::ScopedMutex mutex( _mutexConnection );
        const DatagramNode* node = reinterpret_cast< const DatagramNode* >
                                                       (  _bufRead.getData()  );

        if ( node->idConnection != _myID )
        {
            for ( std::vector< RSPConnection* >::iterator i = 
                   _childrensConnection.begin() ; 
                   i != _childrensConnection.end(); ++i )
            {
                if ( (*i)->_writerId == node->idConnection )
                    return;
            }
            _addNewConnection( node->idConnection );
        }
#ifdef WIN32
        SetEvent( _hEvent );
#else
        const char c = SELF_INTERRUPT;
        _selfPipeHEvent->send( &c, 1, true );
#endif
        break;
    }
    case EXITNODE:
    {
        
        // later
        const DatagramNode* node = reinterpret_cast< const DatagramNode* >
                                                         ( _bufRead.getData() );
        EQWARN << "remove Connection " << node->idConnection << std::endl;
        if ( node->idConnection != _myID )
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
            if ( (*i)->_writerId == node->idConnection )
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
        base::ScopedMutex mutex( _mutexConnection );
        const DatagramCountConnection* countConn = 
                reinterpret_cast< const DatagramCountConnection* >
                                                         ( _bufRead.getData() );
        
        if( countConn->idClient == _myID )
            return;
        // we know all connection
        if ( _childrensConnection.size() == countConn->nbClient )
            return;
        
        for ( std::vector< RSPConnection* >::iterator i = 
                    _childrensConnection.begin() ;
              i != _childrensConnection.end(); ++i )
        {
            if ( (*i)->_writerId == countConn->idClient )
                return;
        }
        _addNewConnection( countConn->idClient );    
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


void RSPConnection::_addNewConnection( uint64_t id )
{
    RSPConnection* connection  = new RSPConnection();
    connection->_connection = 0;
    connection->_writerId   = id; 

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
    //* NOP */
#endif
    _idSequenceWrite ++;
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

    sendBuffer.resize( lengthData + sizeof( DatagramData ) );
    
    // set the header
    DatagramData* header = reinterpret_cast< DatagramData* >
                                                ( sendBuffer.getData() );
    DatagramData headerInit = { DATA, _myID, idDatagram, _idSequenceWrite };
    *header = headerInit;
    header++;

    memcpy( header, data, lengthData );

    // send Data
    _connection->write ( sendBuffer.getData(), sendBuffer.getSize() );
}

void RSPConnection::_sendAckRequest()
{
    const DatagramAckRequest ackRequest = { ACKREQ, _myID, 
                                      _numberDatagram -1, 
                                      _idSequenceWrite };
    _connection->write( &ackRequest, sizeof( DatagramAckRequest ) );
}
}
}
