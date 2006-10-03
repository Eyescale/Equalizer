
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "image.h"

#include <eq/client/windowSystem.h>

using namespace eq;
using namespace std;

Image::~Image()
{
    _pixels.clear();
}

size_t Image::getDepth() const
{
    // TODO: OpenGL format/type
    switch( _format )
    {
        case Frame::FORMAT_COLOR: return 4; // assume GL_RGBA/GL_UNSIGNED_BYTE
        case Frame::FORMAT_DEPTH: return 4; //assume GL_DEPTH_COMPONENT/GL_FLOAT
        default:
            EQUNIMPLEMENTED;
            return 0;
    }
}

uint32_t Image::getFormat() const
{
    switch( _format )
    {
        case Frame::FORMAT_COLOR: return GL_RGBA;
        case Frame::FORMAT_DEPTH: return GL_DEPTH_COMPONENT;
        default:
            EQUNIMPLEMENTED;
            return 0;
    }
}

uint32_t Image::getType() const
{
    switch( _format )
    {
        case Frame::FORMAT_COLOR: return GL_UNSIGNED_BYTE;
        case Frame::FORMAT_DEPTH: return GL_FLOAT;
        default:
            EQUNIMPLEMENTED;
            return 0;
    }
}

void Image::startReadback( const PixelViewport& pvp )
{
    EQINFO << "startReadback " << pvp << endl;

    const size_t size = pvp.w * pvp.h * getDepth();
    _pixels.resize( size );

    glReadPixels( pvp.x, pvp.y, pvp.w, pvp.h, getFormat(), getType(),
                  &_pixels[0] );
}

