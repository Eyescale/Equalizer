/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frame.h"

#include "frameData.h"
#include "packets.h"

#include <eq/net/session.h>

using namespace eq;
using namespace std;

Frame::Frame( Pipe* pipe )
        : _pipe( pipe )
{
    setInstanceData( &_data, sizeof( Data ));
}

FrameData* Frame::_getData()
{
    EQASSERT( _data.frameData.objectID != EQ_ID_INVALID );
    return _pipe->getNode()->getFrameData( _data.frameData.objectID,
                                           _data.frameData.version ); 
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

bool Frame::isReady()
{
    return _getData()->isReady();
}

void Frame::waitReady()
{
    _getData()->waitReady();
}


void Frame::addListener( eqBase::Monitor<uint32_t>& listener )
{
    _getData()->addListener( listener );
}

void Frame::removeListener( eqBase::Monitor<uint32_t>& listener )
{
    _getData()->removeListener( listener );
}
