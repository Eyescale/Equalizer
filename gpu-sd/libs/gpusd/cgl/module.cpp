
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

#include "module.h"

#include <gpusd/gpuInfo.h>

//#include <AGL/agl.h>
#include <Carbon/Carbon.h>

#include <deque>

#define MAX_GPUS 32

namespace gpusd
{
namespace cgl
{
namespace
{
    Module* instance = 0;
}

void Module::use()
{
    if( !instance )
        instance = new Module;
}

GPUInfos Module::discoverGPUs_() const
{
    GPUInfos result;
    CGDirectDisplayID displayIDs[MAX_GPUS];
    CGDisplayCount    nDisplays = 0;

    if( CGGetOnlineDisplayList( MAX_GPUS, displayIDs, &nDisplays ) !=
        kCGErrorSuccess )
    {
        return result;
    }

    const CGDirectDisplayID mainDisplayID = CGMainDisplayID();
    std::deque< GPUInfo > infos;
    for( CGDisplayCount i = 0; i < nDisplays; ++i )
    {
        GPUInfo info( "CGL" );
        const CGRect displayRect = CGDisplayBounds( displayIDs[i] );

        info.device = i;
        info.pvp[0] = int32_t(displayRect.origin.x);
        info.pvp[1] = int32_t(displayRect.origin.y);
        info.pvp[2] = int32_t(displayRect.size.width);
        info.pvp[3] = int32_t(displayRect.size.height);

        if( mainDisplayID == displayIDs[i] )
            infos.push_front( info );
        else
            infos.push_back( info );
    }

    result.resize( infos.size( ));
    std::copy( infos.begin(), infos.end(), result.begin( ));
    return result;
}

}
}

