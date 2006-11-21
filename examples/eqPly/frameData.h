
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_FRAMEDATA_H
#define EQ_PLY_FRAMEDATA_H

#include "eqPly.h"

#include <eq/eq.h>

class FrameData : public eqNet::Object
{
public:

    FrameData() : Object( TYPE_FRAMEDATA )
        {
            reset();
            setInstanceData( &_data, sizeof( Data ));
            EQINFO << "New FrameData " << std::endl;
        }

    FrameData( const void* data, const uint64_t size ) 
            : Object( TYPE_FRAMEDATA )
        {
            EQASSERT( size == sizeof( Data ));

            memcpy( &_data, data, sizeof( Data ));
            setInstanceData( &_data, sizeof( Data ));
            EQINFO << "New FrameData instance" << std::endl;
        }

    void reset()
        {
            _data.translation.z = -1.f;
            _data.rotation = vmml::Matrix4f::IDENTITY;
            _data.rotation.rotateX( -M_PI_2 );
            _data.rotation.rotateY( -M_PI_2 );
        }

    struct Data
    {
        vmml::Matrix4f rotation;
        vmml::Vector3f translation;
    } _data;

};



#endif // EQ_PLY_FRAMEDATA_H

