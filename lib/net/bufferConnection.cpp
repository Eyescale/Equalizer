
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "bufferConnection.h"

using namespace eqNet;
using namespace std;

BufferConnection::BufferConnection()
        : _buffer(0),
          _size(0),
          _maxSize(0)
{
    _state = STATE_CONNECTED;
}

BufferConnection::BufferConnection( const BufferConnection& from )
        : _buffer( from._buffer ),
          _size( from._size ),
          _maxSize( from._maxSize )
{
    // More a 'move' constructor. Used primarily when resizing STL containers
    _state   = STATE_CONNECTED;
    
    from._buffer  = 0;
    from._size    = 0;
    from._maxSize = 0;
}

BufferConnection::~BufferConnection()
{
    if( _size )
    {
        EQWARN << "Deleting BufferConnection with buffered data" << endl;
        free( _buffer );
    }

    _buffer  = 0;
    _size    = 0;
    _maxSize = 0;
}

int64_t BufferConnection::write( const void* buffer, const uint64_t bytes) const
{
    if( _maxSize < _size + bytes )
    {
        _maxSize += bytes; // TODO: more aggressive reallocation ?
        if( _buffer )
            _buffer = static_cast<uint8_t*>( realloc( _buffer, _maxSize ));
        else
            _buffer = static_cast<uint8_t*>( malloc( _maxSize ));
    }
    EQASSERT( _buffer );

    memcpy( _buffer + _size, buffer, bytes );
    _size += bytes;
    return bytes;
}

void BufferConnection::sendBuffer( eqBase::RefPtr<Connection> connection )
{
    if( _size == 0 )
        return;

    const bool sent = connection->send( _buffer, _size );
    EQASSERT( sent );
    _size = 0;
}

void BufferConnection::swap( BufferConnection& connection )
{
    uint8_t*    buffer = _buffer;
    _buffer            = connection._buffer;
    connection._buffer = buffer;

    uint64_t    size = _size;
    _size            = connection._size;
    connection._size = size;

    uint64_t    maxSize = _maxSize;
    _maxSize            = connection._maxSize;
    connection._maxSize = maxSize;
}
