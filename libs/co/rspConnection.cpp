
/* Copyright (c) 2009, Cedric Stalder <cedric.stalder@gmail.com>
 *               2009-2012, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifdef CO_USE_BOOST
#include "connection.h"
#include "connectionDescription.h"
#include "global.h"
#include "log.h"

#include <lunchbox/rng.h>
#include <lunchbox/scopedMutex.h>
#include <lunchbox/sleep.h>

#include <boost/bind.hpp>

//#define EQ_INSTRUMENT_RSP
#define EQ_RSP_MERGE_WRITES
#define EQ_RSP_MAX_TIMEOUTS 2000

using namespace boost::asio;
#if defined __GNUC__ // Problems with boost resolver iterators in listen()
#  pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

namespace co
{

namespace
{
#ifdef EQ_INSTRUMENT_RSP
lunchbox::a_int32_t nReadData;
lunchbox::a_int32_t nBytesRead;
lunchbox::a_int32_t nBytesWritten;
lunchbox::a_int32_t nDatagrams;
lunchbox::a_int32_t nRepeated;
lunchbox::a_int32_t nMergedDatagrams;
lunchbox::a_int32_t nAckRequests;
lunchbox::a_int32_t nAcksSend;
lunchbox::a_int32_t nAcksSendTotal;
lunchbox::a_int32_t nAcksRead;
lunchbox::a_int32_t nAcksAccepted;
lunchbox::a_int32_t nNAcksSend;
lunchbox::a_int32_t nNAcksRead;
lunchbox::a_int32_t nNAcksResend;

float writeWaitTime = 0.f;
lunchbox::Clock instrumentClock;
#endif

static uint16_t _numBuffers = 0;
}

RSPConnection::RSPConnection()
        : _id( 0 )
        , _idAccepted( false )
        , _mtu( Global::getIAttribute( Global::IATTR_UDP_MTU ))
        , _ackFreq( Global::getIAttribute( Global::IATTR_RSP_ACK_FREQUENCY ))
        , _payloadSize( _mtu - sizeof( DatagramData ))
        , _timeouts( 0 )
        , _event( new EventConnection )
        , _read( 0 )
        , _write( 0 )
        , _timeout( _ioService )
        , _wakeup( _ioService )
        , _maxBucketSize( ( _mtu * _ackFreq) >> 1 )
        , _bucketSize( 0 )
        , _sendRate( 0 )
        , _thread( 0 )
        , _acked( std::numeric_limits< uint16_t >::max( ))
        , _threadBuffers( Global::getIAttribute( Global::IATTR_RSP_NUM_BUFFERS))
        , _recvBuffer( _mtu )
        , _readBuffer( 0 )
        , _readBufferPos( 0 )
        , _sequence( 0 )
{
    _buildNewID();
    _description->type = CONNECTIONTYPE_RSP;
    _description->bandwidth = 102400;

    LBCHECK( _event->connect( ));

    _buffers.reserve( Global::getIAttribute( Global::IATTR_RSP_NUM_BUFFERS ));
    while( static_cast< int32_t >( _buffers.size( )) <
           Global::getIAttribute( Global::IATTR_RSP_NUM_BUFFERS ))
    {
        _buffers.push_back( new Buffer( _mtu ));
    }

    LBASSERT( sizeof( DatagramNack ) <= size_t( _mtu ));
    LBLOG( LOG_RSP ) << "New RSP connection, " << _buffers.size()
                     << " buffers of " << _mtu << " bytes" << std::endl;
}

RSPConnection::~RSPConnection()
{
    _close();
    while( !_buffers.empty( ))
    {
        delete _buffers.back();
        _buffers.pop_back();
    }
}

void RSPConnection::_close()
{
    if( _parent.isValid() && _parent->_id == _id )
        _parent->close();

    while(( !_parent && _isWriting() ))
    {
        lunchbox::sleep( 10 );
    }

    if( _state == STATE_CLOSED )
        return;

    if( _thread )
    {
        LBASSERT( !_thread->isCurrent( ));
        _sendSimpleDatagram( ID_EXIT, _id );
        _ioService.stop();
        _thread->join();
        delete _thread;
    }

    lunchbox::ScopedWrite mutex( _mutexEvent );
    _state = STATE_CLOSING;
    if( _thread )
    {
         _thread = 0;

        // notify children to close
        for( RSPConnectionsCIter i=_children.begin(); i !=_children.end(); ++i )
        {
            RSPConnectionPtr child = *i;
            lunchbox::ScopedWrite mutexChild( child->_mutexEvent );
            child->_appBuffers.push( 0 );
            child->_event->set();
        }

        _children.clear();
        _newChildren.clear();
    }

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

    mutex.leave();
    _event->set();
}

//----------------------------------------------------------------------
// Async IO handles
//----------------------------------------------------------------------
uint16_t RSPConnection::_buildNewID()
{
    lunchbox::RNG rng;
    _id = rng.get< uint16_t >();
    return _id;
}

bool RSPConnection::listen()
{
    LBASSERT( _description->type == CONNECTIONTYPE_RSP );

    if( _state != STATE_CLOSED )
        return false;
    
    _state = STATE_CONNECTING;
    _numBuffers =  Global::getIAttribute( Global::IATTR_RSP_NUM_BUFFERS );
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
        const ip::udp::endpoint readEndpoint( readAddress, 
                                              _description->port );

        std::stringstream portStr;
        portStr << _description->port;
        const std::string& port = portStr.str();
        ip::udp::resolver resolver( _ioService );
        const ip::udp::resolver::query queryHN( ip::udp::v4(),
                                                _description->getHostname(),
                                                port );
        const ip::udp::resolver::iterator end;
        const ip::udp::resolver::iterator hostnameIP =
            resolver.resolve( queryHN );

        if( hostnameIP == end )
            return false;

        const ip::udp::endpoint writeEndpoint = *hostnameIP;
        const ip::address mcAddr( writeEndpoint.address() );

        _read = new ip::udp::socket( _ioService );
        _write = new ip::udp::socket( _ioService );
        _read->open( readEndpoint.protocol( ));
        _write->open( writeEndpoint.protocol( ));

        _read->set_option( ip::udp::socket::reuse_address( true ));
        _write->set_option( ip::udp::socket::reuse_address( true ));
        _read->set_option( ip::udp::socket::receive_buffer_size( 
                       Global::getIAttribute( Global::IATTR_UDP_BUFFER_SIZE )));
        _write->set_option( ip::udp::socket::send_buffer_size( 
                       Global::getIAttribute( Global::IATTR_UDP_BUFFER_SIZE )));

        _read->bind( readEndpoint );

        const ip::udp::resolver::query queryIF( ip::udp::v4(),
                                            _description->getInterface(), "0" );
        const ip::udp::resolver::iterator interfaceIP =
            resolver.resolve( queryIF );

        if( interfaceIP == end )
            return false;
                
        const ip::address ifAddr( ip::udp::endpoint( *interfaceIP ).address( ));
        LBINFO << "Joining " << mcAddr << " on " << ifAddr << std::endl;

        _read->set_option( ip::multicast::join_group( mcAddr.to_v4(),
                                                      ifAddr.to_v4( )));
        _write->set_option( ip::multicast::outbound_interface( ifAddr.to_v4()));

        _write->connect( writeEndpoint );

        _read->set_option( ip::multicast::enable_loopback( false ));
        _write->set_option( ip::multicast::enable_loopback( false ));
    }
    catch( const boost::system::system_error& e )
    {
        LBWARN << "can't setup underlying UDP connection: " << e.what()
               << std::endl;
        delete _read;
        delete _write;
        _read = 0;
        _write = 0;
        return false;
    }

    // init communication protocol thread
    _thread = new Thread( this );
    _bucketSize = 0;
    _sendRate = _description->bandwidth;

    // waits until RSP protocol establishes connection to the multicast network
    if( !_thread->start( ) )
    {
        close();
        return false;
    }

    // Make all buffers available for writing
    LBASSERT( _appBuffers.isEmpty( ));
    _appBuffers.push( _buffers );

    _fireStateChanged();

    LBINFO << "Listening on " << _description->getHostname() << ":"
           << _description->port << " (" << _description->toString() << " @"
           << (void*)this << ")" << std::endl;
    return true;
}

ConnectionPtr RSPConnection::acceptSync()
{
    if( _state != STATE_LISTENING )
        return 0;
        
    lunchbox::ScopedWrite mutex( _mutexConnection );
    LBASSERT( !_newChildren.empty( ));
    if( _newChildren.empty( ))
        return 0;

    RSPConnectionPtr newConnection = _newChildren.back();
    _newChildren.pop_back();

    LBINFO << _id << " accepted RSP connection " << newConnection->_id
           << std::endl;

    lunchbox::ScopedWrite mutex2( _mutexEvent );
    if( _newChildren.empty() )
        _event->reset();
    else
        _event->set();

    ConnectionPtr connection = newConnection.get();
    return connection;
}

int64_t RSPConnection::readSync( void* buffer, const uint64_t bytes, const bool)
{
    LBASSERT( bytes > 0 );
    if( _state != STATE_CONNECTED )
        return -1;

    uint64_t bytesLeft = bytes;
    uint8_t* ptr = reinterpret_cast< uint8_t* >( buffer );

    // redundant (done by the caller already), but saves some lock ops
    while( bytesLeft )
    {
        if( !_readBuffer )
        {
            LBASSERT( _readBufferPos == 0 );
            _readBuffer = _appBuffers.pop();
            if( !_readBuffer )
            {
                close();
                return (bytes == bytesLeft) ? 
                    -1 : static_cast< int64_t >( bytes - bytesLeft );
            }
        }
        LBASSERT( _readBuffer );

        const DatagramData* header = reinterpret_cast< const DatagramData* >(
            _readBuffer->getData( ));
        const uint8_t* payload = reinterpret_cast< const uint8_t* >( header+1 );
        const size_t dataLeft = header->size - _readBufferPos;
        const size_t size = LB_MIN( static_cast< size_t >( bytesLeft ),
                                    dataLeft );

        memcpy( ptr, payload + _readBufferPos, size );
        _readBufferPos += size;
        ptr += size;
        bytesLeft -= size;

        // if all data in the buffer has been taken
        if( _readBufferPos >= header->size )
        {
            LBASSERT( _readBufferPos == header->size );
            //LBLOG( LOG_RSP ) << "reset read buffer  " << header->sequence
            //                 << std::endl;

            LBCHECK( _threadBuffers.push( _readBuffer ));
            _readBuffer = 0;
            _readBufferPos = 0;
        }
        else
        {
            LBASSERT( _readBufferPos < header->size );
        }
    }

    if( _readBuffer || !_appBuffers.isEmpty( ))
        _event->set();
    else
    {
        lunchbox::ScopedWrite mutex( _mutexEvent );
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
    LBINFO << "Left RSP protocol thread" << std::endl;
}

void RSPConnection::_handleTimeout( const boost::system::error_code& error )
{
    if( error == error::operation_aborted )
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
    if( _timeouts < 20 )
    {
        LBLOG( LOG_RSP ) << "Announce " << _id << std::endl;
        _sendSimpleDatagram( ID_HELLO, _id );
    }
    else 
    {
        LBLOG( LOG_RSP ) << "Confirm " << _id << std::endl;
        _sendSimpleDatagram( ID_CONFIRM, _id );
        _addConnection( _id );
        _idAccepted = true;
        _timeouts = 0;
        // send a first datagram to announce me and discover all other
        // connections
        _sendCountNode();
    }
    _setTimeout( 10 );
}

void RSPConnection::_handleInitTimeout( )
{
    LBASSERT( _state != STATE_LISTENING );
    ++_timeouts;
    if( _timeouts < 20 )
        _sendCountNode();
    else
    {
        _state = STATE_LISTENING;
        LBINFO << "RSP connection " << _id << " listening" << std::endl;
        _timeouts = 0;
        _ioService.stop(); // thread initialized, run restarts
    } 
    _setTimeout( 10 );
}

void RSPConnection::_handleConnectedTimeout()
{
    if( _state != STATE_LISTENING )
    {
        _ioService.stop();
        return;
    }

    _processOutgoing();

    if( _timeouts >= EQ_RSP_MAX_TIMEOUTS )
    {
        LBERROR << "Too many timeouts during send: " << _timeouts << std::endl;
        _sendSimpleDatagram( ID_EXIT, _id );
        _appBuffers.pushFront( 0 ); // unlock write function
        for( RSPConnectionsCIter i =_children.begin(); i !=_children.end(); ++i)
        {
            RSPConnectionPtr child = *i;
            child->_state = STATE_CLOSING;
            child->_appBuffers.push( 0 ); // unlock read func
        }
        _ioService.stop();
    }
}

bool RSPConnection::_initThread()
{
    LBLOG( LOG_RSP ) << "Started RSP protocol thread" << std::endl;
    _timeouts = 0;
 
   // send a first datagram to announce me and discover other connections
    LBLOG( LOG_RSP ) << "Announce " << _id << std::endl;
    _sendSimpleDatagram( ID_HELLO, _id );
    _setTimeout( 10 ); 
    _asyncReceiveFrom();
    _ioService.run();
    return _state == STATE_LISTENING;
}

void RSPConnection::_runThread()
{
    //__debugbreak();
    _ioService.reset();
    _ioService.run();
}

void RSPConnection::_setTimeout( const int32_t timeOut )
{
    LBASSERT( timeOut >= 0 );
    _timeout.expires_from_now( boost::posix_time::milliseconds( timeOut ));
    _timeout.async_wait( boost::bind( &RSPConnection::_handleTimeout, this,
                                      placeholders::error ));
}

void RSPConnection::_postWakeup()
{
    _wakeup.expires_from_now( boost::posix_time::milliseconds( 0 ));
    _wakeup.async_wait( boost::bind( &RSPConnection::_handleTimeout, this,
                                     placeholders::error ));
}

void RSPConnection::_processOutgoing()
{
#ifdef EQ_INSTRUMENT_RSP
    if( instrumentClock.getTime64() > 1000 )
    {
        LBWARN << *this << std::endl;
        instrumentClock.reset();
    }
#endif

    if( !_repeatQueue.empty( ))
        _repeatData();
    else
        _writeData();

    if( !_threadBuffers.isEmpty() || !_repeatQueue.empty( ))
    {
        _setTimeout( 0 ); // call again to send remaining
        return;
    }
    // no more data to write, check/send ack request, reset timeout

    if( _writeBuffers.empty( )) // got all acks
    {
        _timeouts = 0;
        _timeout.cancel();
        return;
    }

    const int64_t timeout =
        Global::getIAttribute( Global::IATTR_RSP_ACK_TIMEOUT );
    const int64_t left = timeout - _clock.getTime64();

    if( left > 0 )
    {
        _setTimeout( left );
        return;
    }

    // (repeat) ack request
    _clock.reset();
    ++_timeouts;
    _sendAckRequest();
    _setTimeout( timeout );
}

void RSPConnection::_writeData()
{
    Buffer* buffer = 0;
    if( !_threadBuffers.pop( buffer )) // nothing to write
        return;

    _timeouts = 0;
    LBASSERT( buffer );

    // write buffer
    DatagramData* header = reinterpret_cast<DatagramData*>( buffer->getData( ));
    header->sequence = _sequence++;

#ifdef EQ_RSP_MERGE_WRITES
    if( header->size < _payloadSize && !_threadBuffers.isEmpty( ))
    {
        std::vector< Buffer* > appBuffers;
        while( header->size < _payloadSize && !_threadBuffers.isEmpty( ))
        {
            Buffer* buffer2 = 0;
            LBCHECK( _threadBuffers.getFront( buffer2 ));
            LBASSERT( buffer2 );
            DatagramData* header2 = 
                reinterpret_cast<DatagramData*>( buffer2->getData( ));

            if( uint32_t( header->size + header2->size ) > _payloadSize )
                break;

            memcpy( reinterpret_cast<uint8_t*>( header + 1 ) + header->size,
                    header2 + 1, header2->size );
            header->size += header2->size;
            LBCHECK( _threadBuffers.pop( buffer2 ));
            appBuffers.push_back( buffer2 );
#ifdef EQ_INSTRUMENT_RSP
            ++nMergedDatagrams;
#endif
        }

        if( !appBuffers.empty( ))
            _appBuffers.push( appBuffers );
    }
#endif

    // send data
    //  Note 1: We could optimize the send away if we're all alone, but this is
    //          not a use case for RSP, so we don't care.
    //  Note 2: Data to myself will be 'written' in _finishWriteQueue once we
    //          got all acks for the packet
    const uint32_t size = header->size + sizeof( DatagramData );

    _waitWritable( size ); // OPT: process incoming in between
    _write->send( boost::asio::buffer( header, size ));

#ifdef EQ_INSTRUMENT_RSP
    ++nDatagrams;
    nBytesWritten += header->size;
#endif

    // save datagram for repeats (and self)
    _writeBuffers.push_back( buffer );

    if( _children.size() == 1 ) // We're all alone
    {
        LBASSERT( _children.front()->_id == _id );
        _finishWriteQueue( _sequence - 1 );
    }
}

void RSPConnection::_waitWritable( const uint64_t bytes )
{
#ifdef EQ_INSTRUMENT_RSP
    lunchbox::Clock clock;
#endif

    _bucketSize += static_cast< uint64_t >( _clock.resetTimef() * _sendRate );
                                                     // opt omit: * 1024 / 1000;
    _bucketSize = LB_MIN( _bucketSize, _maxBucketSize );

    const uint64_t size = LB_MIN( bytes, static_cast< uint64_t >( _mtu ));
    while( _bucketSize < size )
    {
        lunchbox::Thread::yield();
        float time = _clock.resetTimef();

        while( time == 0.f )
        {
            lunchbox::Thread::yield();
            time = _clock.resetTimef();
        }

        _bucketSize += static_cast< int64_t >( time * _sendRate );
        _bucketSize = LB_MIN( _bucketSize, _maxBucketSize );
    }
    _bucketSize -= size;

#ifdef EQ_INSTRUMENT_RSP
    writeWaitTime += clock.getTimef();
#endif

    if( _sendRate < _description->bandwidth )
    {
        _sendRate += int64_t(
            float( Global::getIAttribute( Global::IATTR_RSP_ERROR_UPSCALE )) *
            float( _description->bandwidth ) * .001f );
        LBLOG( LOG_RSP ) << "speeding up to " << _sendRate << " KB/s"
                         << std::endl;
    }
}

void RSPConnection::_repeatData()
{
    _timeouts = 0;

    while( !_repeatQueue.empty( ))
    {
        Nack& request = _repeatQueue.front(); 
        const uint16_t distance = _sequence - request.start;
        LBASSERT( distance != 0 );

        if( distance <= _writeBuffers.size( )) // not already acked
        {
//          LBLOG( LOG_RSP ) << "Repeat " << request.start << ", " << _sendRate
//                           << "KB/s"<< std::endl;

            const size_t i = _writeBuffers.size() - distance;
            Buffer* buffer = _writeBuffers[i];
            LBASSERT( buffer );

            DatagramData* header = 
                reinterpret_cast<DatagramData*>( buffer->getData( ));
            const uint32_t size = header->size + sizeof( DatagramData );
            LBASSERT( header->sequence == request.start );

            // send data
            _waitWritable( size ); // OPT: process incoming in between
            _write->send( boost::asio::buffer( header, size ) );
#ifdef EQ_INSTRUMENT_RSP
            ++nRepeated;
#endif
        }

        if( request.start == request.end )
            _repeatQueue.pop_front();    // done with request
        else
            ++request.start;

        if( distance <= _writeBuffers.size( )) // send something
            return;
    }
}

void RSPConnection::_finishWriteQueue( const uint16_t sequence )
{
    LBASSERT( !_writeBuffers.empty( ));

    RSPConnectionPtr connection = _findConnection( _id );
    LBASSERT( connection.isValid( ));
    LBASSERT( connection->_recvBuffers.empty( ));

    // Bundle pushing the buffers to the app to avoid excessive lock ops
    Buffers readBuffers;
    Buffers freeBuffers;

    const uint16_t size = _sequence - sequence - 1;
    LBASSERTINFO( size <= uint16_t( _writeBuffers.size( )),
                  size << " > " << _writeBuffers.size( ));
    LBLOG( LOG_RSP ) << "Got all remote acks for " << sequence << " current "
                     << _sequence << " advance " << _writeBuffers.size() - size
                     << " buffers" << std::endl;

    while( _writeBuffers.size() > size_t( size ))
    {
        Buffer* buffer = _writeBuffers.front();
        _writeBuffers.pop_front();

#ifndef NDEBUG
        const DatagramData* datagram = 
            reinterpret_cast< const DatagramData* >( buffer->getData( ));
        LBASSERT( datagram->writerID == _id );
        LBASSERTINFO( datagram->sequence == 
                      uint16_t( connection->_sequence + readBuffers.size( )),
                      datagram->sequence << ", " << connection->_sequence <<
                      ", " << readBuffers.size( ));
      //LBLOG( LOG_RSP ) << "self receive " << datagram->sequence << std::endl;
#endif

        Buffer* newBuffer = connection->_newDataBuffer( *buffer );
        if( !newBuffer && !readBuffers.empty( )) // push prepared app buffers
        {
            lunchbox::ScopedWrite mutex( connection->_mutexEvent );
            LBLOG( LOG_RSP ) << "post " << readBuffers.size()
                             << " buffers starting with sequence "
                             << connection->_sequence << std::endl;

            connection->_appBuffers.push( readBuffers );
            connection->_sequence += uint16_t( readBuffers.size( ));
            readBuffers.clear();
            connection->_event->set();
        }

        while( !newBuffer ) // no more data buffers, wait for app to drain
        {
            newBuffer = connection->_newDataBuffer( *buffer );
            lunchbox::Thread::yield();
        }

        freeBuffers.push_back( buffer );
        readBuffers.push_back( newBuffer );
    }

    _appBuffers.push( freeBuffers );
    if( !readBuffers.empty( ))
    {
        lunchbox::ScopedWrite mutex( connection->_mutexEvent );
#if 0
        LBLOG( LOG_RSP ) 
            << "post " << readBuffers.size() << " buffers starting at "
            << connection->_sequence << std::endl;
#endif

        connection->_appBuffers.push( readBuffers );
        connection->_sequence += uint16_t( readBuffers.size( ));
        connection->_event->set();
    }

    connection->_acked = uint16_t( connection->_sequence - 1 );
    LBASSERT( connection->_acked == sequence );

    _timeouts = 0;
}

void RSPConnection::_handlePacket( const boost::system::error_code& /* error */,
                                   const size_t /* bytes */ )
{
    if( _state == STATE_LISTENING )
    {
        _handleConnectedData( _recvBuffer.getData() );

        if( _state == STATE_LISTENING )
            _processOutgoing();
        else
        {
            _ioService.stop();
            return;
        }
    }
    else if( _idAccepted )
        _handleInitData( _recvBuffer.getData() );
    else
        _handleAcceptIDData( _recvBuffer.getData() );

    //LBLOG( LOG_RSP ) << "_handlePacket timeout " << timeout << std::endl;
    _asyncReceiveFrom();
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
                LBLOG( LOG_RSP ) << "Announce " << _id << std::endl;
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
            _addConnection( node->connectionID );
            return;

        case COUNTNODE:
            _handleCountNode();
            break;
    
        case ID_EXIT:
            _removeConnection( node->connectionID );
            return;

        default:
            LBUNIMPLEMENTED;
            break;
    }
}
void RSPConnection::_handleConnectedData( const void* data )
{
    const uint16_t type = *reinterpret_cast< const uint16_t* >( data );
    switch( type )
    {
        case DATA:
            LBCHECK( _handleData( _recvBuffer ));
            break;

        case ACK:
            LBCHECK( _handleAck( 
                      reinterpret_cast< const DatagramAck* >( data )));
            break;

        case NACK:
            LBCHECK( _handleNack(
                      reinterpret_cast< const DatagramNack* >( data )));
            break;

        case ACKREQ: // The writer asks for an ack/nack
            LBCHECK( _handleAckRequest(
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
            _addConnection( node->connectionID );
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
            LBASSERTINFO( false, 
                          "Don't know how to handle packet of type " <<
                          type );
    }

}

void RSPConnection::_asyncReceiveFrom()
{
    _read->async_receive_from(
        buffer( _recvBuffer.getData(), _mtu ), _readAddr,
        boost::bind( &RSPConnection::_handlePacket, this,
                     placeholders::error,
                     placeholders::bytes_transferred ));
}

bool RSPConnection::_handleData( Buffer& buffer )
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
    LBASSERT( writerID != _id );
#endif

    RSPConnectionPtr connection = _findConnection( writerID );

    if( !connection )  // unknown connection ?
    {
        LBASSERTINFO( false, "Can't find connection with id " << writerID );
        return false;
    }
    LBASSERT( connection->_id == writerID );

    const uint16_t sequence = datagram->sequence;
//  LBLOG( LOG_RSP ) << "rcvd " << sequence << " from " << writerID <<std::endl;

    if( connection->_sequence == sequence ) // in-order packet
    {
        Buffer* newBuffer = connection->_newDataBuffer( buffer );
        if( !newBuffer ) // no more data buffers, drop packet
            return true;

        lunchbox::ScopedWrite mutex( connection->_mutexEvent );
        connection->_pushDataBuffer( newBuffer );
            
        while( !connection->_recvBuffers.empty( )) // enqueue ready pending data
        {
            newBuffer = connection->_recvBuffers.front();
            if( !newBuffer )
                break;
            
            connection->_recvBuffers.pop_front();
            connection->_pushDataBuffer( newBuffer );
        }

        if( !connection->_recvBuffers.empty() &&
            !connection->_recvBuffers.front( )) // update for new _sequence
        {
            connection->_recvBuffers.pop_front();
        }

        connection->_event->set();
        return true;
    }
    
    if( connection->_sequence > sequence ||
        uint16_t( connection->_sequence - sequence ) <= _numBuffers )
    {
        // ignore it if it's a repetition for another reader
        return true;
    }

    // else out of order

    const uint16_t size = sequence - connection->_sequence;
    LBASSERT( size != 0 );
    LBASSERTINFO( size <= _numBuffers, size << " > " << _numBuffers );

    ssize_t i = ssize_t( size ) - 1;
    const bool gotPacket = ( connection->_recvBuffers.size() >= size && 
                             connection->_recvBuffers[ i ] );
    if( gotPacket )
        return true;

    Buffer* newBuffer = connection->_newDataBuffer( buffer );
    if( !newBuffer ) // no more data buffers, drop packet
        return true;

    if( connection->_recvBuffers.size() < size )
        connection->_recvBuffers.resize( size, 0 );

    LBASSERT( !connection->_recvBuffers[ i ] );
    connection->_recvBuffers[ i ] = newBuffer;

    // early nack: request missing packets before current
    --i;
    Nack nack = { connection->_sequence, sequence - 1 };
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
            nack.start = last->sequence + 1;
        }
    }

    LBLOG( LOG_RSP ) << "send early nack " << nack.start << ".." << nack.end
                     << " current " << connection->_sequence << " ooo "
                     << connection->_recvBuffers.size() << std::endl;

    if( nack.end < nack.start )
        // OPT: don't drop nack 0..nack.end, but it doesn't happen often
        nack.end = std::numeric_limits< uint16_t >::max();

    _sendNack( writerID, &nack, 1 );
    return true;
}

RSPConnection::Buffer* RSPConnection::_newDataBuffer( Buffer& inBuffer )
{
    LBASSERT( static_cast< int32_t >( inBuffer.getMaxSize( )) == _mtu );

    Buffer* buffer = 0;
    if( _threadBuffers.pop( buffer ))
    {
        buffer->swap( inBuffer );
        return buffer;
    }

    // we do not have a free buffer, which means that the receiver is slower
    // then our read thread. This is bad, because now we'll drop the data and
    // will send a NAck packet upon the ack request, causing retransmission even
    // though we'll probably drop it again
    LBLOG( LOG_RSP ) << "Reader too slow, dropping data" << std::endl;

    // Set the event if there is data to read. This shouldn't be needed since
    // the event should be set in this case, but it'll increase the robustness
    lunchbox::ScopedWrite mutex( _mutexEvent );
    if( !_appBuffers.isEmpty( ))
        _event->set();
    return 0;
}

void RSPConnection::_pushDataBuffer( Buffer* buffer )
{
    LBASSERT( _parent );
    LBASSERTINFO( ((DatagramData*)buffer->getData( ))->sequence == _sequence,
                  ((DatagramData*)buffer->getData( ))->sequence << " != " <<
                  _sequence );

    if( (( _sequence + _parent->_id ) % _ackFreq ) == 0 )
        _parent->_sendAck( _id, _sequence );

    LBLOG( LOG_RSP ) << "post buffer " << _sequence << std::endl;
    ++_sequence;
    _appBuffers.push( buffer );
}

bool RSPConnection::_handleAck( const DatagramAck* ack )
{
#ifdef EQ_INSTRUMENT_RSP
    ++nAcksRead;
#endif

    if( ack->writerID != _id )
        return true;

    LBLOG( LOG_RSP ) << "got ack from " << ack->readerID << " for "
                     << ack->writerID << " sequence " << ack->sequence
                     << " current " << _sequence << std::endl;

    // find destination connection, update ack data if needed
    RSPConnectionPtr connection = _findConnection( ack->readerID );
    if( !connection )
    {
        LBUNREACHABLE;
        return false;
    }

    if( connection->_acked >= ack->sequence &&
        connection->_acked - ack->sequence <= _numBuffers )
    {
        // I have received a later ack previously from the reader
        LBLOG( LOG_RSP ) << "Late ack" << std::endl;
        return true;
    }

#ifdef EQ_INSTRUMENT_RSP
    ++nAcksAccepted;
#endif
    connection->_acked = ack->sequence;
    _timeouts = 0; // reset timeout counter

    // Check if we can advance _acked
    uint16_t acked = ack->sequence;

    for( RSPConnectionsCIter i = _children.begin(); i != _children.end(); ++i )
    {
        RSPConnectionPtr child = *i;
        if( child->_id == _id )
            continue;

        const uint16_t distance = child->_acked - acked;
        if( distance > _numBuffers )
            acked = child->_acked;
    }

    RSPConnectionPtr selfChild = _findConnection( _id );
    const uint16_t distance = acked - selfChild->_acked;
    if( distance <= _numBuffers )
        _finishWriteQueue( acked );
    return true;
}

bool RSPConnection::_handleNack( const DatagramNack* nack )
{
#ifdef EQ_INSTRUMENT_RSP
    ++nNAcksRead;
#endif

    if( _id != nack->writerID )
    {
        LBLOG( LOG_RSP )
            << "ignore " << nack->count << " nacks from " << nack->readerID
            << " for " << nack->writerID << " (not me)"<< std::endl;
        return true;
    }

    LBLOG( LOG_RSP )
        << "handle " << nack->count << " nacks from " << nack->readerID
        << " for " << nack->writerID << std::endl;

    RSPConnectionPtr connection = _findConnection( nack->readerID );
    if( !connection )
    {
        LBUNREACHABLE;
        return false;
        // it's an unknown connection, TODO add this connection?
    }

    _timeouts = 0;
    _addRepeat( nack->nacks, nack->count );
    return true;
}

void RSPConnection::_addRepeat( const Nack* nacks, uint16_t num )
{
    LBLOG( LOG_RSP ) << lunchbox::disableFlush << "Queue repeat requests ";
    size_t lost = 0;

    for( size_t i = 0; i < num; ++i )
    {
        const Nack& nack = nacks[ i ];
        LBASSERT( nack.start <= nack.end );

        LBLOG( LOG_RSP ) << nack.start << ".." << nack.end << " ";

        bool merged = false;
        for( RepeatQueue::iterator j = _repeatQueue.begin();
             j != _repeatQueue.end() && !merged; ++j )
        {
            Nack& old = *j;
            if( old.start <= nack.end && old.end >= nack.start )
            {
                if( old.start > nack.start )
                {
                    lost += old.start - nack.start;
                    old.start = nack.start;
                    merged = true;
                }
                if( old.end < nack.end )
                {
                    lost += nack.end - old.end;
                    old.end = nack.end;
                    merged = true;
                }
                LBASSERT( lost < _numBuffers );
            }
        }

        if( !merged )
        {
            lost += uint16_t( nack.end - nack.start ) + 1;
            LBASSERT( lost <= _numBuffers );
            _repeatQueue.push_back( nack );
        }
    }

    if( _sendRate >
        ( _description->bandwidth >> 
          Global::getIAttribute( Global::IATTR_RSP_MIN_SENDRATE_SHIFT )))
    {
        const float delta = float( lost ) * .001f *
                     Global::getIAttribute( Global::IATTR_RSP_ERROR_DOWNSCALE );
        const float maxDelta = .01f *
            float( Global::getIAttribute( Global::IATTR_RSP_ERROR_MAXSCALE ));
        const float downScale = LB_MIN( delta, maxDelta );
        _sendRate -= 1 + int64_t( _sendRate * downScale );
        LBLOG( LOG_RSP ) 
            << ", lost " << lost << " slowing down " << downScale * 100.f
            << "% to " << _sendRate << " KB/s" << std::endl 
            << lunchbox::enableFlush;
    }
    else
        LBLOG( LOG_RSP ) << std::endl << lunchbox::enableFlush;
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
    LBASSERT( writerID != _id );
#endif
    RSPConnectionPtr connection = _findConnection( writerID );
    if( !connection )
    {
        LBUNREACHABLE;
        return false;
    }

    const uint16_t reqID = ackRequest->sequence;
    const uint16_t gotID = connection->_sequence - 1;
    const uint16_t distance = reqID - gotID;

    LBLOG( LOG_RSP ) << "ack request "  << reqID << " from " << writerID
                     << " got " << gotID << " missing " << distance 
                     << std::endl;

    if( (reqID == gotID) ||
        (gotID > reqID && gotID - reqID <= _numBuffers) ||
        (gotID < reqID && distance > _numBuffers) )
    {
        _sendAck( connection->_id, gotID );
        return true;
    }
    // else find all missing datagrams

    const uint16_t max = EQ_RSP_MAX_NACKS - 2;
    Nack nacks[ EQ_RSP_MAX_NACKS ];
    uint16_t i = 0;

    nacks[ i ].start = connection->_sequence;
    LBLOG( LOG_RSP ) << lunchbox::disableFlush << "nacks: " 
                     << nacks[i].start << "..";
    
    std::deque<Buffer*>::const_iterator j = connection->_recvBuffers.begin();
    std::deque<Buffer*>::const_iterator first = j;
    for( ; j != connection->_recvBuffers.end() && i < max; ++j )
    {
        if( *j ) // got buffer
        {
            nacks[ i ].end = connection->_sequence + std::distance( first, j);
            LBLOG( LOG_RSP ) << nacks[i].end << ", ";
            if( nacks[ i ].end < nacks[ i ].start )
            {
                LBASSERT( nacks[ i ].end < _numBuffers );
                nacks[ i + 1 ].start = 0;
                nacks[ i + 1 ].end = nacks[ i ].end;
                nacks[ i ].end = std::numeric_limits< uint16_t >::max();
                ++i;
            }
            ++i;

            // find next hole
            for( ++j; j != connection->_recvBuffers.end() && (*j); ++j )
                /* nop */;
      
            if( j == connection->_recvBuffers.end( ))
                break;

            nacks[i].start = connection->_sequence + std::distance(first, j) +1;
            LBLOG( LOG_RSP ) << nacks[i].start << "..";
        }
    }

    if( j != connection->_recvBuffers.end() || i == 0 )
    {
        nacks[ i ].end = reqID;
        LBLOG( LOG_RSP ) << nacks[i].end;
        ++i;
    }
    else if( uint16_t( reqID - nacks[i-1].end ) < _numBuffers )
    {
        nacks[i].start = nacks[i-1].end + 1;
        nacks[i].end = reqID;
        LBLOG( LOG_RSP ) << nacks[i].start << ".." << nacks[i].end;
        ++i;
    }
    if( nacks[ i -1 ].end < nacks[ i - 1 ].start )
    {
        LBASSERT( nacks[ i - 1 ].end < _numBuffers );
        nacks[ i ].start = 0;
        nacks[ i ].end = nacks[ i - 1 ].end;
        nacks[ i - 1 ].end = std::numeric_limits< uint16_t >::max();
        ++i;
    }

    LBLOG( LOG_RSP ) << std::endl << lunchbox::enableFlush << "send " << i
                     << " nacks to " << connection->_id << std::endl;

    LBASSERT( i > 0 );
    _sendNack( connection->_id, nacks, i );
    return true;
}

void RSPConnection::_handleCountNode()
{
    const DatagramCount* countConn = 
        reinterpret_cast< const DatagramCount* >( _recvBuffer.getData( ));

    LBLOG( LOG_RSP ) << "Got " << countConn->numConnections << " nodes from " 
                     << countConn->clientID << std::endl;

    _addConnection( countConn->clientID );
}

void RSPConnection::_checkNewID( uint16_t id )
{
    // look if the new ID exist in another connection
    if( id == _id || _findConnection( id ).isValid() )
    {
        LBLOG( LOG_RSP ) << "Deny " << id << std::endl;
        _sendSimpleDatagram( ID_DENY, _id );
    }
}

RSPConnectionPtr RSPConnection::_findConnection( const uint16_t id )
{
    for( RSPConnectionsCIter i = _children.begin(); i != _children.end(); ++i )
        if( (*i)->_id == id )
            return *i;
    return 0;
}

bool RSPConnection::_addConnection( const uint16_t id )
{
    if( _findConnection( id ))
        return false;

    LBINFO << "add connection " << id << std::endl;
    RSPConnectionPtr connection = new RSPConnection();
    connection->_id = id;
    connection->_parent = this;
    connection->_state = STATE_CONNECTED;
    connection->_description = _description;
    LBASSERT( connection->_appBuffers.isEmpty( ));

    // Make all buffers available for reading
    for( BuffersCIter i = connection->_buffers.begin();
         i != connection->_buffers.end(); ++i )
    {
        Buffer* buffer = *i;
        LBCHECK( connection->_threadBuffers.push( buffer ));
    }

    _children.push_back( connection );
    _sendCountNode();

    lunchbox::ScopedWrite mutex( _mutexConnection );
    _newChildren.push_back( connection );

    lunchbox::ScopedWrite mutex2( _mutexEvent ); 
    _event->set();
    return true;
}

void RSPConnection::_removeConnection( const uint16_t id )
{
    LBINFO << "remove connection " << id << std::endl;
    if( id == _id )
        return;

    for( RSPConnectionsIter i = _children.begin(); i != _children.end(); ++i )
    {
        RSPConnectionPtr child = *i;
        if( child->_id == id )
        {
            _children.erase( i );

            lunchbox::ScopedWrite mutex( child->_mutexEvent ); 
            child->_appBuffers.push( 0 );
            child->_event->set();
            break;
        }
    }
    
    _sendCountNode();
}

int64_t RSPConnection::write( const void* inData, const uint64_t bytes )
{
    if ( _parent.isValid() )
        return _parent->write( inData, bytes );
    LBASSERT( _state == STATE_LISTENING );

    if( !_write )
        return -1;

    // compute number of datagrams
    uint64_t nDatagrams = bytes  / _payloadSize;    
    if( nDatagrams * _payloadSize != bytes )
        ++nDatagrams;

    // queue each datagram (might block if buffers are exhausted)
    const uint8_t* data = reinterpret_cast< const uint8_t* >( inData );
    const uint8_t* end = data + bytes;
    for( uint64_t i = 0; i < nDatagrams; ++i )
    {
        size_t packetSize = end - data;
        packetSize = LB_MIN( packetSize, _payloadSize );

        if( _appBuffers.isEmpty( ))
            // trigger processing
            _postWakeup();

        Buffer* buffer = _appBuffers.pop();
        if( !buffer )
        {
            close();
            return -1;
        }

        // prepare packet header (sequence is done by thread)
        DatagramData* header =
            reinterpret_cast< DatagramData* >( buffer->getData( ));
        header->type = DATA;
        header->size = uint16_t( packetSize );
        header->writerID = _id;
        
        memcpy( header + 1, data, packetSize );
        data += packetSize;

        LBCHECK( _threadBuffers.push( buffer ));
    }
    _postWakeup();
    LBLOG( LOG_RSP ) << "queued " << nDatagrams << " datagrams, " 
                     << bytes << " bytes" << std::endl;
    return bytes;
}

void RSPConnection::finish()
{
    if( _parent.isValid( ))
    {
        LBASSERTINFO( !_parent, "Writes are only allowed on RSP listeners" );
        return;
    }
    LBASSERT( _state == STATE_LISTENING );
    _appBuffers.waitSize( _buffers.size( ));
}

void RSPConnection::_sendCountNode()
{
    if( !_findConnection( _id ))
        return;

    LBLOG( LOG_RSP ) << _children.size() << " nodes" << std::endl;
    const DatagramCount count = { COUNTNODE, _id, uint16_t( _children.size( ))};
    _write->send( buffer( &count, sizeof( count )) );
}

void RSPConnection::_sendSimpleDatagram( DatagramType type, uint16_t id )
{
    const DatagramNode simple = { type, id };
    _write->send( buffer( &simple, sizeof( simple )) );
}

void RSPConnection::_sendAck( const uint16_t writerID,
                              const uint16_t sequence )
{
    LBASSERT( _id != writerID );
#ifdef EQ_INSTRUMENT_RSP
    ++nAcksSend;
#endif

    LBLOG( LOG_RSP ) << "send ack " << sequence << std::endl;
    const DatagramAck ack = { ACK, _id, writerID, sequence };
    _write->send( buffer( &ack, sizeof( ack )) );
}

void RSPConnection::_sendNack( const uint16_t writerID, const Nack* nacks,
                               const uint16_t count )
{
    LBASSERT( count > 0 );
    LBASSERT( count <= EQ_RSP_MAX_NACKS );
#ifdef EQ_INSTRUMENT_RSP
    ++nNAcksSend;
#endif
    /* optimization: use the direct access to the reader. */
    if( writerID == _id )
    {
         _addRepeat( nacks, count );
         return;
    }

    const size_t size = sizeof( DatagramNack ) - 
                        (EQ_RSP_MAX_NACKS - count) * sizeof( Nack );

    // set the header
    DatagramNack packet;
    packet.set( _id, writerID, count );
    memcpy( packet.nacks, nacks, count * sizeof( Nack ));
    _write->send( buffer( &packet, size ));
}

void RSPConnection::_sendAckRequest()
{
#ifdef EQ_INSTRUMENT_RSP
    ++nAckRequests;
#endif
    LBLOG( LOG_RSP ) << "send ack request for " << uint16_t( _sequence -1 )
                     << std::endl;
    const DatagramAckRequest ackRequest = { ACKREQ, _id, _sequence - 1 };
    _write->send( buffer( &ackRequest, sizeof( DatagramAckRequest )) );
}

std::ostream& operator << ( std::ostream& os,
                            const RSPConnection& connection )
{
    os << lunchbox::disableFlush << lunchbox::disableHeader 
       << "RSPConnection id " << connection.getID() << " send rate " 
       << connection.getSendRate();

#ifdef EQ_INSTRUMENT_RSP
    const int prec = os.precision();
    os.precision( 3 );

    const float time = instrumentClock.getTimef();
    const float mbps = 1048.576f * time;
    os << ": " << lunchbox::indent << std::endl
       << float( nBytesRead ) / mbps << " / " << float( nBytesWritten ) / mbps
       <<  " MB/s r/w using " << nDatagrams << " dgrams " << nRepeated
       << " repeats " << nMergedDatagrams
       << " merged"
       << std::endl;

    os.precision( prec );
    os << "sender: " << nAckRequests << " ack requests " << nAcksAccepted << "/"
       << nAcksRead << " acks " << nNAcksRead << " nacks, throttle "
       << writeWaitTime << " ms"
       << std::endl
       << "receiver: " << nAcksSend << " acks " << nNAcksSend << " nacks"
       << lunchbox::exdent;

    nReadData = 0;
    nBytesRead = 0;
    nBytesWritten = 0;
    nDatagrams = 0;
    nRepeated = 0;
    nMergedDatagrams = 0;
    nAckRequests = 0;
    nAcksSend = 0;
    nAcksRead = 0;
    nAcksAccepted = 0;
    nNAcksSend = 0;
    nNAcksRead = 0;
    writeWaitTime = 0.f;
#endif
    os << std::endl << lunchbox::enableHeader << lunchbox::enableFlush;

    return os;
}

}
#endif //CO_USE_BOOST
