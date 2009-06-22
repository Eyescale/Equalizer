
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

const Range& Frame::getRange() const
{
    EQASSERT( _frameData );
    return _frameData->getRange();
}

void Frame::setRange( const Range& range )
{
    EQASSERT( _frameData );
    _frameData->setRange( range );    
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

void Frame::setColorType( const GLuint colorType )
{
    if( _frameData )
        _frameData->setColorType( colorType );
}

void Frame::startReadback( Window::ObjectManager* glObjects ) 
{
    EQASSERT( _frameData );
    _frameData->startReadback( *this, glObjects );
}

void Frame::syncReadback() 
{
    EQASSERT( _frameData );
    _frameData->syncReadback();
}

void Frame::transmit( net::NodePtr toNode, Event& event )
{
    EQASSERT( _frameData );
    _frameData->transmit( toNode, event );
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
