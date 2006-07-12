
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_FRAMEDATA_H
#define EQ_PLY_FRAMEDATA_H

#include "eqPly.h"

#include <eq/eq.h>

class FrameData : public eqNet::Object
{
public:
    FrameData() : Object( TYPE_FRAMEDATA, eqNet::CMD_OBJECT_CUSTOM )
        {
            reset();
        }

    FrameData( const void* data, const uint64_t size ) 
            : Object( TYPE_FRAMEDATA, eqNet::CMD_OBJECT_CUSTOM )
        {
            EQASSERT( size == sizeof( Data ));
            _data = *(Data*)data;
            EQINFO << "New FrameData instance" << std::endl;
        }
    
    void reset()
        {
            bzero( &_data, sizeof( Data ));
            _data.translation[2] = -3.;
        }

    struct Data
    {
        float rotation[2];
        float translation[3];
    } _data;

protected:
    const void* getInstanceData( uint64_t* size )
        { return pack( size ); }

    const void* pack( uint64_t* size )
        {
            *size = sizeof( Data );
            return &_data;
        }

    void unpack( const void* data, const uint64_t size )
        {
            EQASSERT( size == sizeof( Data ));
            _data = *(Data*)data;
        }
};



#endif // EQ_PLY_FRAMEDATA_H

