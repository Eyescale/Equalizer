/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frame.h"

#include "frameData.h"
#include "packets.h"

#include <eq/net/session.h>

using namespace std;

namespace eq
{

Frame::Frame( Pipe* pipe )
        : _pipe( pipe ),
          _eyePass( EYE_CYCLOP )
{
    setInstanceData( &_data, sizeof( Data ));
}


FrameData* Frame::_getData() const
{
    const eqNet::ObjectVersion& frameData = _data.frameData[ _eyePass ];
    EQASSERT( frameData.objectID != EQ_ID_INVALID );

    return _pipe->getNode()->getFrameData( frameData.objectID,
                                           frameData.version ); 
}

uint32_t Frame::getBuffers() const
{
    return _getData()->getBuffers();
}

const Pixel& Frame::getPixel() const
{
    return _getData()->getPixel();
}

const Range& Frame::getRange() const
{
    return _getData()->getRange();
}

const std::vector<Image*>& Frame::getImages() const
{
    return _getData()->getImages();
}

void Frame::startReadback() 
{
    _getData()->startReadback( *this );
}

void Frame::syncReadback() 
{
    _getData()->syncReadback();
}

void Frame::transmit( eqBase::RefPtr<eqNet::Node> toNode )
{
    _getData()->transmit( toNode );
}

bool Frame::isReady() const
{
    return _getData()->isReady();
}

void Frame::waitReady() const
{
    _getData()->waitReady();
}

void Frame::disableBuffer( const Buffer buffer )
{
    _getData()->disableBuffer( buffer );
}


void Frame::addListener( eqBase::Monitor<uint32_t>& listener )
{
    _getData()->addListener( listener );
}

void Frame::removeListener( eqBase::Monitor<uint32_t>& listener )
{
    _getData()->removeListener( listener );
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
