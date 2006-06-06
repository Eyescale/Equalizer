
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_FRAMEDATA_H
#define EQ_PLY_FRAMEDATA_H

#include "eqPly.h"

#include <eq/eq.h>

class FrameData : public eqNet::Object
{
public:
    FrameData() : Object( TYPE_FRAMEDATA, eqNet::CMD_OBJECT_CUSTOM ),
                  spin(0.) {}

    FrameData( const void* data, const uint64_t size ) 
            : Object( TYPE_FRAMEDATA, eqNet::CMD_OBJECT_CUSTOM )
        {
            EQASSERT( size == sizeof( spin ));
            spin = *(float*)data;
            EQINFO << "New FrameData instance" << std::endl;
        }

    float spin;

protected:
    const void* getInstanceData( uint64_t* size )
        { return pack( size ); }

    const void* pack( uint64_t* size )
        {
            *size = sizeof( spin );
            return &spin;
        }

    void unpack( const void* data, const uint64_t size )
        {
            EQASSERT( size == sizeof( spin ));
            spin = *(float*)data;
        }
};



#endif // EQ_PLY_FRAMEDATA_H

