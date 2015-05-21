
/* Copyright (c) 2011-2013, Stefan Eilemann <eile@eyescale.ch>
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

#include <eq/os.h>
#ifdef AGL
#  include <eq/agl/eventHandler.h>
#  include <eq/agl/pipe.h>
#  include <eq/agl/types.h>
#  include <eq/agl/window.h>
#endif
#ifdef GLX
#  include <eq/glx/eventHandler.h>
#  include <eq/glx/pipe.h>
#  include <eq/glx/types.h>
#  include <eq/glx/window.h>
#endif
#ifdef WGL
#  include <eq/wgl/eventHandler.h>
#  include <eq/wgl/pipe.h>
#  include <eq/wgl/types.h>
#  include <eq/wgl/window.h>
#endif

#endif // EQ_SYSTEM_H
