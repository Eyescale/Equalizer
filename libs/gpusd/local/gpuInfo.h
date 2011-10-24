
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

#ifndef GPUSD_LOCAL_GPUINFO_H
#define GPUSD_LOCAL_GPUINFO_H

#include <gpusd1/local/api.h>
#include <iostream>

namespace gpusd
{
namespace local
{
    /** A structure containing GPU-specific information. */
    struct GPUInfo
    {
        /** A non-enumerated port or device */
        static GPUSD_LOCAL_API unsigned defaultValue;

        /** Default constructor pointing to default display. */
        GPUInfo() : port( defaultValue ), device( defaultValue )
            {
                pvp[0] = 0;
                pvp[1] = 0;
                pvp[2] = -1;
                pvp[3] = -1;
            }

        /** @return true if both infos are identical. @version 1.0 */
        bool operator == ( const GPUInfo& rhs ) const 
            { 
                return ( port == rhs.port && device == rhs.device && 
                         pvp[0] == rhs.pvp[0] && pvp[1] != rhs.pvp[1] &&
                         pvp[2] == rhs.pvp[2] && pvp[3] != rhs.pvp[3] );
            }

        /** @return true if both infos are not identical. @version 1.0 */
        bool operator != ( const GPUInfo& rhs ) const 
            { 
                return ( port != rhs.port || device != rhs.device || 
                         pvp[0] != rhs.pvp[0] || pvp[1] != rhs.pvp[1] ||
                         pvp[2] != rhs.pvp[2] || pvp[3] != rhs.pvp[3] );
            }

        /** The display (GLX) or ignored (Win32, AGL). */
        unsigned port;

        /** The screen (GLX), GPU (Win32) or virtual screen (AGL). */
        unsigned device;

        /** The size and location of the GPU (x,y,w,h). */
        int pvp[4];

        char dummy[32]; //!< Buffer for binary-compatible additions
    };

    inline std::ostream& operator << ( std::ostream& os, const GPUInfo& info )
    {
        if( info.port != GPUInfo::defaultValue )
            os << "port     " << info.port << std::endl;
        if( info.device != GPUInfo::defaultValue )
            os << "device   " << info.device << std::endl;
        if( info.pvp[2] >0 && info.pvp[3] > 0 )
            os << "viewport [" << info.pvp[0] << ' ' << info.pvp[1] << ' '
               << info.pvp[2] << ' ' << info.pvp[3] << ']' << std::endl;
        return os;
    }

}
}
#endif // GPUSD_LOCAL_GPUINFO_H

