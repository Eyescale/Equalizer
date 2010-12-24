
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include "mcipConnection.h"
#include "log.h"
#include "node.h"
#include "pipeConnection.h"
#include "socketConnection.h"
#include "rspConnection.h"

#ifdef _WIN32
#  include "namedPipeConnection.h"
#endif
#ifdef EQ_INFINIBAND
#  include "IBConnection.h"
#endif
#ifdef EQ_PGM
#  include "pgmConnection.h"
#endif

#include <errno.h>

using namespace co::base;
using namespace std;

namespace co
{

Connection::Connection()
        : _state( STATE_CLOSED )
        , _description( new ConnectionDescription )
        , _aioBuffer( 0 )
        , _aioBytes( 0 )
{
    _description->type = CONNECTIONTYPE_NONE;
    EQVERB << "New Connection @" << (void*)this << endl;
}

Connection::~Connection()
{
    if( !isClosed( ))
        close();
    _state = STATE_CLOSED;
    _description = 0;

//    EQASSERTINFO( !_aioBytes && _aioBytes == 0,
//                  "Pending IO operation during connection destruction" );

    EQVERB << "Delete Connection @" << (void*)this << endl;
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

        case CONNECTIONTYPE_MCIP:
            connection = new MCIPConnection;
            break;

#ifdef EQ_PGM
        case CONNECTIONTYPE_PGM:
            connection = new PGMConnection;
            break;

#endif
#ifdef EQ_USE_BOOST
        case CONNECTIONTYPE_RSP:
            connection = new RSPConnection;
            break;
#endif
        default:
            EQWARN << "Connection type not implemented" << endl;
            return connection;
    }

    if( description->bandwidth == 0 )
        description->bandwidth = connection->getDescription()->bandwidth;

    connection->setDescription( description );
    return connection;
}

void Connection::addListener( ConnectionListener* listener )
{
    _listeners.push_back( listener );
}

void Connection::removeListener( ConnectionListener* listener )
{
    vector< ConnectionListener* >::iterator i = find( _listeners.begin(),
                                                      _listeners.end(),
                                                      listener );
    if( i != _listeners.end( ))
        _listeners.erase( i );
}

void Connection::_fireStateChanged()
{
    for( vector<ConnectionListener*>::const_iterator i= _listeners.begin();
         i != _listeners.end(); ++i )

        (*i)->notifyStateChanged( this );
}


//----------------------------------------------------------------------
// read
//----------------------------------------------------------------------
void Connection::recvNB( void* buffer, const uint64_t bytes )
{
    EQASSERT( !_aioBuffer );
    EQASSERT( !_aioBytes );
    EQASSERT( buffer );
    EQASSERT( bytes );

    _aioBuffer = buffer;
    _aioBytes  = bytes;
    readNB( buffer, bytes );
}

bool Connection::recvSync( void** outBuffer, uint64_t* outBytes,
                           const bool block )
{
    // set up async IO data
    EQASSERT( _aioBuffer );
    EQASSERT( _aioBytes );

    if( outBuffer )
        *outBuffer = _aioBuffer;
    if( outBytes )
        *outBytes = _aioBytes;

    void* buffer( _aioBuffer );
    const uint64_t bytes( _aioBytes );
    _aioBuffer = 0;
    _aioBytes  = 0;

    if( _state != STATE_CONNECTED || !buffer || !bytes )
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
        EQASSERTINFO( bytesLeft == bytes, bytesLeft << " != " << bytes );
        if( outBytes )
            *outBytes = 0;

        _aioBuffer = buffer;
        _aioBytes  = bytes;
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
                EQINFO << "Read on dead connection" << endl;
            else
                EQERROR << "Error during read after " << bytes - bytesLeft
                        << " bytes on " << _description << endl;
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
            EQVERB << "Zero bytes read" << endl;
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
            EQASSERTINFO( static_cast< uint64_t >( got ) == bytesLeft,
                          got << " != " << bytesLeft );

#ifndef NDEBUG
            if( bytes <= 1024 && ( co::base::Log::topics & LOG_PACKETS ))
            {
                ptr = static_cast< uint8_t* >( buffer );
                EQINFO << "recv:" << std::hex << co::base::disableFlush
                       << co::base::disableHeader;
                for( size_t i = 0; i < bytes; ++i )
                {
                    if( (i % 16) == 0 )
                        EQINFO << std::endl;
                    if( (i % 4) == 0 )
                        EQINFO << " 0x";
                    EQINFO << std::setfill( '0' ) << std::setw(2)
                           << static_cast< unsigned >( ptr[ i ] );
                }
                EQINFO << std::dec << co::base::enableFlush
                       << std::endl << co::base::enableHeader;
            }
#endif
            return true;
        }
    }

    EQUNREACHABLE;
    return true;
}

//----------------------------------------------------------------------
// write
//----------------------------------------------------------------------
bool Connection::send( const void* buffer, const uint64_t bytes, 
                       const bool isLocked )
{
    EQASSERT( bytes > 0 );
    if( bytes == 0 )
        return true;

    const uint8_t* ptr = static_cast< const uint8_t* >( buffer );

    // possible OPT: We need to lock here to guarantee an atomic transmission of
    // the buffer. Possible improvements are:
    // 1) Disassemble buffer into 'small enough' pieces and use a header to
    //    reassemble correctly on the other side (aka reliable UDP)
    // 2) Introduce a send thread with a thread-safe task queue
    ScopedMutex<> mutex( isLocked ? 0 : &_sendLock );

#ifndef NDEBUG
    if( bytes <= 1024 && ( co::base::Log::topics & LOG_PACKETS ))
    {
        EQINFO << "send:" << std::hex << co::base::disableFlush
               << co::base::disableHeader << std::endl;
        for( size_t i = 0; i < bytes; ++i )
        {
            if( (i % 16) == 0 )
                EQINFO << std::endl;
            if( (i % 4) == 0 )
                EQINFO << " 0x";
            EQINFO << std::setfill( '0' ) << std::setw(2)
                   << static_cast< unsigned >( ptr[ i ] );
        }
        EQINFO << std::dec << co::base::enableFlush
               << std::endl << co::base::enableHeader;
    }
#endif

    uint64_t bytesLeft = bytes;
    while( bytesLeft )
    {
        const int64_t wrote = this->write( ptr, bytesLeft );

        if( wrote == -1 ) // error
        {
            EQERROR << "Error during write after " << bytes - bytesLeft 
                    << " bytes" << endl;
            return false;
        }
        else if( wrote == 0 )
            EQWARN << "Zero bytes write" << endl;

        bytesLeft -= wrote;
        ptr += wrote;
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
        memcpy( (char*)(&packet) + packet.size-8, data, dataSize );
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

    for( Connections::const_iterator i= connections.begin(); 
         i<connections.end(); ++i )
    {        
        ConnectionPtr connection = *i;
        if( !connection->send( &packet, packet.size, isLocked ))
            return false;
    }
    return true;
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

        for( Connections::const_iterator i= connections.begin(); 
             i<connections.end(); ++i )
        {        
            ConnectionPtr connection = *i;

            if( !isLocked )
                connection->lockSend();
            const bool ok = (connection->send( &packet, headerSize, true ) &&
                             connection->send( data, dataSize, true ));
            if( !isLocked )
                connection->unlockSend();
            if( !ok )
                return false;
        }
        return true;
    }

    char*          buffer = (char*)alloca( size );
    memcpy( buffer, &packet, packet.size-8 );
    memcpy( buffer + packet.size-8, data, dataSize );

    ((Packet*)buffer)->size = size;

    for( Connections::const_iterator i = connections.begin(); 
         i < connections.end(); ++i )
    {        
        ConnectionPtr connection = *i;
        if( !connection->send( buffer, size, isLocked ))
            return false;
    }

    return true;
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
        EQASSERT( sizes[i] > 0 );
        packet.size += sizes[ i ] + sizeof( uint64_t );
    }

    for( Connections::const_iterator i = connections.begin(); 
         i < connections.end(); ++i )
    {        
        ConnectionPtr connection = *i;
        connection->lockSend();
            
        bool ok = connection->send( &packet, headerSize, true );

        for( size_t j = 0; j < nItems; ++j )
            ok = ok && connection->send( &sizes[j], sizeof(uint64_t), true )
                    && connection->send( items[j], sizes[j], true );

        connection->unlockSend();
        if( !ok )
            return false;
    }
    return true;
}

ConnectionDescriptionPtr Connection::getDescription() const
{
    return _description;
}

void Connection::setDescription( ConnectionDescriptionPtr description )
{
    EQASSERT( description.isValid( ));
    EQASSERTINFO( _description->type == description->type,
                  "Wrong connection type in description" );
    _description = description;
    EQASSERT( description->bandwidth > 0 );
}

std::ostream& operator << ( std::ostream& os, const Connection& connection )
{
    Connection::State        state = connection.getState();
    ConnectionDescriptionPtr desc  = connection.getDescription();

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
