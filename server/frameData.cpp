
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frameData.h"

using namespace eqs;

FrameData::FrameData()
{
    _data.buffers = eq::Frame::BUFFER_UNDEFINED;
    setInstanceData( &_data, sizeof( eq::FrameData::Data ));
}
