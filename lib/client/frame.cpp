/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "frame.h"

#include "object.h"
#include "packets.h"

using namespace eq;

Frame::Frame( const void* data, uint64_t dataSize )
        : eqNet::Object( eq::Object::TYPE_FRAME, eqNet::CMD_OBJECT_CUSTOM )
{
    EQASSERT( dataSize == sizeof( Data ));
    _data = *(Data*)data;

    setDistributedData( &_data, sizeof( Data ));
}
