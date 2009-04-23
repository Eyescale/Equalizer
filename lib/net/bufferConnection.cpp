
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "bufferConnection.h"

#include <string.h>

using namespace std;

namespace eq
{
namespace net
{
BufferConnection::BufferConnection()
{
    _state = STATE_CONNECTED;
    EQVERB << "New BufferConnection @" << (void*)this << endl;
}

BufferConnection::~BufferConnection()
{
    if( _buffer.size )
        EQWARN << "Deleting BufferConnection with buffered data" << endl;
}

int64_t BufferConnection::write( const void* buffer, const uint64_t bytes) const
{
    _buffer.append( reinterpret_cast< const uint8_t* >( buffer ), bytes );
    return bytes;
}

void BufferConnection::sendBuffer( ConnectionPtr connection )
{
    if( _buffer.size == 0 )
        return;

    if( !connection )
    {
        EQWARN << "NULL connection during buffer write" << endl;
        return;
    }

    EQCHECK( connection->send( _buffer.data, _buffer.size ));
    _buffer.size = 0;
}
}
}
