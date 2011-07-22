
/* Copyright (c) 2011, Daniel Pfeifer <daniel@pfeifer-mail.de>
 *               2011, Stefan Eilemann <eile@eyescale.ch>
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

#include "../windowSystem.h"

#include "../aglWindow.h"
#include "../aglPipe.h"
#include "../aglMessagePump.h"
#include "../os.h"

#include <eq/fabric/gpuInfo.h>

#define MAX_GPUS 32

namespace eq
{

static class : WindowSystemIF
{
    std::string name() const { return "AGL"; }

    eq::SystemWindow* createWindow(eq::Window* window) const
    {
        EQINFO << "Using AGLWindow" << std::endl;
        return new AGLWindow(window);
    }

    eq::SystemPipe* createPipe(eq::Pipe* pipe) const
    {
        EQINFO << "Using AGLPipe" << std::endl;
        return new AGLPipe(pipe);
    }

    eq::MessagePump* createMessagePump() const
    {
        return new AGLMessagePump;
    }

    GPUInfos discoverGPUs() const
    {
        const CGDirectDisplayID mainDisplayID = CGMainDisplayID();

        CGDirectDisplayID displayIDs[MAX_GPUS];
        CGDisplayCount    nDisplays = 0;
        if( CGGetOnlineDisplayList( MAX_GPUS, displayIDs, &nDisplays ) !=
            kCGErrorSuccess )
        {
            GPUInfos result;
            result.push_back( GPUInfo() );
            return result;
        }

        std::deque< GPUInfo > infos;
        for( CGDisplayCount i = 0; i < nDisplays; ++i )
        {
            GPUInfo info;
            const CGRect displayRect = CGDisplayBounds( displayIDs[i] );

            info.device = i;
            info.pvp.x = int32_t(displayRect.origin.x);
            info.pvp.y = int32_t(displayRect.origin.y);
            info.pvp.w = int32_t(displayRect.size.width);
            info.pvp.h = int32_t(displayRect.size.height);

            if( mainDisplayID == displayIDs[i] )
                infos.push_front( info );
            else
                infos.push_back( info );
        }

        GPUInfos result( infos.size( ));
        std::copy( infos.begin(), infos.end(), result.begin( ));
        return result;
    }

    void configInit(eq::Node* node) const
    {
#ifdef EQ_USE_MAGELLAN
        AGLEventHandler::initMagellan(node);
#endif
    }

    void configExit(eq::Node* node) const
    {
#ifdef EQ_USE_MAGELLAN
        AGLEventHandler::exitMagellan(node);
#endif
    }
} _aglFactory;

} // namespace eq
