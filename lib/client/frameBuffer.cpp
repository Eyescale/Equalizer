
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frameBuffer.h"

#include "commands.h"
#include "object.h"

#include <eq/net/session.h>

using namespace eq;

FrameBuffer::FrameBuffer( const void* data, const uint64_t size )
        : Object( eq::Object::TYPE_FRAMEBUFFER, CMD_FRAMEBUFFER_CUSTOM ),
          _data( *(Data*)data ) 
{
    EQASSERT( size == sizeof( Data ));
    setDistributedData( &_data, sizeof( Data ));
}


void FrameBuffer::clear()
{
}

void FrameBuffer::startReadback()
{
}


