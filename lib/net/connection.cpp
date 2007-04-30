
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "connection.h"

#include "connectionDescription.h"
#include "log.h"
#include "node.h"
#include "pipeConnection.h"
#include "socketConnection.h"

#include <errno.h>

#ifdef WIN32
#  define EQ_SOCKET_ERROR getErrorString( WSAGetLastError( ))
#  include <malloc.h>
#else
#  define EQ_SOCKET_ERROR strerror( errno )
#  include <alloca.h>
#endif

using namespace eqNet;
using namespace eqBase;
using namespace std;

Connection::Connection()
        : _state( STATE_CLOSED )
{
    EQINFO << "New Connection @" << (void*)this << endl;
}

Connection::Connection(const Connection& conn)
        : _state( conn._state ),
          _description( conn._description )
{
    EQINFO << "New Connection copy @" << (void*)this << endl;
}

Connection::~Connection()
{
    _state = STATE_CLOSED;
    EQINFO << "Delete Connection @" << (void*)this << endl;
}

RefPtr<Connection> Connection::create( const ConnectionType type )
{
    switch( type )
    {
        case CONNECTIONTYPE_TCPIP:
        case CONNECTIONTYPE_SDP:
            return new SocketConnection( type );

        case CONNECTIONTYPE_PIPE:
            return new PipeConnection();

        default:
            EQWARN << "Connection type not implemented" << endl;
            return 0;
    }
}

RefPtr<Connection> Connection::accept( const int timeout )
{
    if( _state != STATE_LISTENING )
        return 0;

    // prepare select 'set'
    const ReadNotifier notifier = getReadNotifier();
    
    if( notifier == 0 )
    {
        // Could implement the same using a setjmp() + alarm().
        EQWARN << "Cannot accept on connection, it does not use a file descriptor"
             << endl;
        return 0;
    }

#ifdef WIN32
    const DWORD waitTime = timeout > 0 ? timeout : INFINITE;
    const DWORD ret = WaitForSingleObject( notifier, waitTime );
#else
    const ReadNotifier fd = getReadNotifier();
    fd_set fdSet;
    FD_ZERO( &fdSet );
    FD_SET( fd, &fdSet );

    // wait for a connection
    timeval tv;
    tv.tv_sec  = timeout / 1000 ;
    tv.tv_usec = (timeout - tv.tv_sec*1000) * 1000;

    const int ret = ::select( fd+1, &fdSet, 0, 0, timeout ? &tv : 0 );
#endif
    switch( ret )
    {
        case SELECT_TIMEOUT:
            return 0;

        case SELECT_ERROR:
            EQWARN << "Error during select(): " << EQ_SOCKET_ERROR << endl;
            return 0;

        default: // SUCCESS
            return accept();
    }
}

//----------------------------------------------------------------------
// read
//----------------------------------------------------------------------
bool Connection::recv( void* buffer, const uint64_t bytes )
{
    EQLOG( LOG_WIRE ) << "Receiving " << bytes << " bytes on " << this << endl;
    if( _state != STATE_CONNECTED )
        return false;

    if( bytes > 1048576 )
        EQLOG( LOG_NETPERF ) << "Start receive " << bytes << " bytes" << endl;

    unsigned char* ptr       = static_cast<unsigned char*>(buffer);
    uint64_t       bytesLeft = bytes;

    while( bytesLeft )
    {
        int64_t got = this->read( ptr, bytesLeft );

        if( got == -1 ) // error
        {
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
            // Since we have nothing read yet, we have hit this case.
            if( bytesLeft == bytes )
                return false;

            EQVERB << "Zero bytes read" << endl;
        }

        if( bytes > 1048576 )
            EQLOG( LOG_NETPERF ) << "Got " << got << " bytes" << endl;

        bytesLeft -= got;
        ptr += got;
    }

    if( bytes > 1048576 )
        EQLOG( LOG_NETPERF ) << "End receive   " << bytes << " bytes" << endl;

    if( eqBase::Log::topics & LOG_WIRE ) // OPT
    {
        EQLOG( LOG_WIRE ) << disableFlush << "Received " << bytes << " bytes: ";
        const uint32_t printBytes = MIN( bytes, 256 );
        unsigned char* data       = static_cast<unsigned char*>(buffer);

        for( uint32_t i=0; i<printBytes; i++ )
        {
            if( i%4 )
                EQLOG( LOG_WIRE ) << " ";
            else if( i )
                EQLOG( LOG_WIRE ) << "|";

            EQLOG( LOG_WIRE ) << static_cast<int>(data[i]);
        }
        if( printBytes < bytes ) 
            EQLOG( LOG_WIRE ) << "|...";
        EQLOG( LOG_WIRE ) << endl << enableFlush;
    }
            
    return true;
}

//----------------------------------------------------------------------
// write
//----------------------------------------------------------------------
bool Connection::send( const void* buffer, const uint64_t bytes, 
                       bool isLocked ) const
{
    if( _state != STATE_CONNECTED )
        return false;

    // possible OPT: We need to lock here to guarantee an atomic transmission of
    // the buffer. Possible improvements are:
    // 1) Use a spinlock, since this lock should be relatively uncontended
    // 2) Disassemble buffer into 'small enough' pieces and use a header to
    //    reassemble correctly on the other side (aka reliable UDP)
    // 3) Introduce a send thread with a thread-safe task queue
    ScopedMutex mutex( isLocked ? 0 : &_sendLock );

    const unsigned char* ptr = static_cast<const unsigned char*>(buffer);

    if( eqBase::Log::topics & LOG_WIRE ) // OPT
    {
        EQLOG( LOG_WIRE ) << disableFlush << "Sending " << bytes 
                          << " bytes on " << (void*)this << ":";
        const uint32_t printBytes = MIN( bytes, 256 );

        for( uint32_t i=0; i<printBytes; ++i )
        {
            if( i%4 )
                EQLOG( LOG_WIRE ) << " ";
            else if( i )
                EQLOG( LOG_WIRE ) << "|";

            EQLOG( LOG_WIRE ) << static_cast<int>(ptr[i]);
        }
        if( printBytes < bytes ) 
            EQLOG( LOG_WIRE ) << "|...";
        EQLOG( LOG_WIRE ) << endl << enableFlush;
    }

    if( bytes > 1048576 )
        EQLOG( LOG_NETPERF ) << "Start transmit " << bytes << " bytes" << endl;

    uint64_t bytesLeft = bytes;
    while( bytesLeft )
    {
        const int64_t wrote = this->write( ptr, bytesLeft );
        if( bytes > 1048576 )
            EQLOG( LOG_NETPERF ) << "Wrote " << wrote << " bytes" << endl;

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

    if( bytes > 1048576 )
        EQLOG( LOG_NETPERF ) << "End transmit   " << bytes << " bytes" << endl;
    return true;
}

bool Connection::send( Packet& packet, const void* data, 
                       const uint64_t dataSize ) const
{
    if( dataSize == 0 )
        return send( packet );

    if( dataSize <= 8 ) // fits in existing packet
    {
        memcpy( (char*)(&packet) + packet.size-8, data, dataSize );
        return send( packet );
    }
    // else

    uint64_t       size   = packet.size-8 + dataSize;
    char*          buffer = (char*)alloca( size );

    memcpy( buffer, &packet, packet.size-8 );
    memcpy( buffer + packet.size-8, data, dataSize );

    ((Packet*)buffer)->size = size;
    return send( buffer, size );
}

eqBase::RefPtr<ConnectionDescription> Connection::getDescription()
{
    return _description;
}

void Connection::setDescription( eqBase::RefPtr<ConnectionDescription> 
                                 description )
{
    EQASSERT( description.isValid( ));
    EQASSERTINFO( _description->type == description->type,
                  "Wrong connection type in description" );
    _description = description;
}
