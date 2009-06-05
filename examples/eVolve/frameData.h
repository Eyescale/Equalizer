
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

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

