
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

#ifdef WIN32
#  include "namedPipeConnection.h"
#endif
#ifdef EQ_INFINIBAND
#  include "IBConnection.h"
#endif
#ifdef EQ_PGM
#  include "PGMConnection.h"
#endif

#include <errno.h>

using namespace eq::base;
using namespace std;

namespace eq
{
namespace net
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
            
#ifdef WIN32
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
        case CONNECTIONTYPE_RSP:
            connection = new RSPConnection;
            break;
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

bool Connection::recvSync( void** outBuffer, uint64_t* outBytes )
{
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

    uint8_t* ptr = static_cast< uint8_t* >( buffer );
    uint64_t bytesLeft = bytes;

    while( bytesLeft )
    {
        const int64_t got = readSync( ptr, bytesLeft );

        if( got < 0 ) // error
        {
            if( outBytes )
                *outBytes -= bytesLeft;
            if( bytes == bytesLeft )
                EQINFO << "Read on dead connection" << endl;
            else
                EQERROR << "Error during read after " << bytes - bytesLeft
                        << " bytes on " << typeid(*this).name() << endl;
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
        }
        else
        {
            EQASSERT( static_cast< uint64_t >( got ) == bytesLeft );
            bytesLeft = 0;
        }
    }

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

    // possible OPT: We need to lock here to guarantee an atomic transmission of
    // the buffer. Possible improvements are:
    // 1) Disassemble buffer into 'small enough' pieces and use a header to
    //    reassemble correctly on the other side (aka reliable UDP)
    // 2) Introduce a send thread with a thread-safe task queue
    ScopedMutex<> mutex( isLocked ? 0 : &_sendLock );

    const uint8_t* ptr = static_cast< const uint8_t* >( buffer );
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
    if( size > ASSEMBLE_THRESHOLD )
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

    char*          buffer = (char*)alloca( size );

    memcpy( buffer,              &packet, headerSize );
    memcpy( buffer + headerSize, data,    dataSize );

    ((Packet*)buffer)->size = size;
    return send( buffer, size );
}

bool Connection::send( const ConnectionVector& connections,
                       const Packet& packet, const bool isLocked )
{
    if( connections.empty( ))
        return true;

    for( ConnectionVector::const_iterator i= connections.begin(); 
         i<connections.end(); ++i )
    {        
        ConnectionPtr connection = *i;
        if( !connection->send( &packet, packet.size, isLocked ))
            return false;
    }
    return true;
}

bool Connection::send( const ConnectionVector& connections, Packet& packet,
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

    if( size > ASSEMBLE_THRESHOLD )
    {
        // OPT: lock the connection and use two send() to avoid big memcpy
        packet.size = size;

        for( ConnectionVector::const_iterator i= connections.begin(); 
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

    for( ConnectionVector::const_iterator i= connections.begin(); 
         i<connections.end(); ++i )
    {        
        ConnectionPtr connection = *i;
        if( !connection->send( buffer, size, isLocked ))
            return false;
    }

    return true;
}

bool Connection::send( const ConnectionVector& connections, Packet& packet,
                       const void* const* items, const uint64_t* itemSizes, 
                       const size_t nItems )
{
    if( connections.empty( ))
        return true;

    const uint64_t headerSize  = packet.size - 8;
    packet.size = headerSize;
    for( size_t i = 0; i < nItems; ++i )
        packet.size += itemSizes[ i ] + sizeof( uint64_t );

    for( ConnectionVector::const_iterator i = connections.begin(); 
         i < connections.end(); ++i )
    {        
        ConnectionPtr connection = *i;
        connection->lockSend();
            
        bool ok = connection->send( &packet, headerSize, true );

        for( size_t j = 0; j < nItems; ++j )
            ok = ok && connection->send( &itemSizes[j], sizeof(uint64_t), true )
                    && connection->send( items[j], itemSizes[j], true );

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

std::ostream& operator << ( std::ostream& os, const Connection* connection )
{
    if( !connection )
    {
        os << "NULL connection";
        return os;
    }
    
    Connection::State        state = connection->getState();
    ConnectionDescriptionPtr desc  = connection->getDescription();

    os << "Connection " << (void*)connection << " type "
       << typeid(*connection).name() << " state "
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
}
