
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

namespace co
{
BufferConnection::BufferConnection()
{
    _state = STATE_CONNECTED;
    EQVERB << "New BufferConnection @" << (void*)this << endl;
}

BufferConnection::~BufferConnection()
{
    if( !_buffer.isEmpty( ))
        EQWARN << "Deleting BufferConnection with buffered data" << endl;
}

int64_t BufferConnection::write( const void* buffer, const uint64_t bytes )
{
    _buffer.append( reinterpret_cast< const uint8_t* >( buffer ), bytes );
    return bytes;
}

void BufferConnection::sendBuffer( ConnectionPtr connection )
{
    if( _buffer.isEmpty( ))
        return;

    if( !connection )
    {
        EQWARN << "NULL connection during buffer write" << endl;
        return;
    }

    EQCHECK( connection->send( _buffer.getData(), _buffer.getSize() ));
    _buffer.setSize( 0 );
}
}
