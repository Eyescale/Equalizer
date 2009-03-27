
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#include "bufferConnection.h"

#include <string.h>

using namespace std;

namespace eq
{
namespace net
{
BufferConnection::BufferConnection()
        : _buffer(0),
          _size(0),
          _maxSize(0)
{
    _state = STATE_CONNECTED;
    EQVERB << "New BufferConnection @" << (void*)this << endl;
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

    EQCHECK( connection->send( _buffer, _size ));
    _size = 0;
}
}
}
