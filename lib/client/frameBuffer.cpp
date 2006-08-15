
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frameBuffer.h"

#include "commands.h"
#include "image.h"
#include "object.h"

#include <eq/net/session.h>
#include <algorithm>

using namespace eq;
using namespace std;

FrameBuffer::FrameBuffer( const void* data, const uint64_t size )
        : Object( eq::Object::TYPE_FRAMEBUFFER, CMD_FRAMEBUFFER_CUSTOM ),
          _data( *(Data*)data ) 
{
    EQASSERT( size == sizeof( Data ));
    setDistributedData( &_data, sizeof( Data ));
}

FrameBuffer::~FrameBuffer()
{
    flush();
}

void FrameBuffer::clear()
{
    for( uint32_t i=0; i<INDEX_ALL; ++i )
        _clearImages( (FormatIndex)i );
}

void FrameBuffer::_clearImages( const FormatIndex i )
{
    copy( _images[i].begin(), _images[i].end(), _imageCache[i].end() );
    _images[i].clear();
}

void FrameBuffer::flush()
{
    for( uint32_t i=0; i<INDEX_ALL; ++i )
        _flushImages( (FormatIndex)i );
}

void FrameBuffer::_flushImages( const FormatIndex i )
{
    _clearImages( i );

    for( vector<Image*>::const_iterator iter = _imageCache[i].begin();
         iter != _imageCache[i].end(); ++iter )

        delete *iter;

    _imageCache[i].clear();
}


void FrameBuffer::startReadback()
{
}


