
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frameData.h"

#include <eq/client/object.h>

using namespace eqs;

FrameData::FrameData()
        : eqNet::Object( eq::Object::TYPE_FRAMEDATA )
{
    _data.buffers = eq::Frame::BUFFER_UNDEFINED;
    setInstanceData( &_data, sizeof( eq::FrameData::Data ));
}
