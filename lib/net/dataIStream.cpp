
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "dataIStream.h"

#include "log.h"
#include <eq/base/debug.h>

using namespace std;

namespace eqNet
{
DataIStream::DataIStream()
        : _input( 0 )
        , _inputSize( 0 )
        , _position( 0 )
{}

DataIStream::~DataIStream()
{}

void DataIStream::read( void* data, uint64_t size )
{
    if( _position >= _inputSize )
    {
        if( !getNextBuffer( reinterpret_cast< const void** >( &_input ),
                            &_inputSize ))
        {
            EQERROR << "No more input buffers" << endl;
            return;
        }
        _position = 0;
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
}

