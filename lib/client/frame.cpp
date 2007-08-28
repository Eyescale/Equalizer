/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frame.h"

#include "frameData.h"
#include "packets.h"

#include <eq/net/session.h>

using namespace eq;
using namespace std;

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

void Frame::startAssemble() 
{
    _getData()->startAssemble( *this );
}

void Frame::syncAssemble() 
{
    _getData()->syncAssemble();
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
