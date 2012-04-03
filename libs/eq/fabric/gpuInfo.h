
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef EQFABRIC_GPUINFO_H
#define EQFABRIC_GPUINFO_H

#include <eq/fabric/types.h>
#include <iostream>

namespace eq
{
namespace fabric
{
    /** A structure containing GPU-specific information. */
    struct GPUInfo
    {
        /** Default constructor pointing to default display. */
        GPUInfo() : port( LB_UNDEFINED_UINT32 ), device( LB_UNDEFINED_UINT32 )
            {}

        /** @return true if both infos are identical. @version 1.0 */
        bool operator == ( const GPUInfo& rhs ) const 
            { return port==rhs.port && device==rhs.device && pvp==rhs.pvp; }

        /** @return true if both infos are not identical. @version 1.0 */
        bool operator != ( const GPUInfo& rhs ) const 
            { return port!=rhs.port || device!=rhs.device || pvp!=rhs.pvp; }

        /** The display (GLX) or ignored (Win32, AGL). */
        uint32_t port;

        /** The screen (GLX), GPU (Win32) or virtual screen (AGL). */
        uint32_t device;

        /** The size and location of the GPU. */
        PixelViewport pvp;

        std::string hostname; //!< remote system  hostname, empty for local GPUs
    };

    inline std::ostream& operator << ( std::ostream& os, const GPUInfo& info )
    {
        if( !info.hostname.empty( ))
            os << "hostname " << info.hostname << std::endl;
        if( info.port != LB_UNDEFINED_UINT32 )
            os << "port     " << info.port << std::endl;
        if( info.device != LB_UNDEFINED_UINT32 )
            os << "device   " << info.device << std::endl;
        if( info.pvp.isValid( ))
            os << "viewport " << info.pvp << std::endl;
        return os;
    }

}
}
#endif // EQFABRIC_GPUINFO_H

