
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

#include <co/dataIStream.h>
#include <co/dataOStream.h>

using namespace std;

namespace eq
{

Frame::Frame()
        : _frameData( 0 )
        , _zoomFilter( FILTER_LINEAR )
{
    EQINFO << "New Frame @" << (void*)this << endl;
}

Frame::~Frame()
{
    if( _frameData )
        EQINFO << "FrameData attached to frame during deletion" << endl;
}

void Frame::getInstanceData( co::DataOStream& os )
{
    EQUNREACHABLE;
    os << _data;
}

void Frame::applyInstanceData( co::DataIStream& is )
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

void Frame::readback( util::ObjectManager< const void* >* glObjects,
                      const DrawableConfig& config ) 
{
    EQASSERT( _frameData );
    _frameData->readback( *this, glObjects, config );
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
        os << " texture" << endl;
    else if ( type == eq::Frame::TYPE_MEMORY ) 
        os << " memory" << endl;
        
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
