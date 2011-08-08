
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

#ifndef EQ_SYSTEM_H
#define EQ_SYSTEM_H

#include <eq/client/os.h>
#ifdef AGL
#  include <eq/client/agl/eventHandler.h>
#  include <eq/client/agl/pipe.h>
#  include <eq/client/agl/window.h>
#  include <eq/client/aglTypes.h>
#endif
#ifdef GLX
#  include <eq/client/glx/eventHandler.h>
#  include <eq/client/glx/pipe.h>
#  include <eq/client/glx/window.h>
#  include <eq/client/glXTypes.h>
#endif
#ifdef WGL
#  include <eq/client/wglEventHandler.h>
#  include <eq/client/wglPipe.h>
#  include <eq/client/wglWindow.h>
#endif

#endif // EQ_SYSTEM_H
