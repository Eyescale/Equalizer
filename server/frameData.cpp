
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frameData.h"


namespace eq
{
namespace server
{

FrameData::FrameData()
{
    _data.buffers = eq::Frame::BUFFER_UNDEFINED;
    setInstanceData( &_data, sizeof( eq::FrameData::Data ));
}

}
}
