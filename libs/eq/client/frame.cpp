
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
#include <co/dataIStream.h>
#include <co/dataOStream.h>

namespace eq
{

Frame::Frame()
        : _frameData( 0 )
        , _zoomFilter( FILTER_LINEAR )
{
    EQINFO << "New Frame @" << (void*)this << std::endl;
}

Frame::~Frame()
{
    if( _frameData )
        EQINFO << "FrameData attached to frame during deletion" << std::endl;
}

void Frame::getInstanceData( co::DataOStream& os )
{
    EQUNREACHABLE;
    _data.serialize( os );
}

void Frame::applyInstanceData( co::DataIStream& is )
{
    _data.deserialize( is );
}

void Frame::Data::serialize( co::DataOStream& os ) const
{
    os << offset << zoom;

    for( unsigned i = 0; i < NUM_EYES; ++i )
        os << frameDataVersion[i] << toNodes[i].inputNodes 
           << toNodes[i].inputNetNodes;
}

void Frame::Data::deserialize( co::DataIStream& is )
{
    is >> offset >> zoom;

    for( unsigned i = 0; i < NUM_EYES; ++i )
        is >> frameDataVersion[i] >> toNodes[i].inputNodes
           >> toNodes[i].inputNetNodes;
}

const std::string& Frame::getName() const
{
    return _name;
}

uint32_t Frame::getBuffers() const
{
    EQASSERT( _frameData );
    return _frameData->getBuffers();
}

const Pixel& Frame::getPixel() const
{
    EQASSERT( _frameData );
    return _frameData->getPixel();
}

const SubPixel& Frame::getSubPixel() const
{
    EQASSERT( _frameData );
    return _frameData->getSubPixel();
}

const Range& Frame::getRange() const
{
    EQASSERT( _frameData );
    return _frameData->getRange();
}

uint32_t Frame::getPeriod() const
{
    EQASSERT( _frameData );
    return _frameData->getPeriod();
}

uint32_t Frame::getPhase() const
{
    EQASSERT( _frameData );
    return _frameData->getPhase();
}

const Images& Frame::getImages() const
{
    EQASSERT( _frameData );
    return _frameData->getImages();
}

void Frame::clear()
{
    EQASSERT( _frameData );
    _frameData->clear();
}

void Frame::flush()
{
    if( _frameData )
        _frameData->flush();
}

void Frame::setAlphaUsage( const bool useAlpha )
{
    if( _frameData )
        _frameData->setAlphaUsage( useAlpha );
}

void Frame::setQuality( const Frame::Buffer buffer, const float quality )
{
    if( _frameData )
        _frameData->setQuality( buffer, quality );
}

void Frame::useCompressor( const Frame::Buffer buffer, const uint32_t name )
{
    if( _frameData )
        _frameData->useCompressor( buffer, name );
}

void Frame::readback( ObjectManager* glObjects, const DrawableConfig& config )
{
    EQASSERT( _frameData );
    const PixelViewport& pvp = _frameData->getPixelViewport();
    const Images& images = _frameData->startReadback( *this, glObjects, config,
                                                      PixelViewports( 1, pvp ));
    for( ImagesCIter i = images.begin(); i != images.end(); ++i )
        (*i)->finishReadback( getZoom(), glObjects->glewGetContext( ));
}

void Frame::readback( ObjectManager* glObjects,
                      const DrawableConfig& config,
                      const PixelViewports& regions )
{
    EQASSERT( _frameData );
    const Images& images = _frameData->startReadback( *this, glObjects, config,
                                                      regions );
    for( ImagesCIter i = images.begin(); i != images.end(); ++i )
        (*i)->finishReadback( getZoom(), glObjects->glewGetContext( ));
}

Images Frame::startReadback( ObjectManager* glObjects,
                           const DrawableConfig& config,
                           const PixelViewports& regions )
{
    EQASSERT( _frameData );
    return _frameData->startReadback(  *this, glObjects, config, regions );
}

void Frame::setReady()
{
    EQASSERT( _frameData );
    _frameData->setReady();
}

bool Frame::isReady() const
{
    EQASSERT( _frameData );
    return _frameData->isReady();
}

void Frame::waitReady( const uint32_t timeout ) const
{
    EQASSERT( _frameData );
    _frameData->waitReady( timeout );
}

void Frame::disableBuffer( const Buffer buffer )
{
    EQASSERT( _frameData );
    _frameData->disableBuffer( buffer );
}

void Frame::addListener( co::base::Monitor<uint32_t>& listener )
{
    EQASSERT( _frameData );
    _frameData->addListener( listener );
}

void Frame::removeListener( co::base::Monitor<uint32_t>& listener )
{
    EQASSERT( _frameData );
    _frameData->removeListener( listener );
}

std::ostream& operator << ( std::ostream& os, 
                                      const Frame::Type type )
{
    os << "type     ";
    if ( type == eq::Frame::TYPE_TEXTURE ) 
        os << " texture" << std::endl;
    else if ( type == eq::Frame::TYPE_MEMORY ) 
        os << " memory" << std::endl;
        
    return os;
}

std::ostream& operator << ( std::ostream& os, 
                                      const Frame::Buffer buffer )
{
    if( buffer == Frame::BUFFER_NONE )
        os << "none ";
    else if( buffer & Frame::BUFFER_UNDEFINED )
        os << "undefined ";
    else
    {
        if( buffer & Frame::BUFFER_COLOR )
            os << "color ";
        if( buffer & Frame::BUFFER_DEPTH )
            os << "depth ";
    }

    return os;
}

}
