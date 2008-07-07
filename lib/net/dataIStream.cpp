
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "dataIStream.h"

#include "log.h"
#include <eq/base/debug.h>

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
        EQERROR << "No more input data" << endl;
        return;
    }

    EQASSERT( _input );
    
    if( _position + size > _inputSize )
    {
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

