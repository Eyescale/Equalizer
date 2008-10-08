
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EVOLVE_FRAMEDATA_H
#define EVOLVE_FRAMEDATA_H

#include "eVolve.h"

#include <eq/eq.h>

namespace eVolve
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
            Data() : ortho( false ), statistics( false ) {}

            vmml::Matrix4f rotation;
            vmml::Vector3f translation;
            bool           ortho;
            bool           statistics;
        } data;
    
    protected:
        virtual ChangeType getChangeType() const { return INSTANCE; }

        virtual void getInstanceData( eq::net::DataOStream& os )
            { os.writeOnce( &data, sizeof( data )); }

        virtual void applyInstanceData( eq::net::DataIStream& is )
            {
                memcpy( &data, is.getRemainingBuffer(), sizeof( data ));
                is.advanceBuffer( sizeof( data ));
            }
    };
}


#endif // EVOLVE_FRAMEDATA_H

