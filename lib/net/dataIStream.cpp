
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "dataIStream.h"

#include "log.h"
#include <eq/base/debug.h>

#include <string.h>

using namespace std;

namespace eq
{
namespace net
{
DataIStream::DataIStream()
        : _input( 0 )
        , _inputSize( 0 )
        , _position( 0 )
{}

DataIStream::~DataIStream()
{
    reset();
}

void DataIStream::reset()
{
    _input     = 0;
    _inputSize = 0;
    _position  = 0;
}

void DataIStream::read( void* data, uint64_t size )
{
    if( !_checkBuffer( ))
    {
        EQUNREACHABLE;
        EQERROR << "No more input data" << endl;
        return;
    }

    EQASSERT( _input );
    
    if( _position + size > _inputSize )
    {
        EQUNREACHABLE;
        EQERROR << "Not enough data in input buffer" << endl;
        return;
    }

    memcpy( data, _input + _position, size );
    _position += size;
}

const void* DataIStream::getRemainingBuffer()
{
    if( !_checkBuffer( ))
        return 0;

    return _input + _position;
}

uint64_t DataIStream::getRemainingBufferSize()
{
    if( !_checkBuffer( ))
        return 0;

    return _inputSize - _position;
}

void DataIStream::advanceBuffer( const uint64_t offset )
{
    EQASSERT( _position + offset <= _inputSize );
    _position += offset;
}

bool DataIStream::_checkBuffer()
{
    if( _position < _inputSize )
        return true;

    if( !getNextBuffer( &_input, &_inputSize ))
    {
        return false;
    }
    _position = 0;
    return true;
}

}
}

