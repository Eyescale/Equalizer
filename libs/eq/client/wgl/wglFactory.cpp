
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

#include "../config.h"
#include "../node.h"
#include "../pipe.h"
#include "../server.h"
#include "../wglEventHandler.h"
#include "../wglMessagePump.h"
#include "../wglPipe.h"
#include "../wglWindow.h"

#include <eq/fabric/gpuInfo.h>

namespace eq
{

static class : WindowSystemImpl< 'W', 'G', 'L' >
{
    eq::SystemWindow* createSystemWindow(eq::Window* window) const
    {
        EQINFO << "Using WGLWindow" << std::endl;
        return new WGLWindow(window);
    }

    eq::SystemPipe* createSystemPipe(eq::Pipe* pipe) const
    {
        EQINFO << "Using WGLPipe" << std::endl;
        return new WGLPipe(pipe);
    }

    eq::MessagePump* createMessagePump() const
    {
        return new WGLMessagePump;
    }

#define wglewGetContext wglPipe->wglewGetContext

    GPUInfos discoverGPUs() const
    {
        // Create fake config to use WGLPipe affinity code for queries
        ServerPtr server = new Server;
        Config* config = new Config( server );
        Node* node = new Node( config );
        Pipe* pipe = new Pipe( node );
        WGLPipe* wglPipe = new WGLPipe( pipe );

        GPUInfos result;
        if( !wglPipe->configInit( ))
        {
            wglPipe->configExit();
            return result;
        }

        if( !WGLEW_NV_gpu_affinity )
        {
            GPUInfo info;
            info.pvp = pipe->getPixelViewport();
            result.push_back( info );

            wglPipe->configExit();
            return result;
        }

        for( uint32_t i = 0; i < EQ_UNDEFINED_UINT32; ++i )
        {
            pipe->setDevice( i );
            pipe->setPixelViewport( PixelViewport( ));

            HDC dc;
            if( wglPipe->createWGLAffinityDC( dc ))
            {
                GPUInfo info;
                info.device = i;
                info.pvp = pipe->getPixelViewport();
                result.push_back( info );

                wglDeleteDCNV( dc );
            }
            else
            {
                wglPipe->configExit();
                return result;
            }
        }

        EQASSERTINFO( 0, "Unreachable" );
        wglPipe->configExit();
        return result;
    }

    void configInit(eq::Node* node) const
    {
#ifdef EQ_USE_MAGELLAN
        WGLEventHandler::initMagellan(node);
#endif
    }

    void configExit(eq::Node* node) const
    {
#ifdef EQ_USE_MAGELLAN
        WGLEventHandler::exitMagellan(node);
#endif
    }
} _wglFactory;

} // namespace eq
