
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PLY_FRAMEDATA_H
#define EQ_PLY_FRAMEDATA_H

#include "eqPly.h"

#include <eq/eq.h>

namespace eqPly
{
    class FrameData : public eq::net::Object
    {
    public:

        FrameData()
            {
                reset();
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
            Data() : renderMode( mesh::RENDER_MODE_DISPLAY_LIST ), color( true )
                   , ortho( false ), statistics( false ), wireframe( false ) {}

            vmml::Matrix4f rotation;
            vmml::Vector3f translation;
            mesh::RenderMode renderMode;
            bool           color;
            bool           ortho;
            bool           statistics;
            bool           wireframe;
        } data;
    
    protected:
        virtual ChangeType getChangeType() const { return INSTANCE; }

        virtual void getInstanceData( eq::net::DataOStream& os )
            { os << data; }

        virtual void applyInstanceData( eq::net::DataIStream& is )
            { is >> data; }
    };
}


#endif // EQ_PLY_FRAMEDATA_H

