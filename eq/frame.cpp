
/* Copyright (c) 2006-2014, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Daniel Nachbaur <danielnachbaur@gmail.com>
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
    FrameDataPtr frameData;

    ZoomFilter zoomFilter; // texture filter

    Frame() : zoomFilter( FILTER_LINEAR ) {}
    ~Frame()
    {
        if( frameData )
            LBDEBUG << "FrameData attached in frame destructor" << std::endl;
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

void Frame::setFrameData( FrameDataPtr data )
{
    _impl->frameData = data;
}

FrameDataPtr Frame::getFrameData()
{
    return _impl->frameData;
}

ConstFrameDataPtr Frame::getFrameData() const
{
    return _impl->frameData;
}

uint32_t Frame::getBuffers() const
{
    return _impl->frameData->getBuffers();
}

const Pixel& Frame::getPixel() const
{
    return _impl->frameData->getPixel();
}

const SubPixel& Frame::getSubPixel() const
{
    return _impl->frameData->getSubPixel();
}

const Range& Frame::getRange() const
{
    return _impl->frameData->getRange();
}

uint32_t Frame::getPeriod() const
{
    return _impl->frameData->getPeriod();
}

uint32_t Frame::getPhase() const
{
    return _impl->frameData->getPhase();
}

const Images& Frame::getImages() const
{
    return _impl->frameData->getImages();
}

void Frame::clear()
{
    _impl->frameData->clear();
}

void Frame::deleteGLObjects( util::ObjectManager& om )
{
    if( _impl->frameData )
        _impl->frameData->deleteGLObjects( om );
}

void Frame::setAlphaUsage( const bool useAlpha )
{
    if( _impl->frameData )
        _impl->frameData->setAlphaUsage( useAlpha );
}

void Frame::setQuality( const Buffer buffer, const float quality )
{
    if( _impl->frameData )
        _impl->frameData->setQuality( buffer, quality );
}

void Frame::useCompressor( const Buffer buffer, const uint32_t name )
{
    if( _impl->frameData )
        _impl->frameData->useCompressor( buffer, name );
}

void Frame::readback( util::ObjectManager& glObjects, const DrawableConfig& config )
{
    const PixelViewport& pvp = _impl->frameData->getPixelViewport();
    const Images& images =
        _impl->frameData->startReadback( *this, glObjects, config,
                                         PixelViewports( 1, pvp ));
    for( ImagesCIter i = images.begin(); i != images.end(); ++i )
        (*i)->finishReadback( glObjects.glewGetContext( ));
}

void Frame::readback( util::ObjectManager& glObjects, const DrawableConfig& config,
                      const PixelViewports& regions )
{
    const Images& images =
        _impl->frameData->startReadback( *this, glObjects, config, regions );
    for( ImagesCIter i = images.begin(); i != images.end(); ++i )
        (*i)->finishReadback( glObjects.glewGetContext( ));
}

Images Frame::startReadback( util::ObjectManager& glObjects,
                             const DrawableConfig& config,
                             const PixelViewports& regions )
{
    return _impl->frameData->startReadback( *this, glObjects, config, regions );
}

void Frame::setReady()
{
    _impl->frameData->setReady();
}

bool Frame::isReady() const
{
    return _impl->frameData->isReady();
}

void Frame::waitReady( const uint32_t timeout ) const
{
    _impl->frameData->waitReady( timeout );
}

void Frame::disableBuffer( const Buffer buffer )
{
    _impl->frameData->disableBuffer( buffer );
}

void Frame::addListener( lunchbox::Monitor<uint32_t>& listener )
{
    _impl->frameData->addListener( listener );
}

void Frame::removeListener( lunchbox::Monitor<uint32_t>& listener )
{
    _impl->frameData->removeListener( listener );
}

}
