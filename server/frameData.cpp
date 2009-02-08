
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frameData.h"

#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

namespace eq
{
namespace server
{

FrameData::FrameData()
{
    _data.buffers = eq::Frame::BUFFER_UNDEFINED;
    _data.frameType = eq::Frame::TYPE_MEMORY;
}

void FrameData::getInstanceData( net::DataOStream& os )
{
    os << _data;
}

void FrameData::applyInstanceData( net::DataIStream& is )
{
    EQUNREACHABLE;
    is >> _data;
}


}
}
