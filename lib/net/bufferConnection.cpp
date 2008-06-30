
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "bufferConnection.h"

using namespace std;

namespace eqNet
{
BufferConnection::BufferConnection()
        : _buffer(0),
          _size(0),
          _maxSize(0)
{
    _state = STATE_CONNECTED;
    EQINFO << "New Buffer Connection @" << (void*)this << endl;
}

BufferConnection::~BufferConnection()
{
    if( _size )
        EQWARN << "Deleting BufferConnection with buffered data" << endl;

    if( _buffer )
        free( _buffer );

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

void BufferConnection::sendBuffer( ConnectionPtr connection )
{
    if( _size == 0 )
        return;

    if( !connection )
    {
        EQWARN << "NULL connection during buffer write" << endl;
        return;
    }

    const bool sent = connection->send( _buffer, _size );
    EQASSERT( sent );
    _size = 0;
}
}
