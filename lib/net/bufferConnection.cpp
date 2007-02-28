
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "bufferConnection.h"

using namespace eqNet;
using namespace std;

BufferConnection::BufferConnection()
        : _buffer(0),
          _size(0),
          _maxSize(0)
{}

BufferConnection::~BufferConnection()
{
    if( _buffer )
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
    if( !_size )
        return;

    const bool sent = connection->send( _buffer, _size );
    EQASSERT( sent );
    _size = 0;
}
