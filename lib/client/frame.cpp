/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frame.h"

#include "frameData.h"
#include "object.h"
#include "packets.h"

#include <eq/net/session.h>

using namespace eq;
using namespace std;

Frame::Frame( const void* data, uint64_t dataSize )
        : _frameData( 0 )
{
    EQASSERT( dataSize == sizeof( Data ));
    _data = *(Data*)data;

    setInstanceData( &_data, sizeof( Data ));
}

FrameData* Frame::_getData()
{
    EQASSERT( _data.frameData.objectID != EQ_ID_INVALID );
    if( !_frameData )
    {
        eqNet::Session* session = getSession();
        eqNet::Object*  object  = session->getObject( _data.frameData.objectID,
                                                      Object::SHARE_NODE,
                                                      _data.frameData.version );
        EQASSERT( dynamic_cast<FrameData*>( object ) );
        _frameData = static_cast<FrameData*>( object );
    }
    return _frameData;
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
