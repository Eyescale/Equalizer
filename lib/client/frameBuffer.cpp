
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frameBuffer.h"

#include "commands.h"
#include "object.h"

#include <eq/net/session.h>

using namespace eq;

FrameBuffer::FrameBuffer( eqBase::RefPtr<eqNet::Node> master )
        : Object( eq::Object::TYPE_FRAMEBUFFER, CMD_FRAMEBUFFER_CUSTOM ),
          _master( master )
{
    _data.master = master->getNodeID();    
}

FrameBuffer::FrameBuffer( const void* instanceData )
        : Object( eq::Object::TYPE_FRAMEBUFFER, CMD_FRAMEBUFFER_CUSTOM ),
          _data( *(Data*)instanceData ) 
{
}

void FrameBuffer::init( const void* data, const uint64_t dataSize )
{
    _master = eqNet::Node::getLocalNode()->connect( _data.master, 
                                                    getSession()->getServer( ));
}


