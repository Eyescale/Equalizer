
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

#ifndef EQ_GLX_TYPES_H
#define EQ_GLX_TYPES_H

#include <lunchbox/types.h>

namespace eq
{
/** 
 * @namespace eq::glx
 * @brief The system abstraction layer for X11 and glX.
 */
namespace glx
{

class EventHandler;
class Pipe;
class Window;
class WindowIF;
class WindowEvent;

}
}

/** @cond INTERNAL */
typedef struct _XDisplay Display;
typedef union _XEvent XEvent;
typedef unsigned long XID;
typedef struct __GLXcontextRec* GLXContext;
typedef struct __GLXFBConfigRec* GLXFBConfig;

/** @endcond */

#endif // EQ_AGL_TYPES_H
