
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
}

void FrameData::getInstanceData( net::DataOStream& os )
{
    os.writeOnce( &_data, sizeof( _data )); 
}

void FrameData::applyInstanceData( net::DataIStream& is )
{
    EQASSERT( is.getRemainingBufferSize() == sizeof( _data )); 

    memcpy( &_data, is.getRemainingBuffer(), sizeof( _data ));
    is.advanceBuffer( sizeof( _data ));

    EQASSERT( is.nRemainingBuffers() == 0 );
    EQASSERT( is.getRemainingBufferSize() == 0 );
}


}
}
