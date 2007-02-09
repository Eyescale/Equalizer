
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_FRAMEDATA_H
#define EQ_PLY_FRAMEDATA_H

#include "eqPly.h"

#include <eq/eq.h>

class FrameData : public eqNet::Object
{
public:

    FrameData()
        {
            reset();
            setInstanceData( &data, sizeof( Data ));
            EQINFO << "New FrameData " << std::endl;
        }

    virtual uint32_t getTypeID() const { return TYPE_FRAMEDATA; }

    void reset()
        {
            data.translation   = vmml::Vector3f::ZERO;
            data.translation.z = -1.f;
            data.rotation = vmml::Matrix4f::IDENTITY;
            data.rotation.rotateX( -M_PI_2 );
            data.rotation.rotateY( -M_PI_2 );
        }

    struct Data
    {
        vmml::Matrix4f rotation;
        vmml::Vector3f translation;
        bool           color;
    } data;
    
protected:
    virtual bool isStatic() const { return false; }
};



#endif // EQ_PLY_FRAMEDATA_H

