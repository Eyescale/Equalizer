
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EVOLVE_FRAMEDATA_H
#define EVOLVE_FRAMEDATA_H

#include "eVolve.h"

#include <eq/eq.h>

namespace eVolve
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
                data.translation.z = -3.f;
                data.rotation = vmml::Matrix4f::IDENTITY;
                data.rotation.rotateX( static_cast<float>( -M_PI_2 ));
                data.rotation.rotateY( static_cast<float>( -M_PI_2 ));
            }

        struct Data
        {
            vmml::Matrix4f rotation;
            vmml::Vector3f translation;
        } data;
    
    protected:
        virtual bool isStatic() const { return false; }
    };
}


#endif // EVOLVE_FRAMEDATA_H

