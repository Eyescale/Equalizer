
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frameBuffer.h"

#include <eq/client/packets.h>

using namespace eqs;

FrameBuffer::FrameBuffer()
        : eqNet::Object( eq::DATATYPE_EQ_FRAMEBUFFER, eqNet::CMD_OBJECT_CUSTOM )
{
    //setDistributedData( &_data, sizeof( Data ));
}
