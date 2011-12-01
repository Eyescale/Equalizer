
/* Copyright (c) 2011 Daniel Pfeifer <daniel@pfeifer-mail.de>
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

#include "window.h"
#include "pipe.h"
#include "messagePump.h"

#include <eq/fabric/gpuInfo.h>

namespace eq
{
namespace glx
{

static class : WindowSystemIF
{
    std::string getName() const { return "GLX"; }

    eq::SystemWindow* createWindow(eq::Window* window) const
    {
        EQINFO << "Using glx::Window" << std::endl;
        return new Window(window);
    }

    eq::SystemPipe* createPipe(eq::Pipe* pipe) const
    {
        EQINFO << "Using glx::Pipe" << std::endl;
        return new Pipe(pipe);
    }

    eq::MessagePump* createMessagePump() const
    {
        return new MessagePump;
    }
} _glXFactory;

}
}
