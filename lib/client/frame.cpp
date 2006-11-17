/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frame.h"

#include "frameBuffer.h"
#include "object.h"
#include "packets.h"

#include <eq/net/session.h>

using namespace eq;

Frame::Frame( const void* data, uint64_t dataSize )
        : eqNet::Object( eq::Object::TYPE_FRAME )
{
    EQASSERT( dataSize == sizeof( Data ));
    _data = *(Data*)data;

    setInstanceData( &_data, sizeof( Data ));
}

FrameBuffer* Frame::_getBuffer()
{
    EQASSERT( _data.buffer.objectID != EQ_ID_INVALID );
    if( !_buffer )
    {
        eqNet::Session* session = getSession();
        eqNet::Object*  object  = session->getObject( _data.buffer.objectID,
                                                      Object::SHARE_NODE,
                                                      _data.buffer.version );
        EQASSERT( dynamic_cast<FrameBuffer*>( object ) );
        _buffer = static_cast<FrameBuffer*>( object );
    }
    return _buffer;
}

void Frame::clear()
{
    _getBuffer()->clear(); 
}

void Frame::startReadback() 
{
    _getBuffer()->startReadback( *this );
}

void Frame::syncReadback() 
{
    _getBuffer()->syncReadback();
}

void Frame::transmit( eqBase::RefPtr<eqNet::Node> toNode )
{
    _getBuffer()->transmit( toNode );
}
