
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_FRAMEDATA_H
#define EQ_PLY_FRAMEDATA_H

#include "eqPly.h"

#include <eq/eq.h>

namespace eqPly
{
    class FrameData : public eqNet::Object
    {
    public:

        FrameData()
            {
                reset();
                setInstanceData( &data, sizeof( Data ));
                EQINFO << "New FrameData " << std::endl;
            }

        void reset()
            {
                data.translation   = vmml::Vector3f::ZERO;
                data.translation.z = -2.f;
                data.rotation = vmml::Matrix4f::IDENTITY;
                data.rotation.rotateX( static_cast<float>( -M_PI_2 ));
                data.rotation.rotateY( static_cast<float>( -M_PI_2 ));
            }

        struct Data
        {
            vmml::Matrix4f rotation;
            vmml::Vector3f translation;
            bool           color;
        } data;
    
    protected:
        virtual ChangeType getChangeType() const { return INSTANCE; }
    };
}


#endif // EQ_PLY_FRAMEDATA_H

