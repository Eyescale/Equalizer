
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

#include <eq/fabric/gpuInfo.h>
#include <co/base/debug.h>

#define MAX_GPUS 32

namespace eq
{
namespace agl
{

static class : WindowSystemIF
{
    std::string getName() const { return "AGL"; }

    eq::SystemWindow* createWindow( eq::Window* window ) const
    {
        EQINFO << "Using agl::Window" << std::endl;
        return new Window(window);
    }

    eq::SystemPipe* createPipe( eq::Pipe* pipe ) const
    {
        EQINFO << "Using agl::Pipe" << std::endl;
        return new Pipe(pipe);
    }

    eq::MessagePump* createMessagePump() const
    {
        return new MessagePump;
    }

    GPUInfos discoverGPUs() const
    {
        EQASSERTINFO( false, "Moved to gpu-sd library" );
        GPUInfos result;
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
} _aglFactory;

}
}
