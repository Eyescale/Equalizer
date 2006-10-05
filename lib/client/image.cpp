
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "image.h"

#include <eq/client/windowSystem.h>

#include <fstream>

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
    _pvp = pvp;

    // XXX remove me
    static size_t counter = 0;
    ostringstream  filename;
    filename << "eqImage" << ++counter << ".rgb";
    writeImage( filename.str( ));
}

void Image::writeImage( const std::string& filename )
{
    ofstream image( filename.c_str(), ios::out | ios::binary );
    if( !image.is_open( ))
    {
        EQERROR << "Can't open " << filename << " for writing" << endl;
        return;
    }

    image << this;
    image.close();
}

std::ostream& eq::operator << ( std::ostream& os, const Image* image )
{
    const size_t nPixels = image->_pvp.w * image->_pvp.h;
    if( image->_pixels.size() < nPixels )
        return os;

    const size_t   nChannels = image->getDepth();
    unsigned short shortVar  = 474;
    const char*    shortVarP = (const char *)&shortVar;
    os.write( shortVarP, 2 ); // magic number
    os << (char)0;            // not RLE-compressed
    os << (char)1;            // bytes per pixel channel
    shortVar = 3;             os.write( shortVarP, 2 ); // number of dimensions
    shortVar = image->_pvp.w; os.write( shortVarP, 2 ); // x
    shortVar = image->_pvp.h; os.write( shortVarP, 2 ); // y
    shortVar = nChannels;     os.write( shortVarP, 2 ); // z
    os << (long)0;           // min pixel value;
    os << (long)255;         // max pixel value;
    os << (unsigned)0;       // dummy
    const char filename[80] = 
        "Created from Equalizer image data (http://www.equalizergraphics.com)";
    os.write( filename, 80 );
    os << (unsigned)0;       // color map: 0 eq normal mode
    const char fill[404] = "";
    os.write( fill, 404 );

    // Each channel is written separately
    const size_t nBytes = nPixels * nChannels;
    for( size_t i = 0; i < nChannels; ++i )
    {
        for( size_t j = i; j < nBytes; j += nChannels )
            os << image->_pixels[j];
    }

    return os;
}
