#include "rspConnection.h"

#include "connection.h"
#include "connectionDescription.h"



namespace eq
{
namespace net
{

namespace
{
static const size_t _maxBuffer = 1454*1000;
}

RSPConnection::RSPConnection()
        : _indexRead( -1 )
        , _connection( 0 )
{
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

bool RSPConnection::connect()
{
    if( _state != STATE_CLOSED )
        return false;
    
    _state = STATE_CONNECTING;
    _fireStateChanged();

    _connection = new UDPConnection();
    ConnectionDescription* description = 
        new ConnectionDescription( *_description );
    description->type = CONNECTIONTYPE_UDP;
    _connection->setDescription( description );
    _description->type = CONNECTIONTYPE_MCIP_RSP;

    if ( !_connection->connect() )
    {
        EQWARN << "can't connect RSP transmission " << std::endl;
        return false;
    }

    _thread = new Thread( this );

#ifdef WIN32
    _hEvent = CreateEvent( 0, FALSE, FALSE, 0 );
    _writeEndEvent = CreateEvent( 0, FALSE, FALSE, 0 );
#else

#endif
     _countNbAckInWrite =  0;

#ifndef WIN32
    //_eventUDP.events  = POLLIN; 
    //_eventUDP.fd = _connection->getNotifier();
#endif
    _myID = _rng.get<uint32_t>();
    _bufRead.resize( _connection->getMTU() );
    
    _thread->start();
    const DatagramNode newnode ={ NEWNODE, _getID() };
    _connection->write( &newnode, sizeof( DatagramNode ) );

    _state = STATE_CONNECTED;
    _fireStateChanged();

    return true;
}

void RSPConnection::readNB( void* buffer, const uint64_t bytes )
{
    _updateEvent();
}

int64_t RSPConnection::readSync( void* buffer, const uint64_t bytes )
{
    uint64_t sizeRead = 0;
    uint32_t size = EQ_MIN( bytes, _maxBuffer );
    while ( sizeRead != size )
    {
        sizeRead = _readSync( buffer, size );
        _updateEvent();
        if ( sizeRead != size )
        {
            
#ifdef WIN32
            WaitForSingleObject( getNotifier(), INFINITE );
#else
            //poll( &_thread->_eventUDP, 1, -1 );
#endif
        }
    }
    return sizeRead;
}

void RSPConnection::_run()
{
    while ( true )
    {
        _connection->readNB( _bufRead.getData(), _connection->getMTU() );
#ifdef WIN32
        WaitForSingleObject( _connection->getNotifier(), INFINITE );
#else
       // poll( &_eventUDP, 1, -1 );
#endif
        _read();
    }
}

void* RSPConnection::Thread::run()
{
    _connection->_run();
    return 0;
}

void RSPConnection::_updateEvent()
{
    // ToDo use an event count
    for ( uint32_t i = 0; i < clientsRSP.size(); i++ )
    {
        ClientRSP* client = clientsRSP[i];
        if ( !client->ackSend )
            continue;
        if ( !client->allRead )
        {
#ifdef WIN32
            SetEvent( _hEvent );
#else
            // interrupt
#endif
            return;
        }
    }
#ifdef WIN32
    ResetEvent( _hEvent );
#else

#endif
}

int64_t RSPConnection::_readSync( void* buffer, const uint64_t bytes )
{
    
    int indexRead = -1;

    // if no current buffer 
    if ( _indexRead == -1 )
    {
        // find a buffer which be ready to read
        for ( uint32_t i = 0; i < clientsRSP.size(); i++ )
        {
            const ClientRSP* client = clientsRSP[i];
            if ( !client->ackSend )
                continue;
            if ( !client->allRead  )
                indexRead = i;
        }
    }
    else
        indexRead = _indexRead;
    
    if ( indexRead == -1 )
        return 0;

    ClientRSP* client = clientsRSP[ indexRead ];

    const uint64_t size = EQ_MIN( bytes, client->totalSize - client->posRead );
    const uint8_t* data = client->dataBuffer.getData()+ client->posRead;
    memcpy( buffer, data, size );
    
    client->posRead += size;
    // if all data uin the buffer has been taken
    if ( client->posRead == client->totalSize )
    {
        memset( client->boolBuffer.getData(), 
                false, client->boolBuffer.getSize() * sizeof( bool ));
        client->allRead = true;
        _indexRead = -1;
        // ack data
        DatagramAck ack = { ACK, _myID, client->writerId, 
                            client->idSequence };
        client->lastidSequenceAck = client->idSequence;
        _connection->write( &ack, sizeof( ack ) );
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
        int indexRead = -2;
        ClientRSP* clientRSP = 0;
        // find connection destination
        for ( uint32_t i = 0 ; i < clientsRSP.size(); i++ )
        {
            if ( clientsRSP[i]->writerId == datagram->idwriter )
            {
                indexRead = i;
                clientRSP = clientsRSP[i];
                break;
            }
        }
        const uint64_t sizeToRead = readSize - sizeof(DatagramData);
        
        if ( !clientRSP )
            return;

        if ( !clientRSP->allRead )
            return;

        if ( indexRead == _indexRead)
            return;
        
        // if it's the first datagram 
        if ( clientRSP->ackSend )
        {
            if ( datagram->idSequence == clientRSP->idSequence )
                return;

            clientRSP->idSequence    = datagram->idSequence;
            clientRSP->posRead   = 0;
            clientRSP->totalSize = 0;
            clientRSP->ackSend   = false;
        }

        uint64_t index = datagram->idData;

        // if it's a repetition and we have the data then we ignore it
        if ( !clientRSP->boolBuffer.getData()[ index ] )
        {
            datagram++;
            const uint8_t* data = reinterpret_cast< const uint8_t* >
                                                         ( datagram );
            clientRSP->boolBuffer.getData()[ index ] = true;
            clientRSP->totalSize += sizeToRead;
            const uint64_t pos = ( index ) * ( _connection->getMTU() - 
                                         sizeof( DatagramData ) );
            memcpy( clientRSP->dataBuffer.getData() + pos,
                    data, sizeToRead );
        }
        
        break;
    }
    case ACK: 
    {
        const DatagramAck* ack = 
                  reinterpret_cast< const DatagramAck* >( type );
        
        // if the ack is not for me : ignore it
        if ( ack->writerID != _myID )
            return;
        if ( ack->idSequence != _idSequenceWrite )
            return;
        ClientRSP* clientRSP = 0;
        // find connection destination and if we have not receive an ack from,
        // we update the ack data.
        for ( std::vector< ClientRSP* >::iterator i = clientsRSP.begin() ;
              i != clientsRSP.end(); ++i )
        {
            clientRSP = *i;
            // si pas trouver faire assert
            if ( clientRSP->writerId == ack->readerID )
            {
                if ( clientRSP->ackReceive )
                    return;
                clientRSP->ackReceive = true;
                _countNbAckInWrite++;
                if ( _countNbAckInWrite == clientsRSP.size() )
                {
#ifdef WIN32
                    SetEvent( _writeEndEvent );
#else
 
#endif
                    _countNbAckInWrite = 0;
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
        if ( nack->idSequence != _idSequenceWrite )
            return;

        // find connection destination reset timeout
        for ( uint32_t i = 0 ; i < clientsRSP.size(); i++ )
        {
            if ( clientsRSP[i]->writerId == nack->readerId )
            {
                clientsRSP[i]->countTimeOut = 0;
                break;
            }
        }

        for (uint32_t i = nack->idDataStart; i <= nack->idDataEnd; i++ )
            _sendDatagram( i );
        
        const DatagramAckRequest ackRequest = { ACKREQ, _myID, 
                                      _lengthsData.getSize() - 1, 
                                      _idSequenceWrite };

        _connection->write( &ackRequest, sizeof( ackRequest ) );

        break;
    }
    case ACKREQ: // The writer ask for a ack data
    {
        const DatagramAckRequest* ackRequest = 
                  reinterpret_cast< const DatagramAckRequest* >
                                                         ( _bufRead.getData() );
        
        if ( ackRequest->writerID == _getID() )
            return;

        ClientRSP* clientRSP = 0;
        for ( std::vector< ClientRSP* >::iterator k = clientsRSP.begin() ;
              k != clientsRSP.end(); ++k )
        {
            clientRSP = *k;

            if ( clientRSP->ackSend && !clientRSP->allRead )
                continue;

            if( clientRSP->writerId != ackRequest->writerID )
                continue;
            
            // !!!!!!!!!!!!!!!!!
            if ( clientRSP->idSequence != ackRequest->idSequence )
                continue;

            // Equalizer has ever read data and we are in the case that
            // the writer has not receive my ack datagram. We have to repeat it
            if ( clientRSP->lastidSequenceAck == ackRequest->idSequence )
            {
                const DatagramAck ack = { ACK, _myID, ackRequest->writerID, 
                                    ackRequest->idSequence };
                _connection->write( &ack, sizeof( ack ) );
            }
            else
            {
                clientRSP->boolBuffer.resize( ackRequest->lastDatagramid + 1);
                for ( uint32_t i = 0; i < clientRSP->boolBuffer.getSize(); i++)
                {
                    if ( clientRSP->boolBuffer.getData()[i] )
                        continue;

                    // OPT: Send all NACK packets at once
                    for( uint32_t j = i; j < clientRSP->boolBuffer.getSize();
                         ++j )
                    {
                        if( !clientRSP->boolBuffer.getData()[j] )
                            continue;

                        DatagramNack datagramNack = { NACK, _myID, 
                                              clientRSP->writerId, 
                                              i, j-1,
                                              clientRSP->idSequence};
                        _connection->write( &datagramNack, 
                                    sizeof( DatagramNack ) );
                        return;
                    }
                    const DatagramNack datagramNack = { NACK, _myID, 
                                                  clientRSP->writerId, 
                                                  i, clientRSP->boolBuffer.getSize()-1,
                                                  clientRSP->idSequence };
                    _connection->write( &datagramNack, 
                                        sizeof( DatagramNack ) );
                    return;
                }
                
                // OPT: send ack, use second buffer to recv data while this
                // buffer is read

                clientRSP->ackSend = true;
                clientRSP->allRead = false;
#ifdef WIN32
                SetEvent( _hEvent );
#else

#endif
                break;
            }
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
            for ( std::vector< ClientRSP* >::iterator i = clientsRSP.begin() ;
              i != clientsRSP.end(); ++i )
            {
                if ( (*i)->writerId == node->idConnection )
                    return;
            }
            _addNewConnection( node->idConnection );
        }
        const DatagramCountConnection countNode = 
                             { COUNTNODE, _getID(), clientsRSP.size() };
        _connection->write( &countNode, sizeof( DatagramCountConnection ) );
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
                             { COUNTNODE, _getID(), clientsRSP.size() };
            _connection->write( &countNode, sizeof( DatagramCountConnection ) );
            return;
        }
        for ( std::vector< ClientRSP* >::iterator i = clientsRSP.begin() ;
              i != clientsRSP.end(); ++i )
        {
            if ( (*i)->writerId == node->idConnection )
            {
                clientsRSP.erase( i );
                break;
            }
        }

        const DatagramCountConnection countNode = 
                             { COUNTNODE, _getID(), clientsRSP.size() };
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
        if ( clientsRSP.size() == countConn->nbClient )
            return;
        
        for ( std::vector< ClientRSP* >::iterator i = clientsRSP.begin() ;
              i != clientsRSP.end(); ++i )
        {
            if ( (*i)->writerId == countConn->idClient )
                return;
        }
        _addNewConnection( countConn->idClient );    
        const DatagramCountConnection countNode = 
                           { COUNTNODE, _getID(), clientsRSP.size() };
        _connection->write( &countNode,  sizeof( DatagramCountConnection ) );
        break;
    }
    }//END switch
}


void RSPConnection::_addNewConnection( uint64_t id )
{
    ClientRSP* clientrsp = new ClientRSP();
    clientrsp->writerId  = id; 
    clientrsp->posRead   = 0;
    clientrsp->allRead   = true;
    clientrsp->boolBuffer.resize( _maxBuffer /  ( _connection->getMTU() - 
                                                  sizeof( DatagramData ))+1 );
    
    memset( clientrsp->boolBuffer.getData() , false, 
            clientrsp->boolBuffer.getSize() );

    clientrsp->dataBuffer.resize( _maxBuffer );

    clientsRSP.push_back( clientrsp );
    EQWARN << "New connection " << id << std::endl;
}

int64_t RSPConnection::write( const void* buffer, const uint64_t bytes )
{
    base::ScopedMutex mutex( _mutexConnection );
    if ( _getCountConnection() == 0 )
        return bytes;
    uint32_t size =   EQ_MIN( bytes, _maxBuffer );

    // init data structure
    _datagramsData.resize( 0 );
    _lengthsData.resize( 0 );
    uint64_t posInBuffer = 0;

#ifdef WIN32
    ResetEvent( _writeEndEvent );
#else
    // interrupt
#endif
    _idSequenceWrite ++;
    const char* bufferChar = reinterpret_cast< const char* >(buffer);
    const uint32_t maxLength = _connection->getMTU() - 
                         sizeof( DatagramData ); 
    uint32_t lastLength = 0;
    uint32_t compt = 0;
    
    // share buffer in max _mtu partition
    while ( posInBuffer < size )
    {
        WriteDatagramData ddata = { { DATA, _myID, compt, _idSequenceWrite },
                                    bufferChar + posInBuffer };
        // OPT: Recalculate data on the fly (nack)
        _datagramsData.append( ddata );
        posInBuffer += maxLength;
        
        if ( posInBuffer > size )
        {
            lastLength = _connection->getMTU() - ( posInBuffer - size );
            _lengthsData.append( lastLength );
        }
        else
        {
            _lengthsData.append( _connection->getMTU() );
        }
        _sendDatagram( compt );
        compt++;
    }

    // send a datagram Ack Request
    const DatagramAckRequest ackRequest = { ACKREQ, _myID, 
                                      _lengthsData.getSize() -1, 
                                      _idSequenceWrite };
    _connection->write( &ackRequest, sizeof( DatagramAckRequest ) );

    if ( clientsRSP.size() == 0 )
        return size;

    for ( std::vector< ClientRSP* >::iterator i = clientsRSP.begin() ;
              i != clientsRSP.end(); ++i )
            (*i)->ackReceive = false;
    
    while ( true )
    {
#ifdef WIN32
        const DWORD ret = WaitForSingleObject( _writeEndEvent, 100 );
#else
        const int ret = poll( &_writeEndEvent, 1, 100 );
#endif
        switch ( ret )
        {
#ifdef WIN32
        case WAIT_TIMEOUT:
#else
		case 0:			
#endif
        {
            for ( std::vector< ClientRSP* >::iterator i = clientsRSP.begin() ;
                  i != clientsRSP.end(); ++i )
            {
                ClientRSP* clientRSP = *i;
                if ( !clientRSP->ackReceive )
                {
                    clientRSP->countTimeOut++;
                    // send a ack request
                    if ( clientRSP->countTimeOut % 5 == 0)
                    {                    
                        // re-send a datagram Ack Request
                        _connection->write( &ackRequest, 
                                            sizeof( DatagramAckRequest ) );
                        
                        // may the connection is dead
                        if ( clientRSP->countTimeOut >= 100 )
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
           return size;
        }
    }
    return size;
}

void RSPConnection::_sendDatagram( const uint64_t idDatagram )
{
    eq::base::Bufferb sendBuffer;
    sendBuffer.resize( _connection->getMTU() );
    
    // set the header
    DatagramData* header = reinterpret_cast< DatagramData* >
                                                (sendBuffer.getData());
    *header = _datagramsData.getData()[idDatagram].header;
    header++;

    // prepare data
    const uint32_t sizeHeader =  sizeof( DatagramData ); 
    const char* datasrc = reinterpret_cast< const char* >
                              ( _datagramsData.getData()[idDatagram].data );

    // copy Data
    char* datadest = reinterpret_cast< char* >(header);
    memcpy( datadest, datasrc, _lengthsData.getData()[idDatagram] - sizeHeader);

    // send Data
    _connection->write ( sendBuffer.getData(), 
                         _lengthsData.getData()[idDatagram] );
}
}
}
