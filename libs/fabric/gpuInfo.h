
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
        GPUInfo() : port( EQ_UNDEFINED_UINT32 ), device( EQ_UNDEFINED_UINT32 )
            {}

        /** The display (GLX) or ignored (Win32, AGL). */
        uint32_t port;

        /** The screen (GLX), GPU (Win32) or virtual screen (AGL). */
        uint32_t device;

        /** The size and location of the GPU. */
        PixelViewport pvp;
    };
}
}
#endif // EQFABRIC_GPUINFO_H

