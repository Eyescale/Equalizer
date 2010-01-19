
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com>
 *               2009-2010, Stefan Eilemann <eile@equalizergraphics.com>
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
#include "global.h"
#include "log.h"

#include <eq/base/rng.h>
#include <eq/base/sleep.h>

//#define EQ_INSTRUMENT_RSP
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

using std::distance;

namespace eq
{
namespace net
{

static int32_t _mtu = -1;
static int32_t _ackFreq = -1;
uint32_t RSPConnection::_payloadSize = 0;
size_t   RSPConnection::_bufferSize = 0;
int32_t RSPConnection::_maxNAck = 0;

namespace
{
#ifdef EQ_INSTRUMENT_RSP
base::a_int32_t nReadDataAccepted;
base::a_int32_t nReadData;
base::a_int32_t nBytesRead;
base::a_int32_t nBytesWritten;
base::a_int32_t nDataPackets;
base::a_int32_t nTotalDatagrams;
base::a_int32_t nAckRequests;
base::a_int32_t nTotalAckRequests;
base::a_int32_t nAcksSend;
base::a_int32_t nAcksSendTotal;
base::a_int32_t nAcksRead;
base::a_int32_t nAcksAccepted;
base::a_int32_t nNAcksSend;
base::a_int32_t nNAcksRead;
base::a_int32_t nNAcksResend;
base::a_int32_t nTimeOuts;
base::a_int32_t nTimeInWrite;
base::a_int32_t nTimeInWriteWaitAck;
base::a_int32_t nTimeInReadSync;
base::a_int32_t nTimeInReadData;
base::a_int32_t nTimeInHandleData;
#endif
}



RSPConnection::RSPConnection()
        : _sendBuffer( Global::getIAttribute( Global::IATTR_UDP_MTU ))
        , _countAcceptChildren( 0 )
        , _id( 0 )
        , _timeouts( 0 )
        , _ackReceived( std::numeric_limits< uint16_t >::max( ))
        , _event( new EventConnection )
        , _writing( false )
        , _numWriteAcks( 0 )
        , _thread ( 0 )
        , _connection( 0 )
        , _parent( 0 )
        , _lastAck( -1 )
        , _freeBuffers( Global::getIAttribute( Global::IATTR_RSP_NUM_BUFFERS ))
        , _recvBuffer( Global::getIAttribute( Global::IATTR_UDP_MTU ))
        , _readBuffer( 0 )
        , _readBufferPos( 0 )
        , _sequenceID( 0 )
{
    _buildNewID();
    _description->type = CONNECTIONTYPE_RSP;
    _description->bandwidth = 102400;

    EQCHECK( _event->connect( ));

    if( _mtu == -1 )
    {
        _mtu = Global::getIAttribute( Global::IATTR_UDP_MTU );
        _ackFreq = Global::getIAttribute( Global::IATTR_UDP_PACKET_RATE );
        _payloadSize = _mtu - sizeof( DatagramData );
        _bufferSize = _payloadSize * _ackFreq;
        _maxNAck = (_mtu - sizeof( DatagramNack )) / sizeof( uint32_t );
        _maxNAck = EQ_MIN( _maxNAck, _ackFreq );
        EQASSERT( _maxNAck < std::numeric_limits< uint16_t >::max( ));
    }

    _inBuffers.reserve( Global::getIAttribute( Global::IATTR_RSP_NUM_BUFFERS ));
    while( static_cast< int32_t >( _inBuffers.size( )) <
           Global::getIAttribute( Global::IATTR_RSP_NUM_BUFFERS ))
    {
        _inBuffers.push_back( new Buffer( _mtu ));
        EQCHECK( _freeBuffers.push( _inBuffers.back( )));
    }

    _nackBuffer.reserve( _mtu );
    EQLOG( LOG_RSP ) << "New RSP Connection, buffer size " << _bufferSize
                     << ", packet size " << _mtu << std::endl;
}
RSPConnection::~RSPConnection()
{
    close();

    while( !_inBuffers.empty( ))
    {
        delete _inBuffers.back();
        _inBuffers.pop_back();
    }
}

void RSPConnection::close()
{
    if( _state == STATE_CLOSED )
        return;

    _state = STATE_CLOSED;

    if( _thread )
    {
        const DatagramNode exitNode = { ID_EXIT, _id };
        _connection->write( &exitNode, sizeof( DatagramNode ) );
        _connectionSet.interrupt();
        _thread->join();
    }

    _event->set();
    _children.clear();
    _parent = 0;

    if( _connection.isValid( ))
        _connection->close();

    _connection = 0;

    Buffer* dummy;
    while( _freeBuffers.pop( dummy ))
        /* nop */;

    for( BufferVector::iterator i = _inBuffers.begin();
         i != _inBuffers.end(); ++i )
    {
        Buffer* buffer = *i;
        EQCHECK( _freeBuffers.push( buffer ));
    }

    _readBuffer = 0;
    _readBufferPos = 0;
    _fireStateChanged();
}

//----------------------------------------------------------------------
// Async IO handles
//----------------------------------------------------------------------
void RSPConnection::_initAIORead(){ /* NOP */ }
void RSPConnection::_exitAIORead(){ /* NOP */ }

uint16_t RSPConnection::_buildNewID()
{
    eq::base::RNG rng;
    _id = rng.get< uint16_t >();
    return _id;
}

bool RSPConnection::listen()
{
    EQASSERT( _description->type == CONNECTIONTYPE_RSP );

    if( _state != STATE_CLOSED )
        return false;
    
    _state = STATE_CONNECTING;
    _fireStateChanged();

    // init udp connection
    _connection = new UDPConnection();
    ConnectionDescriptionPtr description = 
        new ConnectionDescription( *_description.get( ));
    description->type = CONNECTIONTYPE_UDP;
    _connection->setDescription( description );

    // connect UDP multicast connection
    if( !_connection->connect() )
    {
        EQWARN << "can't setup underlying UDP connection " << std::endl;
        return false;
    }

    EQASSERT( _mtu == _connection->getMTU( ));
    EQASSERT( _ackFreq =  _connection->getPacketRate( ));

    _description = new ConnectionDescription( *description.get( ));
    _description->type = CONNECTIONTYPE_RSP;
    
    _connectionSet.addConnection( _connection.get( ));

    // init a thread to manage the communication protocol 
    _thread = new Thread( this );

    _numWriteAcks =  0;

    // waits until RSP protocol establishes connection to the multicast network
    if( !_thread->start( ))
    {
        close();
        return false;
    }

    _state = STATE_LISTENING;
    _fireStateChanged();

    EQINFO << "Listening on " << _description->getHostname() << ":"
           << _description->port << " (" << _description->toString() << " @"
           << (void*)this << ")" << std::endl;
    return true;
}

ConnectionPtr RSPConnection::acceptSync()
{
    CHECK_THREAD( _recvThread );
    if( _state != STATE_LISTENING )
        return 0;

    EQASSERT ( _countAcceptChildren < static_cast< int >( _children.size( )));

    RSPConnectionPtr newConnection = _children[ _countAcceptChildren ];

    newConnection->_initAIORead();
    newConnection->_parent      = this;
    newConnection->_connection  = 0;
    newConnection->_state       = STATE_CONNECTED;
    newConnection->_description = _description;

    ++_countAcceptChildren;
    _sendDatagramCountNode();
    
    EQINFO << "accepted RSP connection " << newConnection->_id << std::endl;
    base::ScopedMutex mutexConn( _mutexConnection );

    if ( static_cast< int >( _children.size() ) > _countAcceptChildren )
        _event->set();
    else 
        _event->reset();

    ConnectionPtr connection = newConnection.get();
    return connection;
}

int64_t RSPConnection::readSync( void* buffer, const uint64_t bytes )
{
#ifdef EQ_INSTRUMENT_RSP
    base::Clock clock;
#endif

    uint64_t bytesLeft = bytes;
    uint8_t* ptr = reinterpret_cast< uint8_t* >( buffer );

    while( bytesLeft ) // this is somewhat redundant (done by higher-level
                       // function already), but saves quiet a few lock ops
    {
        if( !_readBuffer )
        {
            _readBuffer = _readBuffers.pop();
            EQASSERT( _readBufferPos == 0 );
        }
        EQASSERT( _readBuffer );

        const DatagramData* header = reinterpret_cast< const DatagramData* >(
            _readBuffer->getData( ));
        const uint8_t* payload = reinterpret_cast< const uint8_t* >( header+1 );

        const size_t dataLeft = header->size - _readBufferPos;
        const size_t size = EQ_MIN( static_cast< size_t >( bytesLeft ),
                                    dataLeft );

        memcpy( ptr, payload + _readBufferPos, size );
        _readBufferPos += size;
        ptr += size;
        bytesLeft -= size;

        // if all data in the buffer has been taken
        if( _readBufferPos == header->size )
        {
            EQLOG( LOG_RSP ) << "reset read buffer of sequence "
                             << header->sequenceID << std::endl;

            EQCHECK( _freeBuffers.push( _readBuffer ));
            _readBuffer = 0;
            _readBufferPos = 0;
        }
    }

    if( !_readBuffers.isEmpty( ))
        _event->set();
    else
    {
        base::ScopedMutex mutex( _mutexEvent );
        if( _readBuffers.isEmpty( ))
            _event->reset();
    }

#ifdef EQ_INSTRUMENT_RSP
    nTimeInReadSync += clock.getTime64();
    nBytesRead += size;
#endif

    return bytes;
}

bool RSPConnection::_acceptID()
{
    _connection->readNB( _recvBuffer.getData(), _mtu );

    // send a first datagram for announce me and discover other connection 
    EQLOG( LOG_RSP ) << "Announce " << _id << std::endl;
    const DatagramNode newnode = { ID_HELLO, _id };
    _connection->write( &newnode, sizeof( DatagramNode ) );

    _timeouts = 0;
    while ( true )
    {
        switch( _connectionSet.select( 10 ))
        {
            case ConnectionSet::EVENT_TIMEOUT:
                ++_timeouts;
                if ( _timeouts < 20 )
                {
                    EQLOG( LOG_RSP ) << "Announce " << _id << std::endl;
                    const DatagramNode ackNode ={ ID_HELLO, _id };
                    _connection->write( &ackNode, sizeof( DatagramNode ) );
                }
                else 
                {
                    EQLOG( LOG_RSP ) << "Confirm " << _id << std::endl;
                    EQINFO << "opened RSP connection " << _id << std::endl;
                    const DatagramNode confirmNode ={ ID_CONFIRM, _id };
                    _connection->write( &confirmNode, sizeof( DatagramNode ) );
                    _addNewConnection( _id );
                    return true;
                }
                break;

            case ConnectionSet::EVENT_DATA:
                if( !_handleAcceptID() )
                {
                    EQERROR << " Error during Read UDP Connection" 
                            << std::endl;
                    return false;
                }

                _connection->readNB( _recvBuffer.getData(), _mtu );
                break;

            case ConnectionSet::EVENT_INTERRUPT:
            default:
                break;
        }
    }
}

bool RSPConnection::_handleAcceptID()
{
    // read datagram 
    void* data = _recvBuffer.getData();
    if( _connection->readSync( data, _mtu ) == -1 )
    {
        EQERROR << "Error read on Connection UDP" << std::endl;
        return false;
    }

    // read datagram type
    const uint16_t type = *reinterpret_cast< uint16_t* >( data );
    const DatagramNode* node = reinterpret_cast< const DatagramNode* >( data );
    switch( type )
    {
        case ID_HELLO:
            _checkNewID( node->connectionID );
            return true;

        case ID_DENY:
            // a connection refused my ID, try another ID
            if( node->connectionID == _id ) 
            {
                _timeouts = 0;
                const DatagramNode newnode = { ID_HELLO, _buildNewID() };
                EQLOG( LOG_RSP ) << "Announce " << _id << std::endl;
                _connection->write( &newnode, sizeof( DatagramNode ));
            }
            return true;

        case ID_EXIT:
            return _removeConnection( node->connectionID );

        default: break;    
    }

    return true;
}

bool RSPConnection::_initReadThread()
{
    // send a first datagram to announce me and discover all other connections 
    _sendDatagramCountNode();
    _timeouts = 0;
    while ( true )
    {
        switch( _connectionSet.select( 10 ) )
        {
            case ConnectionSet::EVENT_TIMEOUT:
                ++_timeouts;
                if( _timeouts >= 20 )
                    return true;

                _sendDatagramCountNode();
                break;

            case ConnectionSet::EVENT_DATA:
                if ( !_handleInitData() )
                {
                    EQERROR << " Error during Read UDP Connection" 
                            << std::endl;
                    return false;
                }

                _connection->readNB( _recvBuffer.getData(), _mtu );
                break;

            case ConnectionSet::EVENT_INTERRUPT:
            default:
                break;
        }
    }
}

bool RSPConnection::_handleInitData()
{
    // read datagram 
    void* data = _recvBuffer.getData();
    if( _connection->readSync( data, _mtu ) == -1 )
    {
        EQERROR << "Read error" << std::endl;
        return false;
    }

    // read datagram type
    const uint16_t type = *reinterpret_cast< uint16_t* >( data );
    const DatagramNode* node = reinterpret_cast< const DatagramNode* >( data );
    switch( type )
    {
        case ID_HELLO:
            _timeouts = 0;
            _checkNewID( node->connectionID ) ;
            return true;

        case ID_CONFIRM:
            _timeouts = 0;
            return _addNewConnection( node->connectionID );

        case COUNTNODE:
            if( _handleCountNode( ))
                _timeouts = 20;
            else
                _timeouts = 0;
            break;
    
        case ID_EXIT:
            return _removeConnection( node->connectionID );

        default:
            EQUNIMPLEMENTED;
            break;
    }
    
    return true;
}

void* RSPConnection::Thread::run()
{
    _connection->_runReadThread();
    _connection = 0;
    return 0;
}

void RSPConnection::_runReadThread()
{
    EQINFO << "Started RSP read thread" << std::endl;
 
    while ( _state != STATE_CLOSED && !_children.empty( ))
    {
        const int32_t timeOut = ( _writing && _repeatQueue.isEmpty( )) ? 
            Global::getIAttribute( Global::IATTR_RSP_ACK_TIMEOUT ) : -1;

        const ConnectionSet::Event event = _connectionSet.select( timeOut );
        switch( event )
        {
            case ConnectionSet::EVENT_TIMEOUT:
            {
#ifdef EQ_INSTRUMENT_RSP
                ++nTimeOuts;
#endif
                ++_timeouts;
                if( _timeouts >= 
                    Global::getIAttribute( Global::IATTR_RSP_MAX_TIMEOUTS ))
                {
                    EQERROR << "Too many timeouts during send " << _timeouts
                            << std::endl;

                    // unblock and terminate write function
                    _repeatQueue.push( RepeatRequest( RepeatRequest::DONE ));
                    while( _writing )
                        base::sleep( 1 );

                    _connection = 0;
                    return;
                }
            
                // repeat ack request
                _repeatQueue.push( RepeatRequest( RepeatRequest::ACKREQ ));
                break;
            }

            case ConnectionSet::EVENT_DATA:
            {
#ifdef EQ_INSTRUMENT_RSP
                base::Clock clock;
#endif            
                if ( !_handleData() )
                    return;

                _connection->readNB( _recvBuffer.getData(), _mtu );
#ifdef EQ_INSTRUMENT_RSP
                nTimeInHandleData += clock.getTime64();
#endif
                break;
            }

            case ConnectionSet::EVENT_INTERRUPT:
                break;
            default:
                EQUNIMPLEMENTED;
                break;
        }
    }
}

bool RSPConnection::_handleData( )
{
    // read datagram 
    void* data = _recvBuffer.getData();
    if( _connection->readSync( data, _mtu ) == -1 )
    {
        EQERROR << "Error reading from on UDP connection" << std::endl;
        return false;
    }

    // read datagram type
    const uint16_t type = *reinterpret_cast< uint16_t* >( data );
    switch( type )
    {
        case DATA:
        {
#ifdef EQ_INSTRUMENT_RSP
            base::Clock clock;
#endif
            const bool result = _handleDataDatagram( _recvBuffer );

#ifdef EQ_INSTRUMENT_RSP
            nTimeInReadData += clock.getTime64();
#endif
            return result;
        }

        case ACK:
            return _handleAck( reinterpret_cast< const DatagramAck* >( data )) ;
    
        case NACK:
            return _handleNack( reinterpret_cast< const DatagramNack* >( data));
    
        case ACKREQ: // The writer asks for an ack/nack
            return _handleAckRequest(
                reinterpret_cast< const DatagramAckRequest* >( data ));
    
        case ID_HELLO:
        {
            const DatagramNode* node =
                reinterpret_cast< const DatagramNode* >( data );
            _checkNewID( node->connectionID );
            return true;
        }

        case ID_CONFIRM:
        {
            const DatagramNode* node =
                reinterpret_cast< const DatagramNode* >( data );
            return _addNewConnection( node->connectionID );
        }

        case ID_EXIT:
        {
            const DatagramNode* node = 
                reinterpret_cast< const DatagramNode* >( data );
            return _removeConnection( node->connectionID );
        }

        case COUNTNODE:
            _handleCountNode();
            return true;

        default: 
            EQUNIMPLEMENTED;
    }

    return true;
}

bool RSPConnection::_handleDataDatagram( Buffer& buffer )
{
#ifdef EQ_INSTRUMENT_RSP
    ++nReadData;
#endif
    const DatagramData* datagram = 
        reinterpret_cast< const DatagramData* >( buffer.getData( ));
    const uint16_t writerID = datagram->writerID;
    RSPConnectionPtr connection = _findConnection( writerID );
    EQASSERT( connection->_id == writerID );

    // it's an unknown connection or when we run netperf client before
    // server netperf
    // TODO find a solution in this situation - _addNewConnection()?
    if( !connection )
    {
        EQASSERTINFO( false, "Can't find connection with id " << writerID );
        return false;
    }

    // ignore it if it's a datagram repetition for another receiver
    const int32_t sequenceID = datagram->sequenceID;
    if( connection->_sequenceID > sequenceID )
    {
        if( connection->_sequenceID - sequenceID <= 16384 )
            return true;

        // else the sequence ID was reset by sender, reset as well
        EQASSERT( connection->_lastAck > 16384 );
        EQASSERTINFO( connection->_lastAck < 49152, connection->_lastAck );
        EQASSERT( connection->_lastAck + 1 == connection->_sequenceID );
        EQASSERT( sequenceID < 16384 );
        connection->_sequenceID = 0;
    }

    EQLOG( LOG_RSP ) << "receive data from " << writerID << " sequence "
                     << sequenceID << std::endl;

    if( connection->_sequenceID == sequenceID ) // standard case
    {
        Buffer* newBuffer = connection->_newDataBuffer( buffer );
        if( !newBuffer ) // no more data buffers, drop packet
            return true;

#ifdef EQ_INSTRUMENT_RSP
        ++nReadDataAccepted;
#endif

        base::ScopedMutex mutex( connection->_mutexEvent );
        EQLOG( LOG_RSP ) << "post buffer with sequence " << sequenceID
                         << std::endl;

        ++connection->_sequenceID;
        connection->_readBuffers.push( newBuffer );

        while( !connection->_recvBuffers.empty( )) // queue ready pending data
        {
            newBuffer = connection->_recvBuffers.front();
            if( !newBuffer )
                break;
            
            connection->_recvBuffers.pop_front();
            EQASSERT( reinterpret_cast<DatagramData*>( 
                          newBuffer->getData( ))->sequenceID ==
                      connection->_sequenceID );
            ++connection->_sequenceID;
            connection->_readBuffers.push( newBuffer );
        }

        if( !connection->_recvBuffers.empty() &&
            !connection->_recvBuffers.front( )) // update for new _sequenceID
        {
            connection->_recvBuffers.pop_front();
        }

        connection->_event->set();
        return true;
    }
    // else out of order

    EQASSERT( sequenceID > connection->_sequenceID );
    const size_t size = sequenceID - connection->_sequenceID;

    if( connection->_recvBuffers.size() >= size && 
        connection->_recvBuffers[ size - 1 ] ) // repetition
    {
        return true;
    }

    Buffer* newBuffer = connection->_newDataBuffer( buffer );
    if( !newBuffer ) // no more data buffers, drop packet
        return true;

#ifdef EQ_INSTRUMENT_RSP
    ++nReadDataAccepted;
#endif
    if( connection->_recvBuffers.size() < size )
        connection->_recvBuffers.resize( size, 0 );

    EQASSERT( !connection->_recvBuffers[ size - 1 ] );
    connection->_recvBuffers[ size - 1 ] = newBuffer;

    // early nack: request missing packets before current
    uint16_t nacks[2] = { sequenceID - 1, sequenceID - 1 };
    size_t i = size - 1;
    if( i > 0 )
    {
        --i;
        if( connection->_recvBuffers[i] ) // got previous packet
            return true;

        while( i > 0 && !connection->_recvBuffers[i] )
        {
            --nacks[0];
            --i;
        }

        if( i==0 )
            --nacks[0];           
    }

    EQLOG( LOG_RSP ) << "send early nack " << nacks[0] << ".." << nacks[1]
                     << std::endl;
    _sendNack( writerID, sequenceID, 1, nacks );
    return true;
}

RSPConnection::Buffer* RSPConnection::_newDataBuffer( Buffer& inBuffer )
{
    EQASSERT( static_cast< int32_t >( inBuffer.getMaxSize( )) == _mtu );

    Buffer* buffer = 0;
    if( !_freeBuffers.pop( buffer ))
    {
        // we do not have a free buffer, which means that the receiver is slower
        // then our read thread. This is bad, because now we'll drop the data
        // and will send a NACK packet upon the ack request, causing
        // retransmission even though we'll probably drop it again
        EQLOG( LOG_RSP ) << "Reader too slow, dropping data" << std::endl;
        return 0;
    }
    // else

    buffer->swap( inBuffer );
    return buffer;
}

bool RSPConnection::_handleAck( const DatagramAck* ack )
{
#ifdef EQ_INSTRUMENT_RSP
    ++nAcksRead;
#endif
    EQLOG( LOG_RSP ) << "got ack from " << ack->readerID << " for "
                     << ack->writerID << " sequence " << ack->sequenceID
                     << " current " << _sequenceID << std::endl;

    // ignore sequenceID which is different from my write sequence id
    //  Reason : - repeated, late ack or an ack for another writer
    if( !_isCurrentSequence( ack->sequenceID, ack->writerID ) )
    {
        EQLOG( LOG_RSP ) << "ignore ack " << ack->sequenceID << " for "
                         << ack->writerID << ", it's not for me (" 
                         << _sequenceID << ", " << _id << ")" << std::endl;
        return true;
    }

    // find connection destination and if we have not received an ack from it,
    // we update the ack data.
    RSPConnectionPtr connection = _findConnection( ack->readerID );
    if ( !connection )
    {
        EQUNREACHABLE;
        return false;
    }

    // if I have received an ack previously from the reader
    if( connection->_ackReceived == ack->sequenceID )
        return true;

#ifdef EQ_INSTRUMENT_RSP
    ++nAcksAccepted;
#endif
    connection->_ackReceived = ack->sequenceID;
    ++_numWriteAcks;
    _timeouts = 0; // reset timeout counter

    if( _numWriteAcks != _children.size( ))
    {
        EQASSERT( _numWriteAcks < _children.size( ));
        return true;
    }

    EQLOG( LOG_RSP ) << "unlock write function " << _sequenceID << std::endl;

    // unblock write function if all acks have been received
    _repeatQueue.push( RepeatRequest( RepeatRequest::DONE ));
    return true;
}

bool RSPConnection::_handleNack( const DatagramNack* nack )
{
#ifdef EQ_INSTRUMENT_RSP
    nNAcksRead += nack->count;
#endif

    EQLOG( LOG_RSP ) << "handle nack from " << nack->readerID << " for "
                     << nack->writerID << " sequence " << nack->sequenceID
                     << std::endl;

    if( !_isCurrentSequence( nack->sequenceID, nack->writerID ) )
    {
        EQLOG( LOG_RSP ) << "ignore nack, it's not for me" << std::endl;
        return true;
    }
    
    RSPConnectionPtr connection = _findConnection( nack->readerID );
    if ( !connection )
    {
        EQUNREACHABLE;
        return false;
        // it's an unknown connection, TODO add this connection?
    }

    if( connection->_ackReceived == nack->sequenceID )
    {
        EQLOG( LOG_RSP ) << "ignore nack, already got an ack" << std::endl;
        return true;
    }

    _timeouts = 0;

    EQLOG( LOG_RSP ) << "Queue data repeat request" << std::endl;
    _addRepeat( reinterpret_cast< const uint16_t* >( nack + 1 ), nack->count );
    return true;
}

void RSPConnection::_addRepeat( const uint16_t* nacks, uint16_t num )
{
    for( size_t i = 0; i < num; ++i )
    {
        RepeatRequest repeat;
        repeat.start = nacks[ i * 2 ];
        repeat.end   = nacks[ i * 2 + 1 ];
        _repeatQueue.push( repeat );

        EQASSERT( repeat.start <= repeat.end);
    }
}

bool RSPConnection::_handleAckRequest( const DatagramAckRequest* ackRequest )
{
    const uint16_t writerID = ackRequest->writerID;
    const int32_t sequenceID = ackRequest->sequenceID;
    EQLOG( LOG_RSP ) << "got ack request "  << sequenceID << " from "
                     << writerID << std::endl;

    RSPConnectionPtr connection = _findConnection( writerID );
    if( !connection )
    {
        EQUNREACHABLE;
        return false;
    }

    // Repeat ack
    if( connection->_lastAck == sequenceID )
    {
        EQLOG( LOG_RSP ) << "repeat Ack for sequence " << sequenceID
                         << std::endl;
        _sendAck( writerID, sequenceID );
        return true;
    }
    EQASSERTINFO( sequenceID > connection->_lastAck || 
                  connection->_lastAck - sequenceID > 16384,
                  sequenceID << ", " << connection->_lastAck );

    if( connection->_sequenceID == sequenceID + 1 ) // got all data
    {
        EQLOG( LOG_RSP ) << "send ack to " << connection->_id << ", sequence "
                         << sequenceID << std::endl;
        _sendAck( connection->_id, sequenceID );
        connection->_lastAck = sequenceID;
#ifdef EQ_INSTRUMENT_RSP
        ++nAcksSend;
#endif
        return true;
    }
    // else 

    if( sequenceID < connection->_sequenceID )
    {   // sequence reset, no data received yet
        EQASSERTINFO( connection->_sequenceID - sequenceID > 16384,
                      connection->_sequenceID << ", " << sequenceID );

        EQLOG( LOG_RSP ) << "nack all datagrams (" << connection->_lastAck + 1
                         << ".." << sequenceID << ")" << std::endl;

        const uint16_t nacks[2] = { connection->_lastAck + 1, sequenceID };
        _sendNack( connection->_id, sequenceID, 1, nacks ); // full nack
        return true;
    }
    // else find all missing datagrams
    EQASSERT( sequenceID >= connection->_sequenceID );
    EQASSERT( sequenceID - connection->_sequenceID < 16384 );

    uint16_t* nacks = static_cast< uint16_t* >(
                                    alloca( _maxNAck * 2 * sizeof( uint16_t )));
    uint32_t num = 0;
    const uint32_t max = 2 * _maxNAck - 2;

    nacks[ num++ ] = connection->_sequenceID;
    EQLOG( LOG_RSP ) << base::disableFlush << "nacks: " << nacks[num-1] << "..";
    
    std::deque<Buffer*>::const_iterator first =connection->_recvBuffers.begin();
    for( std::deque<Buffer*>::const_iterator i = first;
         i != connection->_recvBuffers.end() && num < max; ++i )
    {
        if( *i ) // got buffer
        {
            nacks[ num++ ] = connection->_sequenceID + distance( first, i );
            EQLOG( LOG_RSP ) << nacks[num-1] << ", ";

            // find next hole
            for( ++i; i != connection->_recvBuffers.end() && (*i); ++i )
                /* nop */;
      
            if( i != connection->_recvBuffers.end( ))
            {
                nacks[ num++ ] = connection->_sequenceID + distance( first, i );
                EQLOG( LOG_RSP ) << nacks[num-1] << "..";
            }
            else
            {
                nacks[ num++ ] = std::numeric_limits< uint16_t >::max();
                break; // else end iter gets incremented again...
            }
        }
    }

    if( nacks[ num - 1 ] == std::numeric_limits< uint16_t >::max( )) //overshoot
        --num;
    else
    {
        EQASSERT( nacks[ num - 1 ] <= sequenceID );
        nacks[ num++ ] = sequenceID;
        EQLOG( LOG_RSP ) << nacks[num-1];
    }
    EQLOG( LOG_RSP ) << std::endl << base::enableFlush;

    EQASSERT( num > 0 );
    EQASSERT( (num % 2) == 0 );

    EQLOG( LOG_RSP ) << "receiver send nack to " << connection->_id 
                     << ", sequence " << sequenceID << std::endl;
    _sendNack( connection->_id, sequenceID, num/2, nacks );
    return true;
}

bool RSPConnection::_handleCountNode()
{
    const DatagramCountConnection* countConn = 
    reinterpret_cast< const DatagramCountConnection* >( _recvBuffer.getData( ));

    EQLOG( LOG_RSP ) << "Got " << countConn->nbClient << " from " 
                     << countConn->clientID << std::endl;
    // we know all connections
    if( _children.size() == countConn->nbClient ) 
        return true;

    RSPConnectionPtr connection = _findConnection( countConn->clientID );
    if( !connection )
        _addNewConnection( countConn->clientID );

    return false;
}

void RSPConnection::_checkNewID( uint16_t id )
{
    if( id == _id )
    {
        EQLOG( LOG_RSP ) << "Deny " << id << std::endl;
        DatagramNode nodeSend = { ID_DENY, _id };
        _connection->write( &nodeSend, sizeof( DatagramNode ) );
        return;
    }
    
    // look if the new ID exist in another connection
    RSPConnectionPtr child = _findConnection( id );
    if( child.isValid() )
    {
        EQLOG( LOG_RSP ) << "Deny " << id << std::endl;
        DatagramNode nodeSend = { ID_DENY, id };
        _connection->write( &nodeSend, sizeof( DatagramNode ) );
    }
    
    return;
}

RSPConnection::RSPConnectionPtr RSPConnection::_findConnection( 
    const uint16_t id )
{
    for( std::vector< RSPConnectionPtr >::const_iterator i = _children.begin();
         i != _children.end(); ++i )
    {
        if( (*i)->_id == id )
            return *i;
    }
    return 0;
}


bool RSPConnection::_addNewConnection( const uint16_t id )
{
    if( _findConnection( id ).isValid() )
        return true;

    RSPConnection* connection = new RSPConnection();
    connection->_connection   = 0;
    connection->_id           = id;

    // protect the event and child size which can be used at the same time 
    // in acceptSync
    {
        base::ScopedMutex mutexConn( _mutexConnection );
        _children.push_back( connection );
        EQWARN << "new rsp connection " << id << std::endl;
        _event->set();
    }

    _sendDatagramCountNode();
    return true;
}

bool RSPConnection::_removeConnection( const uint16_t id )
{
    EQWARN << "remove connection " << id << std::endl;

    for( std::vector< RSPConnectionPtr >::iterator i = _children.begin(); 
          i != _children.end(); ++i )
    {
        RSPConnectionPtr child = *i;
        if( child->_id == id )
        {
            --_countAcceptChildren;
            child->close();
            _children.erase( i );
            break;
        }
    }
    
    _sendDatagramCountNode();

    if( _children.size() == 1 )
    {
        --_countAcceptChildren;
        _children[0]->close();
        _children.clear();
    }

    return true;
}

bool RSPConnection::_isCurrentSequence( uint16_t sequenceID, uint16_t writer )
{
    if( writer != _id )
        return false;

    if( _sequenceID <= sequenceID )
        return ( (sequenceID - _sequenceID) < 16384 ); // no reset?

    if( sequenceID < 16384 ) // reset during current write
    {
        EQASSERTINFO( _sequenceID > 16384, _sequenceID );
        return true;
    }

    EQUNREACHABLE;
    return false;
}

int64_t RSPConnection::write( const void* buffer, const uint64_t bytes )
{
    if( _state != STATE_CONNECTED && _state != STATE_LISTENING )
        return -1;
    
    if ( _parent.isValid() )
        return _parent->write( buffer, bytes );

    const uint64_t size = EQ_MIN( bytes, _bufferSize );

    if ( !_connection.isValid() )
        return -1;

#ifdef EQ_INSTRUMENT_RSP
    base::Clock clock;
    nBytesWritten += size;
#endif
    _timeouts = 0;
    _numWriteAcks = 0;

    if( _sequenceID > 32768 ) // reset regularly
        _sequenceID = 0;
    // compute number of datagrams
    uint64_t nDatagrams = size  / _payloadSize;    
    if ( nDatagrams * _payloadSize != size )
        ++nDatagrams;
    EQASSERT( nDatagrams < 16384 );

    const uint16_t lastSequenceID = _sequenceID + nDatagrams - 1;
    EQLOG( LOG_RSP ) << "write sequence " << _sequenceID << ".."
                     << lastSequenceID << " (" << nDatagrams << ")"
                     << std::endl;

    // send each datagram
    const uint8_t* data = reinterpret_cast< const uint8_t* >( buffer );
    for( size_t i = 0; i < nDatagrams; ++i )
        _sendDatagram( data, size, i );

#ifdef EQ_INSTRUMENT_RSP
    nDataPackets += nDatagrams;
#endif

    EQLOG( LOG_RSP ) << "Initial write done, send ack request for "
                     << _sequenceID << ".." << lastSequenceID << std::endl;

    _writing = true;
    _connectionSet.interrupt();

#ifdef EQ_INSTRUMENT_RSP
    ++nAckRequests;
    base::Clock clockAck;
#endif

    _sendAckRequest( lastSequenceID );
    _handleRepeat( data, size );
    _adaptSendRate( nDatagrams, 0 );
    _sequenceID = lastSequenceID + 1;

#ifdef EQ_INSTRUMENT_RSP
    nTimeInWriteWaitAck  += clockAck.getTime64();
    nTimeInWrite += clock.getTime64();

    if( bytes <= _bufferSize )
        EQWARN << *this << std::endl;
#endif

    EQLOG( LOG_RSP ) << "wrote sequence " << _sequenceID - 1 << std::endl;
    return size;
}

void RSPConnection::_handleRepeat( const uint8_t* data, const uint64_t size )
{
    size_t nDatagrams = size  / _payloadSize;    
    if ( nDatagrams * _payloadSize != size )
        ++nDatagrams;

    size_t nPackets = nDatagrams;
    while( true )
    {
        std::vector< RepeatRequest > requests;

        while( requests.empty( ))
        {
            const RepeatRequest& request = _repeatQueue.pop();
            switch( request.type )
            {
                case RepeatRequest::DONE:
                    _writing = false;
                    return;

                case RepeatRequest::ACKREQ:
                    _sendAckRequest( _sequenceID + nDatagrams - 1 );
                    _connectionSet.interrupt();
                    break;

                case RepeatRequest::NACK:
                    requests.push_back( request );
                    break;

                default:
                    EQUNIMPLEMENTED;
            }
        }

        const int32_t time =Global::getIAttribute(Global::IATTR_RSP_NACK_DELAY);
        if( time > 0 )
            base::sleep( time );

        // merge nack requests
        while( !_repeatQueue.isEmpty( ))
        {
            const RepeatRequest& candidate = _repeatQueue.pop();
            EQASSERT( candidate.end - candidate.start + 1 <= nDatagrams );

            switch( candidate.type )
            {
                case RepeatRequest::DONE:
                    _writing = false;
                    return;

                case RepeatRequest::ACKREQ:
                    break; // ignore, will send one below anyway

                case RepeatRequest::NACK:
                {
                    bool merged = false;
                    for( std::vector< RepeatRequest >::iterator i = 
                         requests.begin(); i != requests.end() && !merged; ++i )
                    {
                        RepeatRequest& old = *i;
                        EQASSERT( old.end < _sequenceID + nDatagrams );
                        if( old.start <= candidate.end &&
                            old.end   >= candidate.start )
                        {
                            old.start = EQ_MIN( old.start, candidate.start);
                            old.end   = EQ_MAX( old.end, candidate.end );
                            merged    = true;
                        }
                    }

                    if( !merged )
                        requests.push_back( candidate );
                }
            }
        }

        // calculate errors and adapt send rate
        size_t nErrors = 0;
        for( std::vector< RepeatRequest >::iterator i = requests.begin();
             i != requests.end(); ++i )
        {
            RepeatRequest& repeat = *i; 
            nErrors += repeat.end - repeat.start + 1;
        }

        _adaptSendRate( nPackets, nErrors );
        nPackets = nErrors;

        // send merged requests
        for( std::vector< RepeatRequest >::iterator i = requests.begin();
             i != requests.end(); ++i )
        {
#ifdef EQ_INSTRUMENT_RSP
            ++nNAcksResend;
#endif
            RepeatRequest& repeat = *i;        
            EQASSERT( repeat.start <= repeat.end );
            EQASSERT( repeat.start >= _sequenceID );
            EQASSERTINFO( repeat.end < _sequenceID + nDatagrams,
                          repeat.end << ", " << _sequenceID << ", " << 
                          nDatagrams );
            EQLOG( LOG_RSP ) << "Repeat " << repeat.start << ".." << repeat.end
                             << std::endl;

            for( size_t j = repeat.start; j <= repeat.end; ++j )
                _sendDatagram( data, size, j - _sequenceID );
        }

        // re-request ack
        if ( _repeatQueue.isEmpty() )
        {
            _sendAckRequest( _sequenceID + nDatagrams - 1 );
            _connectionSet.interrupt();
        }
    }
}

void RSPConnection::_adaptSendRate( const size_t nPackets, const size_t nErrors)
{
    EQASSERT( nPackets > 0 );
    const float error = ( static_cast< float >( nErrors ) /
                          static_cast< float >( nPackets ) * 100.f ) - 
        Global::getIAttribute( Global::IATTR_RSP_ERROR_BASE_RATE );

    if ( error < 0.f )
    {
        int32_t delta = static_cast< int32_t >( error *
                      Global::getIAttribute( Global::IATTR_RSP_ERROR_UPSCALE ));
        delta = EQ_MIN( Global::getIAttribute( Global::IATTR_RSP_ERROR_MAX ),
                        delta );

        EQLOG( LOG_RSP ) << nErrors << "/" << nPackets
                         << " nErrors, change send rate by " << -delta << "%"
                         << std::endl;
        _connection->adaptSendRate( -delta );
    }
    else
    {
        int32_t delta = static_cast< int32_t >( error /
                    Global::getIAttribute( Global::IATTR_RSP_ERROR_DOWNSCALE ));
        delta = EQ_MIN( Global::getIAttribute( Global::IATTR_RSP_ERROR_MAX ),
                        delta );

        EQLOG( LOG_RSP ) << nErrors << "/" << nPackets
                         << " nErrors, change send rate by " << -delta << "%"
                         << std::endl;
        _connection->adaptSendRate( -delta );
    }
}

void RSPConnection::_sendDatagramCountNode()
{
    if ( !_findConnection( _id ) )
        return;

    EQLOG( LOG_RSP ) << _children.size() << " nodes" << std::endl;
    const DatagramCountConnection count = { COUNTNODE, _id, _children.size() };
    _connection->write( &count, sizeof( count ));
}

void RSPConnection::_sendAck( const uint16_t writerID, 
                              const uint16_t sequenceID )
{
#ifdef EQ_INSTRUMENT_RSP
    ++nAcksSendTotal;
#endif
    const DatagramAck ack = { ACK, _id, writerID, sequenceID };
    if ( _id == writerID )
        _handleAck( &ack );
    else
        _connection->write( &ack, sizeof( ack ) );
}

void RSPConnection::_sendNack( const uint16_t writerID, 
                               const uint16_t sequenceID,
                               const uint16_t count, const uint16_t* repeats )
{
#ifdef EQ_INSTRUMENT_RSP
    ++nNAcksSend;
#endif
    /* optimization: use the direct access to the reader. */
    if( writerID == _id )
    {
         _addRepeat( repeats, count );
         return;
    }

    const int32_t size = 2*count * sizeof( uint16_t ) + sizeof( DatagramNack );
    EQASSERT( size <= _mtu );

    // set the header
    DatagramNack* header =
        reinterpret_cast< DatagramNack* >( _nackBuffer.getData( ));

    header->type = NACK;
    header->readerID = _id;
    header->writerID = writerID;
    header->sequenceID = sequenceID;
    header->count = count;

    memcpy( header + 1, repeats, size - sizeof( DatagramNack ));
    _connection->write( header, size );
}

void RSPConnection::_sendDatagram( const uint8_t* data, const uint64_t size,
                                   const uint16_t which )
{
#ifdef EQ_INSTRUMENT_RSP
    ++nTotalDatagrams;
#endif
    const uint32_t posInData = _payloadSize * which;
    uint32_t packetSize = size - posInData;
    packetSize = EQ_MIN( packetSize, _payloadSize );
    
    const uint8_t* ptr = data + posInData;
    _sendBuffer.resize( packetSize + sizeof( DatagramData ) );
    
    // set the header
    DatagramData* header = reinterpret_cast< DatagramData* >
                                                ( _sendBuffer.getData() );
    header->type = DATA;
    header->size = packetSize;
    header->writerID = _id;
    header->sequenceID = _sequenceID + which;

    memcpy( header + 1, ptr, packetSize );

    // send data
    _connection->waitWritable( _sendBuffer.getSize( ));
    _connection->write ( header, _sendBuffer.getSize() );
    _handleDataDatagram( _sendBuffer ); // after write, may invalidate buffer
}

void RSPConnection::_sendAckRequest( const uint16_t sequenceID )
{
#ifdef EQ_INSTRUMENT_RSP
    ++nTotalAckRequests;
#endif
    const DatagramAckRequest ackRequest = { ACKREQ, _id, sequenceID };
    _handleAckRequest( &ackRequest );
    _connection->write( &ackRequest, sizeof( DatagramAckRequest ) );
}

std::ostream& operator << ( std::ostream& os,
                            const RSPConnection& connection )
{
    os << base::disableFlush << base::disableHeader << "RSPConnection id "
       << connection.getID() << " send rate " << connection.getSendRate();

#ifdef EQ_INSTRUMENT_RSP
    os << ": read " << nBytesRead << " bytes, wrote " 
       << nBytesWritten << " bytes using " << nDataPackets << " dgrams "
       << nTotalDatagrams - nDataPackets << " repeated, "
       << nTimeOuts << " write timeouts, "
       << std::endl
       << nAckRequests << " ack requests " 
       << nTotalAckRequests - nAckRequests << " repeated, "
       << nAcksAccepted << "/" << nAcksRead << " acks read, "
       << nNAcksResend << "/" << nNAcksRead << " nacks answered, "
       << std::endl
       << nAcksSend << " acks " << nAcksSendTotal - nAcksSend << " repeated, "
       << nNAcksSend << " negative acks "

       << std::endl
       << " time in write " << nTimeInWrite 
       << " ack wait time  "  << nTimeInWriteWaitAck
       << " nTimeInReadSync " << nTimeInReadSync
       << " nTimeInReadData " << nTimeInReadData
       << " nTimeInHandleData " << nTimeInHandleData;

    nReadDataAccepted = 0;
    nReadData = 0;
    nBytesRead = 0;
    nBytesWritten = 0;
    nDataPackets = 0;
    nTotalDatagrams = 0;
    nAckRequests = 0;
    nTotalAckRequests = 0;
    nAcksSend = 0;
    nAcksSendTotal = 0;
    nAcksRead = 0;
    nAcksAccepted = 0;
    nNAcksSend = 0;
    nNAcksRead = 0;
    nNAcksResend = 0;
    nTimeOuts = 0;
    nTimeInWrite = 0;
    nTimeInWriteWaitAck = 0;
    nTimeInReadSync = 0;
    nTimeInReadData = 0;
    nTimeInHandleData = 0;
#endif
    os << base::enableHeader << base::enableFlush;

    return os;
}

}
}
