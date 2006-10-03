
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

FrameBuffer::FormatIndex FrameBuffer::_getIndexForFormat( const Frame::Format
                                                          format )
{
    switch( format )
    {
        case Frame::FORMAT_COLOR: return INDEX_COLOR;
        case Frame::FORMAT_DEPTH: return INDEX_DEPTH;

        default: EQUNIMPLEMENTED;
    }
    return static_cast<FormatIndex>(-1);
}

void FrameBuffer::clear()
{
    for( uint32_t i=0; i<INDEX_ALL; ++i )
        _clearImages( (FormatIndex)i );
}

void FrameBuffer::_clearImages( const FormatIndex i )
{
    copy( _images[i].begin(), _images[i].end(), 
          inserter( _imageCache[i], _imageCache[i].end( )));
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

Image* FrameBuffer::newImage( const Frame::Format format )
{
    const FormatIndex index = _getIndexForFormat( format );
    Image* image;
    if( _imageCache[index].empty( ))
        image = new Image( format );
    else
    {
        image = _imageCache[index].back();
        _imageCache[index].pop_back();
    }
    _images[index].push_back( image );
    return image;
}

void FrameBuffer::startReadback( const Frame::Format frameFormat )
{
    const Frame::Format format = (_data.format == Frame::FORMAT_UNDEFINED) ?
        frameFormat : static_cast<Frame::Format>(_data.format & frameFormat);

    if( format & Frame::FORMAT_COLOR )
    {
        Image* image = newImage( Frame::FORMAT_COLOR );
        image->startReadback( _data.pvp );
    }
}

