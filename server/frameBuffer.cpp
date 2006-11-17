
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frameBuffer.h"

#include <eq/client/object.h>

using namespace eqs;

FrameBuffer::FrameBuffer()
        : eqNet::Object( eq::Object::TYPE_FRAMEBUFFER )
{
    _data.format = eq::Frame::FORMAT_UNDEFINED;
    setInstanceData( &_data, sizeof( eq::FrameBuffer::Data ));
}
