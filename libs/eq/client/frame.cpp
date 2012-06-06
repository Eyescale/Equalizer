
/* Copyright (c) 2006-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "frame.h"

#include "frameData.h"
#include "image.h"
#include <eq/util/objectManager.h>

namespace eq
{
namespace detail
{
class Frame
{
public:
    FrameData* frameData; // Remove me in 2.0
    FrameDataPtr frameDataPtr;

    ZoomFilter zoomFilter; // texture filter

    Frame() : frameData( 0 ), zoomFilter( FILTER_LINEAR ) {}
    ~Frame()
    {
        if( frameData )
            LBINFO << "FrameData attached in frame destructor" << std::endl;
    }
};
}

Frame::Frame()
        : _impl( new detail::Frame )
{
}

Frame::~Frame()
{
    delete _impl;
}

void Frame::setZoomFilter( const ZoomFilter zoomFilter )
{
    _impl->zoomFilter = zoomFilter;
}

ZoomFilter Frame::getZoomFilter() const
{
    return _impl->zoomFilter;
}

#ifndef EQ_2_0_API
void Frame::setData( FrameData* data )
{
    LBASSERTINFO( !_impl->frameDataPtr, "Don't mix deprecated with new API" );
    _impl->frameData = data;
}

FrameData* Frame::getData()
{
    return _impl->frameData;
}

const FrameData* Frame::getData() const
{
    return _impl->frameData;
}
#endif

void Frame::setFrameData( FrameDataPtr data )
{
    _impl->frameDataPtr = data;
    _impl->frameData = data.get();
}

FrameDataPtr Frame::getFrameData()
{
    return _impl->frameDataPtr;
}

const FrameDataPtr Frame::getFrameData() const
{
    return _impl->frameDataPtr;
}

uint32_t Frame::getBuffers() const
{
    LBASSERT( _impl->frameData );
    return _impl->frameData->getBuffers();
}

const Pixel& Frame::getPixel() const
{
    LBASSERT( _impl->frameData );
    return _impl->frameData->getPixel();
}

const SubPixel& Frame::getSubPixel() const
{
    LBASSERT( _impl->frameData );
    return _impl->frameData->getSubPixel();
}

const Range& Frame::getRange() const
{
    LBASSERT( _impl->frameData );
    return _impl->frameData->getRange();
}

uint32_t Frame::getPeriod() const
{
    LBASSERT( _impl->frameData );
    return _impl->frameData->getPeriod();
}

uint32_t Frame::getPhase() const
{
    LBASSERT( _impl->frameData );
    return _impl->frameData->getPhase();
}

const Images& Frame::getImages() const
{
    LBASSERT( _impl->frameData );
    return _impl->frameData->getImages();
}

void Frame::clear()
{
    LBASSERT( _impl->frameData );
    _impl->frameData->clear();
}

void Frame::deleteGLObjects( ObjectManager* om )
{
    if( _impl->frameData )
        _impl->frameData->deleteGLObjects( om );
}

void Frame::setAlphaUsage( const bool useAlpha )
{
    if( _impl->frameData )
        _impl->frameData->setAlphaUsage( useAlpha );
}

void Frame::setQuality( const Frame::Buffer buffer, const float quality )
{
    if( _impl->frameData )
        _impl->frameData->setQuality( buffer, quality );
}

void Frame::useCompressor( const Frame::Buffer buffer, const uint32_t name )
{
    if( _impl->frameData )
        _impl->frameData->useCompressor( buffer, name );
}

void Frame::readback( ObjectManager* glObjects, const DrawableConfig& config )
{
    LBASSERT( _impl->frameData );
    const PixelViewport& pvp = _impl->frameData->getPixelViewport();
    const Images& images = _impl->frameData->startReadback( *this, glObjects, config,
                                                      PixelViewports( 1, pvp ));
    for( ImagesCIter i = images.begin(); i != images.end(); ++i )
        (*i)->finishReadback( getZoom(), glObjects->glewGetContext( ));
}

void Frame::readback( ObjectManager* glObjects, const DrawableConfig& config,
                      const PixelViewports& regions )
{
    LBASSERT( _impl->frameData );
    const Images& images = _impl->frameData->startReadback( *this, glObjects, config,
                                                      regions );
    for( ImagesCIter i = images.begin(); i != images.end(); ++i )
        (*i)->finishReadback( getZoom(), glObjects->glewGetContext( ));
}

Images Frame::startReadback( ObjectManager* glObjects,
                           const DrawableConfig& config,
                           const PixelViewports& regions )
{
    LBASSERT( _impl->frameData );
    return _impl->frameData->startReadback(  *this, glObjects, config, regions );
}

void Frame::setReady()
{
    LBASSERT( _impl->frameData );
    _impl->frameData->setReady();
}

bool Frame::isReady() const
{
    LBASSERT( _impl->frameData );
    return _impl->frameData->isReady();
}

void Frame::waitReady( const uint32_t timeout ) const
{
    LBASSERT( _impl->frameData );
    _impl->frameData->waitReady( timeout );
}

void Frame::disableBuffer( const Buffer buffer )
{
    LBASSERT( _impl->frameData );
    _impl->frameData->disableBuffer( buffer );
}

void Frame::addListener( lunchbox::Monitor<uint32_t>& listener )
{
    LBASSERT( _impl->frameData );
    _impl->frameData->addListener( listener );
}

void Frame::removeListener( lunchbox::Monitor<uint32_t>& listener )
{
    LBASSERT( _impl->frameData );
    _impl->frameData->removeListener( listener );
}

}
