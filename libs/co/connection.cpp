
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "connection.h"

#include "connectionDescription.h"
#include "connectionListener.h"
#include "log.h"
#include "pipeConnection.h"
#include "socketConnection.h"
#include "rspConnection.h"

#ifdef _WIN32
#  include "namedPipeConnection.h"
#endif

#include <co/exception.h>

#ifdef EQ_INFINIBAND
#  include "IBConnection.h"
#endif
#ifdef CO_USE_OFED
#  include "rdmaConnection.h"
#endif
#ifdef CO_USE_UDT
#  include "udtConnection.h"
#endif

#include <lunchbox/scopedMutex.h>
#include <lunchbox/stdExt.h>

namespace co
{
namespace detail
{
class Connection
{
public:
    co::Connection::State state; //!< The connection state
    ConnectionDescriptionPtr description; //!< The connection parameters

    /** The lock used to protect concurrent write calls. */
    mutable lunchbox::Lock sendLock;

    void*         aioBuffer;
    uint64_t      aioBytes;

    /** The listeners on state changes */
    ConnectionListeners listeners;

    Connection()
            : state( co::Connection::STATE_CLOSED )
            , description( new ConnectionDescription )
            , aioBuffer( 0 )
            , aioBytes( 0 )
    {
        description->type = CONNECTIONTYPE_NONE;
    }

    ~Connection()
    {
        LBASSERT( state == co::Connection::STATE_CLOSED );
        state = co::Connection::STATE_CLOSED;
        description = 0;

        // LBASSERTINFO( !aioBytes && aioBytes == 0,
        //               "Pending IO operation during connection destruction" );
    }

    void fireStateChanged( co::Connection* connection )
    {
        for( ConnectionListeners::const_iterator i= listeners.begin();
             i != listeners.end(); ++i )
        {
            (*i)->notifyStateChanged( connection );
        }
    }
};
}

Connection::Connection()
        : _impl( new detail::Connection )
{
    LBVERB << "New Connection @" << (void*)this << std::endl;
}

Connection::~Connection()
{
    delete _impl;
    LBVERB << "Delete Connection @" << (void*)this << std::endl;
}

bool Connection::operator == ( const Connection& rhs ) const
{
    if( this == &rhs )
        return true;
    if( _impl->description->type != CONNECTIONTYPE_PIPE )
        return false;
    Connection* pipe = const_cast< Connection* >( this );
    return pipe->acceptSync().get() == &rhs;
}

ConnectionPtr Connection::create( ConnectionDescriptionPtr description )
{
    ConnectionPtr connection;
    switch( description->type )
    {
        case CONNECTIONTYPE_TCPIP:
        case CONNECTIONTYPE_SDP:
            connection = new SocketConnection( description->type );
            break;

        case CONNECTIONTYPE_PIPE:
            connection = new PipeConnection;
            break;

#ifdef _WIN32
        case CONNECTIONTYPE_NAMEDPIPE:
            connection = new NamedPipeConnection;
            break;
#endif

#ifdef EQ_INFINIBAND
        case CONNECTIONTYPE_IB:
            connection = new IBConnection;
            break;
#endif
        case CONNECTIONTYPE_RSP:
            connection = new RSPConnection;
            break;

#ifdef CO_USE_OFED
        case CONNECTIONTYPE_RDMA:
            connection = new RDMAConnection;
            break;
#endif
#ifdef CO_USE_UDT
        case CONNECTIONTYPE_UDT:
            connection = new UDTConnection;
            break;
#endif

        default:
            LBWARN << "Connection type " << description->type
                   << " not supported" << std::endl;
            return 0;
    }

    if( description->bandwidth == 0 )
        description->bandwidth = connection->getDescription()->bandwidth;

    connection->_setDescription( description );
    return connection;
}

Connection::State Connection::getState() const
{
    return _impl->state;
}

void Connection::_setDescription( ConnectionDescriptionPtr description )
{
    LBASSERT( description.isValid( ));
    LBASSERTINFO( _impl->description->type == description->type,
                  "Wrong connection type in description" );
    _impl->description = description;
    LBASSERT( description->bandwidth > 0 );
}

void Connection::_setState( const State state )
{
    if( _impl->state == state )
        return;
    _impl->state = state;
    _impl->fireStateChanged( this );
}

void Connection::lockSend() const
{
    _impl->sendLock.set();
}

void Connection::unlockSend() const
{
    _impl->sendLock.unset();
}

void Connection::addListener( ConnectionListener* listener )
{
    _impl->listeners.push_back( listener );
}

void Connection::removeListener( ConnectionListener* listener )
{
    ConnectionListeners::iterator i = find( _impl->listeners.begin(),
                                            _impl->listeners.end(), listener );
    if( i != _impl->listeners.end( ))
        _impl->listeners.erase( i );
}

//----------------------------------------------------------------------
// read
//----------------------------------------------------------------------
void Connection::recvNB( void* buffer, const uint64_t bytes )
{
    LBASSERT( !_impl->aioBuffer );
    LBASSERT( !_impl->aioBytes );
    LBASSERT( buffer );
    LBASSERT( bytes );

    _impl->aioBuffer = buffer;
    _impl->aioBytes  = bytes;
    readNB( buffer, bytes );
}

bool Connection::recvSync( void** outBuffer, uint64_t* outBytes,
                           const bool block )
{
    // set up async IO data
    LBASSERT( _impl->aioBuffer );
    LBASSERT( _impl->aioBytes );

    if( outBuffer )
        *outBuffer = _impl->aioBuffer;
    if( outBytes )
        *outBytes = _impl->aioBytes;

    void* buffer( _impl->aioBuffer );
    const uint64_t bytes( _impl->aioBytes );
    _impl->aioBuffer = 0;
    _impl->aioBytes  = 0;

    if( _impl->state != STATE_CONNECTED || !buffer || !bytes )
        return false;

    // 'Iterator' data for receive loop
    uint8_t* ptr = static_cast< uint8_t* >( buffer );
    uint64_t bytesLeft = bytes;

    // WAR: On Win32, we get occasionally a data notification and then deadlock
    // when reading from the connection. The callee (Node::handleData) will flag
    // the first read, the underlying SocketConnection will not block and we
    // will restore the AIO operation if no data was present.
    int64_t got = readSync( ptr, bytesLeft, block );
    if( got == READ_TIMEOUT ) // fluke notification
    {
        LBASSERTINFO( bytesLeft == bytes, bytesLeft << " != " << bytes );
        if( outBytes )
            *outBytes = 0;

        _impl->aioBuffer = buffer;
        _impl->aioBytes  = bytes;
        return true;
    }

    // From here on, blocking receive loop until all data read or error
    while( true )
    {
        if( got < 0 ) // error
        {
            if( outBytes )
                *outBytes -= bytesLeft;
            if( bytes == bytesLeft )
                LBINFO << "Read on dead connection" << std::endl;
            else
                LBERROR << "Error during read after " << bytes - bytesLeft
                        << " bytes on " << _impl->description << std::endl;
            return false;
        }
        else if( got == 0 )
        {
            // ConnectionSet::select may report data on an 'empty' connection.
            // If we have nothing read so far, we have hit this case.
            if( bytes == bytesLeft )
            {
                if( outBytes )
                    *outBytes = 0;
                return false;
            }
            LBVERB << "Zero bytes read" << std::endl;
        }

        if( bytesLeft > static_cast< uint64_t >( got )) // partial read
        {
            ptr += got;
            bytesLeft -= got;

            readNB( ptr, bytesLeft );
            got = readSync( ptr, bytesLeft, true );
        }
        else
        {
            LBASSERTINFO( static_cast< uint64_t >( got ) == bytesLeft,
                          got << " != " << bytesLeft );

#ifndef NDEBUG
            if( bytes <= 1024 && ( lunchbox::Log::topics & LOG_PACKETS ))
            {
                ptr = static_cast< uint8_t* >( buffer );
                LBINFO << "recv:" << std::hex << lunchbox::disableFlush
                       << lunchbox::disableHeader;
                for( size_t i = 0; i < bytes; ++i )
                {
                    if( (i % 16) == 0 )
                        LBINFO << std::endl;
                    if( (i % 4) == 0 )
                        LBINFO << " 0x";
                    LBINFO << std::setfill( '0' ) << std::setw(2)
                           << static_cast< unsigned >( ptr[ i ] );
                }
                LBINFO << std::dec << lunchbox::enableFlush
                       << std::endl << lunchbox::enableHeader;
            }
#endif
            return true;
        }
    }

    LBUNREACHABLE;
    return true;
}

void Connection::resetRecvData( void** buffer, uint64_t* bytes )
{
    *buffer = _impl->aioBuffer;
    *bytes = _impl->aioBytes;
    _impl->aioBuffer = 0;
    _impl->aioBytes = 0;
}

//----------------------------------------------------------------------
// write
//----------------------------------------------------------------------
bool Connection::send( const void* buffer, const uint64_t bytes,
                       const bool isLocked )
{
    LBASSERT( bytes > 0 );
    if( bytes == 0 )
        return true;

    const uint8_t* ptr = static_cast< const uint8_t* >( buffer );

    // possible OPT: We need to lock here to guarantee an atomic transmission of
    // the buffer. Possible improvements are:
    // 1) Disassemble buffer into 'small enough' pieces and use a header to
    //    reassemble correctly on the other side (aka reliable UDP)
    // 2) Introduce a send thread with a thread-safe task queue
    lunchbox::ScopedMutex<> mutex( isLocked ? 0 : &_impl->sendLock );

#ifndef NDEBUG
    if( bytes <= 1024 && ( lunchbox::Log::topics & LOG_PACKETS ))
    {
        LBINFO << "send:" << std::hex << lunchbox::disableFlush
               << lunchbox::disableHeader << std::endl;
        for( size_t i = 0; i < bytes; ++i )
        {
            if( (i % 16) == 0 )
                LBINFO << std::endl;
            if( (i % 4) == 0 )
                LBINFO << " 0x";
            LBINFO << std::setfill( '0' ) << std::setw(2)
                   << static_cast< unsigned >( ptr[ i ] );
        }
        LBINFO << std::dec << lunchbox::enableFlush << std::endl
               << lunchbox::enableHeader;
    }
#endif

    uint64_t bytesLeft = bytes;
    while( bytesLeft )
    {
        try
        {
            const int64_t wrote = this->write( ptr, bytesLeft );
            if( wrote == -1 ) // error
            {
                LBERROR << "Error during write after " << bytes - bytesLeft
                        << " bytes, closing connection" << std::endl;
                close();
                return false;
            }
            else if( wrote == 0 )
                LBINFO << "Zero bytes write" << std::endl;

            bytesLeft -= wrote;
            ptr += wrote;
        }
        catch( const co::Exception& e )
        {
            LBERROR << e.what() << " after " << bytes - bytesLeft
                    << " bytes, closing connection" << std::endl;
            close();
            return false;
        }

    }
    return true;
}

bool Connection::send( Packet& packet, const void* data,
                       const uint64_t dataSize )
{
    if( dataSize == 0 )
        return send( packet );

    if( dataSize <= 8 ) // fits in existing packet
    {
#ifndef NDEBUG // fill all eight bytes in debug to keep valgrind happy
        *(uint64_t*)((char*)(&packet) + packet.size - 8) = 0;
#endif
        memcpy( (char*)(&packet) + packet.size - 8, data, dataSize );
        return send( packet );
    }
    // else

    const uint64_t headerSize  = packet.size - 8;
    const uint64_t size        = headerSize + dataSize;
    if( size > EQ_ASSEMBLE_THRESHOLD )
    {
        // OPT: lock the connection and use two send() to avoid big memcpy
        packet.size = size;

        lockSend();
        const bool ret = ( send( &packet, headerSize, true ) &&
                           send( data,    dataSize,   true ));
        unlockSend();
        return ret;
    }
    // else

    char* buffer = (char*)alloca( size );

    memcpy( buffer,              &packet, headerSize );
    memcpy( buffer + headerSize, data,    dataSize );

    ((Packet*)buffer)->size = size;
    return send( buffer, size );
}

bool Connection::send( const Connections& connections,
                       const Packet& packet, const bool isLocked )
{
    if( connections.empty( ))
        return true;

    bool success = true;
    for( Connections::const_iterator i= connections.begin();
         i<connections.end(); ++i )
    {
        ConnectionPtr connection = *i;
        if( !connection->send( &packet, packet.size, isLocked ))
            success = false;
    }
    return success;
}

bool Connection::send( const Connections& connections, Packet& packet,
                       const void* data, const uint64_t dataSize,
                       const bool isLocked )
{
    if( connections.empty( ))
        return true;

    if( dataSize <= 8 ) // fits in existing packet
    {
        if( dataSize != 0 )
            memcpy( (char*)(&packet) + packet.size-8, data, dataSize );
        return send( connections, packet, isLocked );
    }

    const uint64_t headerSize  = packet.size - 8;
    const uint64_t size        = headerSize + dataSize;

    if( size > EQ_ASSEMBLE_THRESHOLD )
    {
        // OPT: lock the connection and use two send() to avoid big memcpy
        packet.size = size;
        bool success = true;

        for( Connections::const_iterator i= connections.begin();
             i<connections.end(); ++i )
        {
            ConnectionPtr connection = *i;

            if( !isLocked )
                connection->lockSend();

            if( !connection->send( &packet, headerSize, true ) ||
                !connection->send( data, dataSize, true ))
            {
                success = false;
            }
            if( !isLocked )
                connection->unlockSend();
        }
        return success;
    }

    char*          buffer = (char*)alloca( size );
    memcpy( buffer, &packet, packet.size-8 );
    memcpy( buffer + packet.size-8, data, dataSize );

    ((Packet*)buffer)->size = size;

    bool success = true;
    for( Connections::const_iterator i = connections.begin();
         i < connections.end(); ++i )
    {
        ConnectionPtr connection = *i;
        if( !connection->send( buffer, size, isLocked ))
            success = false;
    }

    return success;
}

bool Connection::send( const Connections& connections, Packet& packet,
                       const void* const* items, const uint64_t* sizes,
                       const size_t nItems )
{
    if( connections.empty( ))
        return true;

    packet.size -= 8;
    const uint64_t headerSize = packet.size;
    for( size_t i = 0; i < nItems; ++i )
    {
        LBASSERTINFO( sizes[i] != 0, i );
        packet.size += sizes[ i ] + sizeof( uint64_t );
    }

    bool success = true;
    for( Connections::const_iterator i = connections.begin();
         i < connections.end(); ++i )
    {
        ConnectionPtr connection = *i;
        connection->lockSend();

        if( !connection->send( &packet, headerSize, true ))
            success = false;

        for( size_t j = 0; j < nItems; ++j )
            if( !connection->send( &sizes[j], sizeof(uint64_t), true ) ||
                !connection->send( items[j], sizes[j], true ))
            {
                success = false;
            }

        connection->unlockSend();
    }
    return success;
}

ConstConnectionDescriptionPtr Connection::getDescription() const
{
    return _impl->description;
}

ConnectionDescriptionPtr Connection::_getDescription()
{
    return _impl->description;
}

std::ostream& operator << ( std::ostream& os, const Connection& connection )
{
    Connection::State        state = connection.getState();
    ConstConnectionDescriptionPtr desc  = connection.getDescription();

    os << "Connection " << (void*)&connection << " type "
       << typeid( connection ).name() << " state "
       << ( state == Connection::STATE_CLOSED     ? "closed" :
            state == Connection::STATE_CONNECTING ? "connecting" :
            state == Connection::STATE_CONNECTED  ? "connected" :
            state == Connection::STATE_LISTENING  ? "listening" :
            state == Connection::STATE_CLOSING    ? "closing" :
            "UNKNOWN" );
    if( desc.isValid( ))
        os << " description " << desc->toString();

    return os;
}
}
