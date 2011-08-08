
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

#ifndef EQ_GLXTYPES_H
#define EQ_GLXTYPES_H

#include <eq/client/glx/types.h>

namespace eq
{
/** 
 * Set the current X display connection.
 *
 * This function stores a per-thread display connection, similar to the current
 * WGL/AGL context. It is used by some  eq and eq::util classes to retrieve the
 * display without having to know the eq::Pipe. The GLXPipe sets it
 * automatically. Applications using the GLX window system with a custom
 * SystemPipe implementation have to set it using this function.
 *
 * @param display the current display connection to use.
 */
void XSetCurrentDisplay( Display* display );

/** @return the current display connection for the calling thread. */
Display* XGetCurrentDisplay();

#ifndef EQ_2_0_API
    typedef glx::Pipe GLXPipe;
    typedef glx::WindowIF GLXWindowIF;
    typedef glx::Window GLXWindow;
    typedef glx::EventHandler GLXEventHandler;
    typedef glx::WindowEvent GLXWindowEvent;
#endif
}


#endif // EQ_GLXTYPES_H
