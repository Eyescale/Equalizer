
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frame.h"

#include "frameBuffer.h"

#include <eq/client/packets.h>
#include <eq/net/session.h>

using namespace eqs;
using namespace eqNet;

Frame::Frame()
        : eqNet::Object( eq::DATATYPE_EQ_FRAME, eqNet::CMD_OBJECT_CUSTOM ),
          _buffer( NULL )
{
    setDistributedData( &_inherit, sizeof( eq::Frame::Data ));
}

Frame::~Frame()
{
    EQASSERT( _buffers.empty());
}

void Frame::flush()
{
    Session* session = getSession();
    EQASSERT( session );
    while( !_buffers.empty( ))
    {
        FrameBuffer* buffer = _buffers.front();
        session->deregisterObject( buffer );
        _buffers.pop_front();
    }
    _buffer = NULL;
}

void Frame::updateInheritData( const Compound* compound )
{
}

void Frame::cycleFrameBuffer( const uint32_t frameNumber, const uint32_t maxAge)
{
    // find unused frame buffer
    FrameBuffer* buffer = _buffers.back();
    
    if( buffer->getFrameNumber() < frameNumber-maxAge ) // not used anymore
        _buffers.pop_back();
    else
    {
        buffer = new FrameBuffer;
        
        Session* session = getSession();
        EQASSERT( session );

        session->registerObject( buffer, session->getLocalNode( ));
    }

    buffer->setFrameNumber( frameNumber );
    buffer->commit();
    
    _buffers.push_front( buffer );
    _setFrameBuffer( buffer );
}

void Frame::setOutputFrame( Frame* frame )
{
    EQASSERT( frame->_buffer );
    _setFrameBuffer( frame->_buffer );
}

void Frame::_setFrameBuffer( FrameBuffer* buffer )
{
    _buffer = buffer;
    if( !buffer )
        return;

    _inherit.bufferID      = buffer->getID();
    _inherit.bufferVersion = buffer->getVersion();
}
