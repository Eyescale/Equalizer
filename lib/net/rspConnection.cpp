
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
#ifdef EQ_USE_BOOST
#include "connection.h"
#include "connectionDescription.h"
#include "global.h"
#include "log.h"

#include <eq/base/rng.h>
#include <eq/base/sleep.h>

#include <boost/bind.hpp>

//#define EQ_INSTRUMENT_RSP
#define EQ_RSP_MERGE_WRITES

using std::distance;
using namespace boost::asio;

namespace eq
{
namespace net
{

namespace
{
#ifdef EQ_INSTRUMENT_RSP
base::a_int32_t nReadData;
base::a_int32_t nBytesRead;
base::a_int32_t nBytesWritten;
base::a_int32_t nDataPackets;
base::a_int32_t nTotalDatagrams;
base::a_int32_t nMergedDatagrams;
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
#endif
}

RSPConnection::RSPConnection()
        : _id( 0 )
        , _idAccepted( false )
        , _mtu( Global::getIAttribute( Global::IATTR_UDP_MTU ))
        , _ackFreq( Global::getIAttribute( Global::IATTR_UDP_PACKET_RATE ))
        , _maxBucketSize( ( _mtu * _ackFreq) >> 1 )
        , _payloadSize( _mtu - sizeof( DatagramData ))
        , _maxNAck( (_mtu - sizeof( DatagramNack )) / sizeof( uint32_t ))
        , _timeouts( 0 )
        , _sendBuffer( _mtu )
        , _event( new EventConnection )
        , _read( 0 )
        , _write( 0 )
        , _timeout( _ioService )
        , _wakeup( _ioService )
        , _allowedData( 0 )
        , _sendRate( 0 )
        , _numWriteAcks( 0 )
        , _thread ( 0 )
        , _parent( 0 )
        , _ackReceived( -1 )
        , _lastAck( -1 )
        , _ackSend( false )
        , _threadBuffers( Global::getIAttribute( Global::IATTR_RSP_NUM_BUFFERS))
        , _recvBuffer( _mtu )
        , _readBuffer( 0 )
        , _readBufferPos( 0 )
        , _sequenceID( 0 )
{
    _buildNewID();
    _description->type = CONNECTIONTYPE_RSP;
    _description->bandwidth = 102400;

    EQCHECK( _event->connect( ));

    _maxNAck = EQ_MIN( _maxNAck, _ackFreq );
    EQASSERT( _maxNAck < std::numeric_limits< uint16_t >::max( ));

    _buffers.reserve( Global::getIAttribute( Global::IATTR_RSP_NUM_BUFFERS ));
    while( static_cast< int32_t >( _buffers.size( )) <
           Global::getIAttribute( Global::IATTR_RSP_NUM_BUFFERS ))
    {
        _buffers.push_back( new Buffer( _mtu ));
    }

    _nackBuffer.reserve( _mtu );
    EQLOG( LOG_RSP ) << "New RSP Connection, " << _buffers.size() 
                     << " packet buffer of size " << _mtu << std::endl;
}

RSPConnection::~RSPConnection()
{
    close();

    while( !_buffers.empty( ))
    {
        delete _buffers.back();
        _buffers.pop_back();
    }
}

void RSPConnection::close()
{
    if( _parent.isValid() && _parent->_id == _id )
        _parent->close();

    while(( !_parent && _isWriting() ))
    {
        base::sleep( 10 );
    }
    _close();
    _event->set();
}

void RSPConnection::_close()
{
    if( _state == STATE_CLOSED )
        return;
    _state = STATE_CLOSING;

    if( _thread )
    {
        EQASSERT( !_thread->isCurrent( ));
        _sendSimpleDatagram( ID_EXIT, _id );
        _ioService.stop();
        _thread->join();
        _thread = 0;
    }

    base::ScopedMutex<> mutex( _mutexEvent );
    for( RSPConnectionVector::iterator i = _children.begin();
         i != _children.end(); ++i )
    {
        RSPConnectionPtr child = *i;
        base::ScopedMutex<> mutexChild( child->_mutexEvent );
        child->_appBuffers.push( 0 );
        child->_event->set();
    }

    _children.clear();
    _childrenConnecting.clear();
    _parent = 0;

    if( _read )
        _read->close();
    delete _read;
    _read = 0;

    if( _write )
        _write->close();
    delete _write;
    _write = 0;

    _threadBuffers.clear();
    _appBuffers.push( 0 ); // unlock any other read/write threads

    _state = STATE_CLOSED;
    _fireStateChanged();
}

//----------------------------------------------------------------------
// Async IO handles
//----------------------------------------------------------------------
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
    if( _description->port == 0 )
        _description->port = EQ_DEFAULT_PORT;
    if( _description->getHostname().empty( ))
        _description->setHostname( "239.255.42.43" );
    if( _description->getInterface().empty( ))
        _description->setInterface( "0.0.0.0" );

    try
    {
        const ip::address readAddress( ip::address::from_string( "0.0.0.0" ));
        const ip::udp::endpoint readEndpoint( readAddress, _description->port );

        const ip::address mcAddr(
            ip::address::from_string( _description->getHostname( )));
        const ip::udp::endpoint writeEndpoint( mcAddr, _description->port );

        _read = new ip::udp::socket( _ioService );
        _write = new ip::udp::socket( _ioService );
        _read->open( readEndpoint.protocol( ));
        _write->open( writeEndpoint.protocol( ));

        _read->set_option( ip::udp::socket::reuse_address( true ));
        _write->set_option( ip::udp::socket::reuse_address( true ));

        _read->bind( readEndpoint );

        const ip::address ifAddr( 
            ip::address::from_string( _description->getInterface( )));

        _read->set_option( ip::multicast::join_group( mcAddr.to_v4(),
                                                      ifAddr.to_v4( )));
        _write->set_option( ip::multicast::outbound_interface( ifAddr.to_v4()));

        _write->connect( writeEndpoint );

        _read->set_option( ip::multicast::enable_loopback( false ));
        _write->set_option( ip::multicast::enable_loopback( false ));
    }
    catch( boost::system::system_error& error )
    {
        EQWARN << "can't setup underlying UDP connection " << error.what()
               << std::endl;
        delete _read;
        delete _write;
        _read = 0;
        _write = 0;
        return false;
    }

    // init a thread to manage the communication protocol 
    _thread = new Thread( this );
    _numWriteAcks =  0;
    _allowedData = 0;
    _sendRate = _description->bandwidth;

    // waits until RSP protocol establishes connection to the multicast network
    if( !_thread->start( ) )
    {
        close();
        return false;
    }

    // Make all buffers available for writing
    EQASSERT(_appBuffers.isEmpty());
    _appBuffers.push( _buffers );

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
        
    // protect event->set, _children and _childrenConnecting
    base::ScopedMutex<> mutexConn( _mutexConnection );
    EQASSERT( !_childrenConnecting.empty( ));
    if( _childrenConnecting.empty( ))
        return 0;

    RSPConnectionPtr newConnection = _childrenConnecting.back();
    _childrenConnecting.pop_back();
    _children.push_back( newConnection );
    _sendDatagramCountNode();

    EQINFO << "accepted RSP connection " << newConnection->_id << std::endl;

    if ( !_childrenConnecting.empty() )
        _event->set();
    else 
        _event->reset();

    ConnectionPtr connection = newConnection.get();
    return connection;
}

int64_t RSPConnection::readSync( void* buffer, const uint64_t bytes )
{
    EQASSERT( bytes > 0 );
    if( _state != STATE_CONNECTED )
        return -1;

    uint64_t bytesLeft = bytes;
    uint8_t* ptr = reinterpret_cast< uint8_t* >( buffer );

    while( bytesLeft ) // this is somewhat redundant (done by higher-level
                       // function already), but saves quiet a few lock ops
    {
        if( !_readBuffer )
        {
            EQASSERT( _readBufferPos == 0 );
            _readBuffer = _appBuffers.pop();
            if( !_readBuffer )
            {
                close();
                return (bytes == bytesLeft) ? 
                    -1 : static_cast< int64_t >( bytes - bytesLeft );
            }
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

            EQCHECK( _threadBuffers.push( _readBuffer ));
            _readBuffer = 0;
            _readBufferPos = 0;
        }
        else
            EQASSERT( _readBufferPos < header->size );
    }

    if( _readBuffer || !_appBuffers.isEmpty( ))
        _event->set();
    else
    {
        base::ScopedMutex<> mutex( _mutexEvent );
        if( _appBuffers.isEmpty( ))
            _event->reset();

    }

#ifdef EQ_INSTRUMENT_RSP
    nBytesRead += bytes;
#endif
    return bytes;
}

void RSPConnection::Thread::run()
{
    _connection->_runThread();
    _connection = 0;
    EQINFO << "Left RSP protocol thread" << std::endl;
}

void RSPConnection::_handleTimeout( const boost::system::error_code& error )
{
    if( error == boost::asio::error::operation_aborted )
        return;
    
    if( _state == STATE_LISTENING )
        _handleConnectedTimeout();
    else if( _idAccepted )
        _handleInitTimeout();
    else
        _handleAcceptIDTimeout();
}

void RSPConnection::_handleAcceptIDTimeout( )
{
    ++_timeouts;
    if ( _timeouts < 20 )
    {
        EQLOG( LOG_RSP ) << "Announce " << _id << std::endl;
        _sendSimpleDatagram( ID_HELLO, _id );
    }
    else 
    {
        EQLOG( LOG_RSP ) << "Confirm " << _id << std::endl;
        EQINFO << "opened RSP connection " << _id << std::endl;
        _sendSimpleDatagram( ID_CONFIRM, _id );
        _addNewConnection( _id );
        _idAccepted = true;
        _timeouts = 0;
        // send a first datagram to announce me and discover all other
        // connections
        _sendDatagramCountNode();
    }
    _resetTimeout( 10 );
}

void RSPConnection::_handleInitTimeout( )
{
    EQASSERT( _state != STATE_LISTENING );
    ++_timeouts;
    if( _timeouts < 20 )
        _sendDatagramCountNode();
    else
    {
        _state = STATE_LISTENING;
        _timeouts = 0;
        if( _children.empty() )
            _ioService.stop();
    } 
    _resetTimeout( 10 );
}

void RSPConnection::_handleConnectedTimeout( )
{
    if( _ackSend && _repeatQueue.empty( ))
    {
#ifdef EQ_INSTRUMENT_RSP
        ++nTimeOuts;
#endif
        ++_timeouts;
        if( _timeouts >= 
            Global::getIAttribute( Global::IATTR_RSP_MAX_TIMEOUTS ))
        {
            EQERROR << "Too many timeouts during send: " << _timeouts
                    << std::endl;
            _sendSimpleDatagram( ID_EXIT, _id );
            _appBuffers.pushFront( 0 ); // unlock write function
            for( RSPConnectionVector::iterator i = _children.begin();
                 i != _children.end(); ++i )
            {
                RSPConnectionPtr child = *i;
                child->_state = STATE_CLOSING;
                child->_appBuffers.push( 0 ); // unlock read func
            }
            _ioService.stop();
            return;
        }
                
        // repeat ack request
        EQASSERT( _sequenceID > 0 );
        _sendAckRequest( _sequenceID - 1 );
    }
    else
        _timeouts = 0;

    if( _state == STATE_LISTENING )
    {
        const int32_t timeout = _handleWrite();
        if( timeout >= 0 )
            EQLOG( LOG_RSP ) << "_handleTimeout timeout " 
                             << timeout << std::endl;
        _resetTimeout( timeout );   
    }
    else
        _ioService.stop();

}

bool RSPConnection::_initThread()
{
    EQINFO << "Started RSP protocol thread" << std::endl;
    _timeouts = 0;
 
   // send a first datagram for announce me and discover other connection 
    EQLOG( LOG_RSP ) << "Announce " << _id << std::endl;
    _sendSimpleDatagram( ID_HELLO, _id );
    _resetTimeout( 10 ); 
    _asyncReceiveFrom();
    _ioService.run();
    return  _state == STATE_LISTENING;
}

void RSPConnection::_runThread()
{
    //__debugbreak();
    EQINFO << "Started RSP protocol thread" << std::endl;
    _ioService.reset();
    _ioService.run();
}

void RSPConnection::_resetTimeout( int32_t timeOut )
{
    if( timeOut >= 0 )
    {
        _timeout.expires_from_now( boost::posix_time::milliseconds( timeOut ));
        _timeout.async_wait( boost::bind( &RSPConnection::_handleTimeout, this,
                                          placeholders::error ));
    }
    else
        _timeout.cancel();
}

void RSPConnection::_postWakeup()
{
    _wakeup.expires_from_now( boost::posix_time::milliseconds( 0 ));
    _wakeup.async_wait( boost::bind( &RSPConnection::_handleTimeout, this,
                                      placeholders::error ));
}

int32_t RSPConnection::_handleWrite()
{
    if( !_repeatQueue.empty( ))
    {
        _handleRepeat();
        return 0;
    }

    if( _ackSend ) // wait for all acks
    {
        EQASSERT( !_writeBuffers.isEmpty( ));
        return Global::getIAttribute( Global::IATTR_RSP_ACK_TIMEOUT );
    }

    _timeouts = 0;
    Buffer* buffer = 0;
    if( !_threadBuffers.pop( buffer )) // nothing to write (and to repeat)
        return -1;

    EQASSERT( buffer );

    // write buffer
    DatagramData* header = reinterpret_cast<DatagramData*>( buffer->getData( ));
    header->sequenceID = _sequenceID++;
#ifdef EQ_RSP_MERGE_WRITES
    if( header->size < _payloadSize && !_threadBuffers.isEmpty( ))
    {
        std::vector< Buffer* > appBuffers;
        while( header->size < _payloadSize && !_threadBuffers.isEmpty( ))
        {
            Buffer* buffer2 = 0;
            EQCHECK( _threadBuffers.getFront( buffer2 ));
            EQASSERT( buffer2 );
            DatagramData* header2 = 
                reinterpret_cast<DatagramData*>( buffer2->getData( ));
            
            if( header->size + header2->size > _payloadSize )
                break;

            memcpy( reinterpret_cast< uint8_t* >( header + 1 ) + header->size,
                    header2 + 1, header2->size );
            header->size += header2->size;
            EQCHECK( _threadBuffers.pop( buffer2 ));
            appBuffers.push_back( buffer2 );
#ifdef EQ_INSTRUMENT_RSP
            ++nMergedDatagrams;
#endif
        }

        if( !appBuffers.empty( ))
            _appBuffers.push( appBuffers );
    }
#endif
    const uint32_t size = header->size + sizeof( DatagramData );

    // send data
    //  Note: We could optimize the send away if we're all alone, but this
    //        is not a use case for RSP, so we don't care.
    _waitWritable( size ); // OPT: process incoming in between
    _write->send( boost::asio::buffer( header, size ));
    // Note: the data to myself will be 'written' in _finishWriteQueue once
    // we've got all other acks

#ifdef EQ_INSTRUMENT_RSP
    ++nTotalDatagrams;
    ++nDataPackets;
    nBytesWritten += header->size;
#endif

    // save datagram for repeats
    _writeBuffers.append( buffer );

    if( _writeBuffers.getSize() < static_cast< size_t >( _ackFreq ) &&
        !_threadBuffers.isEmpty( ))
    {
        return 0; // call again
    }

    EQASSERT( !_children.empty( ));
    EQLOG( LOG_RSP ) << "wrote sequence of " << _writeBuffers.getSize() 
                     << " datagrams" << std::endl;

    // Send ack request when needed
    EQASSERT( !_ackSend );
    _ackSend = true;
    _timeouts = 0;
    _numWriteAcks = 1; // self ack request - will handle in _finishWriteQueue

    if( _numWriteAcks == _children.size( )) // We're all alone
    {
        EQASSERT( _children.front()->_id == _id );
        _finishWriteQueue();
        return _threadBuffers.isEmpty() ? -1 : 0;
    }
    // else

    EQLOG( LOG_RSP ) << "Send ack request for sequence " 
                     << _sequenceID - _writeBuffers.getSize() << ".."
                     << _sequenceID - 1 << std::endl;

#ifdef EQ_INSTRUMENT_RSP
    ++nAckRequests;
#endif
    _sendAckRequest( _sequenceID - 1 );
    return Global::getIAttribute( Global::IATTR_RSP_ACK_TIMEOUT );
}

void RSPConnection::_waitWritable( const uint64_t bytes )
{
    _allowedData += static_cast< uint64_t >( _clock.getResetTimef() *
                                             _sendRate ); // opt: * 1024 / 1000;
    _allowedData = EQ_MIN( _allowedData, _maxBucketSize );

    const uint64_t size = EQ_MIN( bytes, static_cast< uint64_t >( _mtu ));
    while( _allowedData < size )
    {
        eq::base::sleep( 1 );
        _allowedData += static_cast< int64_t >( _clock.getResetTimef() *
                                                _sendRate );
        _allowedData = EQ_MIN( _allowedData, _maxBucketSize );
    }
    _allowedData -= size;
}

void RSPConnection::_handleRepeat()
{
    EQASSERT( !_repeatQueue.empty( ));
    _timeouts = 0;
    RepeatRequest& request = _repeatQueue.front(); 
    EQASSERT( request.start <= request.end );
    EQASSERT( request.end < _sequenceID );
    EQASSERTINFO( request.start >= _sequenceID - _writeBuffers.getSize(),
                  request.start << ", " << _sequenceID << ", " <<
                  _writeBuffers.getSize( ));

    EQLOG( LOG_RSP ) << "Repeat datagram " << request.start << std::endl;

    const size_t i = _writeBuffers.getSize() - ( _sequenceID - request.start );
    Buffer* buffer = _writeBuffers[i];
    DatagramData* header = reinterpret_cast<DatagramData*>( buffer->getData( ));
    const uint32_t size = header->size + sizeof( DatagramData );
    EQASSERT( header->sequenceID == request.start );

    // send data
    _waitWritable( size ); // OPT: process incoming in between
    _write->send( boost::asio::buffer( header, size ) );
#ifdef EQ_INSTRUMENT_RSP
    ++nTotalDatagrams;
#endif

    if( request.start == request.end )
    {
#ifdef EQ_INSTRUMENT_RSP
        ++nNAcksResend;
#endif
        _repeatQueue.pop_front();    // done with request
        if( _repeatQueue.empty( )) // re-request ack
        {
            if( _ackSend )
                _sendAckRequest( _sequenceID - 1 );
        }
    }
    else
        ++request.start;
}

void RSPConnection::_finishWriteQueue()
{
    EQASSERT( !_writeBuffers.isEmpty( ));
    EQASSERT( _ackSend );
    EQASSERT( _numWriteAcks == _children.size( ));

    RSPConnectionPtr connection = _findConnection( _id );
    EQASSERT( connection.isValid( ));
    EQASSERT( connection->_recvBuffers.empty( ));

    // Bundle pushing the buffers to the app to avoid excessive lock ops
    BufferVector readBuffers;
    BufferVector freeBuffers;

    for( size_t i = 0; i < _writeBuffers.getSize(); ++ i)
    {
        Buffer* buffer = _writeBuffers[i];

#ifndef NDEBUG
        const DatagramData* datagram = 
            reinterpret_cast< const DatagramData* >( buffer->getData( ));
        EQASSERT( datagram->writerID == _id );
        EQASSERT( datagram->sequenceID == 
                  connection->_sequenceID + readBuffers.size( ));
        EQLOG( LOG_RSP ) << "receive " << datagram->size
                         << " bytes from self, sequence "
                         << datagram->sequenceID << std::endl;
#endif

        Buffer* newBuffer = connection->_newDataBuffer( *buffer );
        if( !newBuffer && !readBuffers.empty( )) // push prepared app buffers
        {
            base::ScopedMutex<> mutex( connection->_mutexEvent );
            EQLOG( LOG_RSP ) << "post " << readBuffers.size()
                             << " buffers starting with sequence "
                             << connection->_sequenceID << std::endl;

            connection->_appBuffers.push( readBuffers );
            connection->_sequenceID += readBuffers.size();
            readBuffers.clear();
            connection->_event->set();
        }

        while( !newBuffer ) // no more data buffers, wait for app to drain
        {
            newBuffer = connection->_newDataBuffer( *buffer );
            base::sleep( 1 );
        }

        freeBuffers.push_back( buffer );
        readBuffers.push_back( newBuffer );
    }

    _appBuffers.push( freeBuffers );
    if( !readBuffers.empty( ))
    {
        base::ScopedMutex<> mutex( connection->_mutexEvent );
        EQLOG( LOG_RSP ) << "post " << readBuffers.size()
                         << " buffers starting with sequence "
                         << connection->_sequenceID << std::endl;

        connection->_appBuffers.push( readBuffers );
        connection->_sequenceID += readBuffers.size();
        connection->_event->set();
    }

    _writeBuffers.resize( 0 );
    connection->_lastAck = connection->_sequenceID - 1;

    if( _sequenceID > 32768 ) // reset regularly
    {
        _sequenceID = 0;    
        connection->_sequenceID = 0;
    }

    _adaptSendRate( 1, 0 ); // speed up again
    _ackSend = false;
    _numWriteAcks = 0;
    _timeouts = 0;

#ifdef EQ_INSTRUMENT_RSP
    if( nBytesWritten > 10000000 )
        EQWARN << *this << std::endl;
#endif
}

void RSPConnection::_handleData( const boost::system::error_code& error,
                                 const size_t bytes )
{
    int32_t timeout = 10;

    if( _state == STATE_LISTENING )
    {
        _handleConnectedData( _recvBuffer.getData() );

        if( _state == STATE_LISTENING )
            timeout = _handleWrite();
        else
        {
            _ioService.stop();
            return;
        }
    }
    else if ( _idAccepted )
        _handleInitData( _recvBuffer.getData() );
    else
        _handleAcceptIDData( _recvBuffer.getData() );

    EQLOG( LOG_RSP ) << "_handleData timeout " << timeout << std::endl;
    
    _asyncReceiveFrom();
    _resetTimeout( timeout );
}

void RSPConnection::_handleAcceptIDData( const void* data )
{
    const uint16_t type = *reinterpret_cast< const uint16_t* >( data );
    const DatagramNode* node = 
        reinterpret_cast< const DatagramNode* >( data );
    switch( type )
    {
        case ID_HELLO:
            _checkNewID( node->connectionID );
            break;

        case ID_DENY:
            // a connection refused my ID, try another ID
            if( node->connectionID == _id ) 
            {
                _timeouts = 0;
                _sendSimpleDatagram( ID_HELLO, _buildNewID() );
                EQLOG( LOG_RSP ) << "Announce " << _id << std::endl;
            }
            break;

        case ID_EXIT:
            _removeConnection( node->connectionID );
            break;

        default:
            break;    
    }
}

void RSPConnection::_handleInitData( const void* data)
{
    const uint16_t type = *reinterpret_cast< const uint16_t* >( data );
    const DatagramNode* node = reinterpret_cast< const DatagramNode* >( data );
    switch( type )
    {
        case ID_HELLO:
            _timeouts = 0;
            _checkNewID( node->connectionID ) ;
            return;

        case ID_CONFIRM:
            _timeouts = 0;
            _addNewConnection( node->connectionID );
            return;

        case COUNTNODE:
            if( _handleCountNode( ))
                _state = STATE_LISTENING;
            break;
    
        case ID_EXIT:
            _removeConnection( node->connectionID );
            return;

        default:
            EQUNIMPLEMENTED;
            break;
    }
}
void RSPConnection::_handleConnectedData( const void* data )
{
    const uint16_t type = *reinterpret_cast< const uint16_t* >( data );
    switch( type )
    {
        case DATA:
            EQCHECK( _handleDataDatagram( _recvBuffer ));
            break;

        case ACK:
            EQCHECK( _handleAck( 
                      reinterpret_cast< const DatagramAck* >( data )));
            break;

        case NACK:
            EQCHECK( _handleNack(
                      reinterpret_cast< const DatagramNack* >( data )));
            break;

        case ACKREQ: // The writer asks for an ack/nack
            EQCHECK( _handleAckRequest(
                reinterpret_cast< const DatagramAckRequest* >( data )));
            break;

        case ID_HELLO:
        {
            const DatagramNode* node =
                reinterpret_cast< const DatagramNode* >( data );
            _checkNewID( node->connectionID );
            break;
        }

        case ID_CONFIRM:
        {
            const DatagramNode* node =
                reinterpret_cast< const DatagramNode* >( data );
            _addNewConnection( node->connectionID );
            break;
        }

        case ID_EXIT:
        {
            const DatagramNode* node = 
                reinterpret_cast< const DatagramNode* >( data );
            _removeConnection( node->connectionID );
            break;
        }

        case COUNTNODE:
            _handleCountNode();
            break;

        default:
            EQASSERTINFO( false, 
                          "Don't know how to handle packet of type " <<
                          type );
    }

}

void RSPConnection::_asyncReceiveFrom()
{
    _read->async_receive_from(
        buffer( _recvBuffer.getData(), _mtu ), _readAddr,
        boost::bind( &RSPConnection::_handleData, this,
                     placeholders::error,
                     placeholders::bytes_transferred ));
}

bool RSPConnection::_handleDataDatagram( Buffer& buffer )
{
#ifdef EQ_INSTRUMENT_RSP
    ++nReadData;
#endif
    const DatagramData* datagram = 
        reinterpret_cast< const DatagramData* >( buffer.getData( ));
    const uint16_t writerID = datagram->writerID;
#ifdef Darwin
    // There is occasionally a packet from ourselves, even though multicast loop
    // is not set?!
    if( writerID == _id )
        return true;
#else
    EQASSERT( writerID != _id );
#endif

    RSPConnectionPtr connection = _findConnection( writerID );

    // it's an unknown connection or when we run netperf client before
    // server netperf
    // TODO find a solution in this situation - _addNewConnection()?
    if( !connection )
    {
        EQASSERTINFO( false, "Can't find connection with id " << writerID );
        return false;
    }
    EQASSERT( connection->_id == writerID );


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
    else if( sequenceID - connection->_sequenceID >= _ackFreq )
        return true;

    EQLOG( LOG_RSP ) << "receive " << datagram->size << " bytes from "
                     << writerID << ", sequence " << sequenceID << std::endl;

    if( connection->_sequenceID == sequenceID ) // in-order packet
    {
        Buffer* newBuffer = connection->_newDataBuffer( buffer );
        if( !newBuffer ) // no more data buffers, drop packet
            return true;

        base::ScopedMutex<> mutex( connection->_mutexEvent );
        EQLOG( LOG_RSP ) << "post buffer with sequence " << sequenceID
                         << std::endl;

        ++connection->_sequenceID;
        connection->_appBuffers.push( newBuffer );

        while( !connection->_recvBuffers.empty( )) // enqueue ready pending data
        {
            newBuffer = connection->_recvBuffers.front();
            if( !newBuffer )
                break;
            
            connection->_recvBuffers.pop_front();
            EQASSERT( reinterpret_cast<DatagramData*>( 
                          newBuffer->getData( ))->sequenceID ==
                      connection->_sequenceID );
            ++connection->_sequenceID;
            connection->_appBuffers.push( newBuffer );
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
    ssize_t i = size - 1;
    EQASSERT( size < static_cast< size_t >( _ackFreq ));

    if( connection->_recvBuffers.size() >= size && 
        connection->_recvBuffers[ i ] ) // repetition
    {
        return true;
    }

    Buffer* newBuffer = connection->_newDataBuffer( buffer );
    if( !newBuffer ) // no more data buffers, drop packet
        return true;

    if( connection->_recvBuffers.size() < size )
        connection->_recvBuffers.resize( size, 0 );

    EQASSERT( !connection->_recvBuffers[ i ] );
    connection->_recvBuffers[ i ] = newBuffer;

    // early nack: request missing packets before current
    --i;
    uint16_t nacks[2] = { connection->_sequenceID, sequenceID - 1 };
    if( i > 0 )
    {
        if( connection->_recvBuffers[i] ) // got previous packet
            return true;

        while( i >= 0 && !connection->_recvBuffers[i] )
            --i;

        const Buffer* lastBuffer = i>=0 ? connection->_recvBuffers[i] : 0;
        if( lastBuffer )
        {
            const DatagramData* last = 
                reinterpret_cast<const DatagramData*>( lastBuffer->getData( ));
            nacks[0] = last->sequenceID + 1;
        }
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
    if( !_threadBuffers.pop( buffer ))
    {
        // we do not have a free buffer, which means that the receiver is slower
        // then our read thread. This is bad, because now we'll drop the data
        // and will send a NACK packet upon the ack request, causing
        // retransmission even though we'll probably drop it again
        EQLOG( LOG_RSP ) << "Reader too slow, dropping data" << std::endl;
        return 0;
    }

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

    if ( ack->writerID != _id )
        return true;

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

    // ignore sequenceID which is different from my write sequence id
    //  Reason : - repeated, late ack or an ack for another writer
    if( _id != ack->writerID || _sequenceID - 1 != ack->sequenceID )
    {
        EQLOG( LOG_RSP ) << "ignore ack " << ack->sequenceID << " for "
                         << ack->writerID << ", it's not for me (" 
                         << _sequenceID << ", " << _id << ")" << std::endl;
        return true;
    }
    
    EQASSERT( _ackSend );
    EQASSERT( _numWriteAcks > 0 ); // self must be there
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

    EQLOG( LOG_RSP ) << "Got all remote acks for " << _sequenceID - 1
                     << std::endl;
    _repeatQueue.clear();
    _finishWriteQueue();
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

    if( _id != nack->writerID )
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

    const int32_t acked = connection->_ackReceived;
    const int32_t current = nack->sequenceID;

    if(( acked >= current && acked - current < 16384 ) ||
       ( current - acked > 16384 && current < _ackFreq ))
    {
        EQLOG( LOG_RSP ) << "ignore nack for sequence " << current
                         << ", already got ack " << acked << std::endl;
        return true;
    }

    _timeouts = 0;
    _addRepeat( reinterpret_cast< const uint16_t* >( nack + 1 ), nack->count );
    return true;
}

void RSPConnection::_addRepeat( const uint16_t* nacks, uint16_t num )
{
    EQLOG( LOG_RSP ) << base::disableFlush << "Queue repeat requests ";

    for( size_t i = 0; i < num; ++i )
    {
        RepeatRequest request( nacks[ i * 2 ], nacks[ i * 2 + 1 ] );
        EQLOG( LOG_RSP ) << request.start << ".." << request.end << " ";
        EQASSERT( request.start <= request.end );
        EQASSERT( request.end - request.start + 1 <= _writeBuffers.getSize( ));

        bool merged = false;
        for( std::deque< RepeatRequest >::iterator j = _repeatQueue.begin();
             j != _repeatQueue.end() && !merged; ++j )
        {
            RepeatRequest& old = *j;
            if( old.start <= request.end && old.end >= request.start )
            {
                old.start = EQ_MIN( old.start, request.start );
                old.end   = EQ_MAX( old.end, request.end );
                merged    = true;
            }
        }

        if( !merged )
            _repeatQueue.push_back( request );
    }
    EQLOG( LOG_RSP ) << std::endl << base::enableFlush;

    size_t nErrors = 0;
    for( std::deque< RepeatRequest >::const_iterator i = _repeatQueue.begin();
         i != _repeatQueue.end(); ++i )
    {
        const RepeatRequest& request = *i;
        nErrors += request.end - request.start + 1;
    }
    _adaptSendRate( _writeBuffers.getSize(), nErrors );
}

bool RSPConnection::_handleAckRequest( const DatagramAckRequest* ackRequest )
{
    const uint16_t writerID = ackRequest->writerID;
#ifdef Darwin
    // There is occasionally a packet from ourselves, even though multicast loop
    // is not set?!
    if( writerID == _id )
        return true;
#else
    EQASSERT( writerID != _id );
#endif

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

        EQLOG( LOG_RSP ) << "nack all datagrams ( 0.." << sequenceID << ")" 
                         << std::endl;

        const uint16_t nacks[2] = { 0, sequenceID };
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
                nacks[ num++ ] = connection->_sequenceID+distance( first, i )+1;
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
    const DatagramCount* countConn = 
    reinterpret_cast< const DatagramCount* >( _recvBuffer.getData( ));

    EQLOG( LOG_RSP ) << "Got " << countConn->numConnections << " from " 
                     << countConn->clientID << std::endl;
    // we know all connections
    if( _children.size() == countConn->numConnections ) 
        return true;

    _addNewConnection( countConn->clientID );

    return false;
}

void RSPConnection::_checkNewID( uint16_t id )
{
    // look if the new ID exist in another connection
    if( id == _id || _findConnection( id ).isValid() )
    {
        EQLOG( LOG_RSP ) << "Deny " << id << std::endl;
        _sendSimpleDatagram( ID_DENY, _id );
    }
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
        return false;

    base::ScopedMutex<> mutexConn( _mutexConnection );
    for( std::vector< RSPConnectionPtr >::const_iterator i = _children.begin();
         i != _children.end(); ++i )
    {
        if( (*i)->_id == id )
            return false;
    }
    for( std::vector< RSPConnectionPtr >::const_iterator i = 
             _childrenConnecting.begin();
         i != _childrenConnecting.end(); ++i )
    {
        if( (*i)->_id == id )
            return false;
    }

    RSPConnectionPtr connection = new RSPConnection();
    connection->_id            = id;
    connection->_parent        = this;
    connection->_state         = STATE_CONNECTED;
    connection->_description   = _description;
    EQASSERT( connection->_appBuffers.isEmpty( ));

    // Make all buffers available for reading
    for( BufferVector::iterator i = connection->_buffers.begin();
        i != connection->_buffers.end(); ++i )
    {
        Buffer* buffer = *i;
        EQCHECK( connection->_threadBuffers.push( buffer ));
    }

    _childrenConnecting.push_back( connection );
    EQINFO << "new rsp connection " << id << std::endl;
    _event->set();
    return true;
}

void RSPConnection::_removeConnection( const uint16_t id )
{
    EQINFO << "remove connection " << id << std::endl;
    if( id == _id )
        return;

    for( std::vector< RSPConnectionPtr >::iterator i = _children.begin(); 
          i != _children.end(); ++i )
    {
        RSPConnectionPtr child = *i;
        if( child->_id == id )
        {
            base::ScopedMutex<> mutex( _mutexEvent ); 
            _children.erase( i );
                
            child->_appBuffers.push( 0 );
            child->_event->set();
            break;
        }
    }
    
    _sendDatagramCountNode();
}

int64_t RSPConnection::write( const void* inData, const uint64_t bytes )
{
    EQASSERT( !_parent );
    EQASSERT( _state == STATE_LISTENING );

    if ( !_write )
        return -1;

    // compute number of datagrams
    uint64_t nDatagrams = bytes  / _payloadSize;    
    if ( nDatagrams * _payloadSize != bytes )
        ++nDatagrams;

    // queue each datagram (might block if buffers exhausted)
    const uint8_t* data = reinterpret_cast< const uint8_t* >( inData );
    const uint8_t* end = data + bytes;
    for( uint64_t i = 0; i < nDatagrams; ++i )
    {
        size_t packetSize = end - data;
        packetSize = EQ_MIN( packetSize, _payloadSize );

        if( (i % _ackFreq) == static_cast< uint32_t >( _ackFreq >> 1 ) ||
            _appBuffers.isEmpty( ))
        {
            // trigger processing
            _postWakeup();
        }
        Buffer* buffer = _appBuffers.pop();
        if( !buffer )
        {
            close();
            return -1;
        }

        // prepare packet header (sequenceID is done by thread)
        DatagramData* header =
            reinterpret_cast< DatagramData* >( buffer->getData( ));
        header->type = DATA;
        header->size = packetSize;
        header->writerID = _id;
        
        memcpy( header + 1, data, packetSize );
        data += packetSize;

        EQCHECK( _threadBuffers.push( buffer ));
    }
    _postWakeup();
    EQLOG( LOG_RSP ) << "queued " << nDatagrams << " datagrams, " 
                     << bytes << " bytes" << std::endl;
    return bytes;
}

void RSPConnection::_adaptSendRate( const size_t nPackets, const size_t nErrors)
{
    EQASSERT( nPackets > 0 );
    const float error = ( static_cast< float >( nErrors ) /
                          static_cast< float >( nPackets ) * 100.f ) - 
        Global::getIAttribute( Global::IATTR_RSP_ERROR_BASE_RATE );

    int32_t delta = 0;
    if ( error < 0.f )
        delta = static_cast< int32_t >( error *
                      Global::getIAttribute( Global::IATTR_RSP_ERROR_UPSCALE ));
    else
        delta = static_cast< int32_t >( error /
                    Global::getIAttribute( Global::IATTR_RSP_ERROR_DOWNSCALE ));        
    
    delta = EQ_MIN( Global::getIAttribute( Global::IATTR_RSP_ERROR_MAX ),
                        delta );
    EQLOG( LOG_RSP ) << nErrors << "/" << nPackets
                         << " errors, change send rate by " << -delta << "%"
                         << std::endl;
    
    _sendRate += _sendRate * (-delta) / 100;
    _sendRate = EQ_MAX( _description->bandwidth/50, _sendRate );
    _sendRate = EQ_MIN( _sendRate, _description->bandwidth );
    EQLOG( LOG_RSP ) << "new send rate " << _sendRate << std::endl;
}

void RSPConnection::_sendDatagramCountNode()
{
    if ( !_findConnection( _id ) )
        return;

    EQLOG( LOG_RSP ) << _children.size() << " nodes" << std::endl;
    const DatagramCount count = { COUNTNODE, _id, _children.size() };
    _write->send( buffer( &count, sizeof( count )) );
}

void RSPConnection::_sendSimpleDatagram( DatagramType type, uint16_t id )
{
    const DatagramNode simple = { type, id };
    _write->send( buffer( &simple, sizeof( simple )) );
}

void RSPConnection::_sendAck( const uint16_t writerID, 
                              const uint16_t sequenceID )
{
    EQASSERT( _id != writerID );

#ifdef EQ_INSTRUMENT_RSP
    ++nAcksSendTotal;
#endif
    const DatagramAck ack = { ACK, _id, writerID, sequenceID };
    _write->send( buffer( &ack, sizeof( ack )) );
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

    header->set( _id, writerID, sequenceID, count );

    memcpy( header + 1, repeats, size - sizeof( DatagramNack ));
    _write->send( buffer( header, size ));
}

void RSPConnection::_sendAckRequest( const uint16_t sequenceID )
{
    EQASSERT( _ackSend );
#ifdef EQ_INSTRUMENT_RSP
    ++nTotalAckRequests;
#endif
    EQLOG( LOG_RSP ) << "send ack request for " << sequenceID << std::endl;
    const DatagramAckRequest ackRequest = { ACKREQ, _id, sequenceID };
    _write->send( buffer( &ackRequest, sizeof( DatagramAckRequest )) );
}

std::ostream& operator << ( std::ostream& os,
                            const RSPConnection& connection )
{
    os << base::disableFlush << base::disableHeader << "RSPConnection id "
       << connection.getID() << " send rate " << connection.getSendRate();

#ifdef EQ_INSTRUMENT_RSP
    os << ": read " << nBytesRead << " bytes, wrote " 
       << nBytesWritten << " bytes using " << nDataPackets << " dgrams "
       << nMergedDatagrams << " merged " << nTotalDatagrams - nDataPackets
       << " repeated, " << nTimeOuts << " write timeouts, "
       << std::endl
       << nAckRequests << " ack requests " 
       << nTotalAckRequests - nAckRequests << " repeated, "
       << nAcksAccepted << "/" << nAcksRead << " acks read, "
       << nNAcksResend << "/" << nNAcksRead << " nacks answered, "
       << std::endl
       << nAcksSend << " acks " << nAcksSendTotal - nAcksSend << " repeated, "
       << nNAcksSend << " negative acks ";

    nReadData = 0;
    nBytesRead = 0;
    nBytesWritten = 0;
    nDataPackets = 0;
    nTotalDatagrams = 0;
    nMergedDatagrams = 0;
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
#endif
    os << base::enableHeader << base::enableFlush;

    return os;
}

}
}
#endif //EQ_USE_BOOST
