
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

#include "eventHandler.h"
#include "messagePump.h"
#include "pipe.h"
#include "window.h"

#include "../config.h"
#include "../node.h"
#include "../pipe.h"
#include "../server.h"

#include <eq/fabric/gpuInfo.h>

namespace eq
{
namespace wgl
{

static class : WindowSystemIF
{
    std::string getName() const { return "WGL"; }

    eq::SystemWindow* createWindow(eq::Window* window) const
    {
        EQINFO << "Using wgl::Window" << std::endl;
        return new Window(window);
    }

    eq::SystemPipe* createPipe(eq::Pipe* pipe) const
    {
        EQINFO << "Using wgl::Pipe" << std::endl;
        return new Pipe(pipe);
    }

    eq::MessagePump* createMessagePump() const
    {
        return new MessagePump;
    }

#define wglewGetContext wglPipe->wglewGetContext

    GPUInfos discoverGPUs() const
    {
        // Create fake config to use wgl::Pipe affinity code for queries
        ServerPtr server = new Server;
        Config* config = new Config( server );
        Node* node = new Node( config );
        eq::Pipe* pipe = new eq::Pipe( node );
        Pipe* wglPipe = new Pipe( pipe );

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
        EventHandler::initMagellan(node);
#endif
    }

    void configExit(eq::Node* node) const
    {
#ifdef EQ_USE_MAGELLAN
        EventHandler::exitMagellan(node);
#endif
    }
} _wglFactory;

}
}
