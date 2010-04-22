
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include "packets.h"

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>
#include <eq/net/session.h>

using namespace std;

namespace eq
{

Frame::Frame()
        : _frameData( 0 )
{
    EQINFO << "New Frame @" << (void*)this << endl;
}

Frame::~Frame()
{
    if( _frameData )
        EQINFO << "FrameData attached to frame during deletion" << endl;
}

void Frame::getInstanceData( net::DataOStream& os )
{
    EQUNREACHABLE;
    os << _data;
}

void Frame::applyInstanceData( net::DataIStream& is )
{
    is >> _data;
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

void Frame::setRange( const Range& range )
{
    EQASSERT( _frameData );
    _frameData->setRange( range );    
}

void Frame::setInputZoom( const Zoom& zoom )
{
    EQASSERT( _frameData );
    EQASSERT( zoom.isValid( ));
    _frameData->setZoom( zoom );
}

const Zoom& Frame::getInputZoom() const
{
    EQASSERT( _frameData );
    return _frameData->getZoom();
}

const ImageVector& Frame::getImages() const
{
    EQASSERT( _frameData );
    return _frameData->getImages();
}

void Frame::setPixelViewport( const PixelViewport& pvp )
{
    EQASSERT( _frameData );
    _frameData->setPixelViewport( pvp );
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

void Frame::readback( util::ObjectManager< const void* >* glObjects,
                      const DrawableConfig& config ) 
{
    EQASSERT( _frameData );
    _frameData->readback( *this, glObjects, config );
}

void Frame::transmit( net::NodePtr toNode, const uint32_t frameNumber,
                      const uint32_t originator )
{
    EQASSERT( _frameData );
    _frameData->transmit( toNode, frameNumber, originator );
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

void Frame::waitReady() const
{
    EQASSERT( _frameData );
    _frameData->waitReady();
}

void Frame::disableBuffer( const Buffer buffer )
{
    EQASSERT( _frameData );
    _frameData->disableBuffer( buffer );
}

void Frame::useSendToken( const bool use )
{
    EQASSERT( _frameData );
    _frameData->useSendToken( use );
}

void Frame::addListener( base::Monitor<uint32_t>& listener )
{
    EQASSERT( _frameData );
    _frameData->addListener( listener );
}

void Frame::removeListener( base::Monitor<uint32_t>& listener )
{
    EQASSERT( _frameData );
    _frameData->removeListener( listener );
}

EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                      const Frame::Type type )
{
    os << "type     ";
    if ( type == eq::Frame::TYPE_TEXTURE ) 
        os << " texture" << endl;
    else if ( type == eq::Frame::TYPE_MEMORY ) 
        os << " memory" << endl;
        
    return os;
}

EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
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
