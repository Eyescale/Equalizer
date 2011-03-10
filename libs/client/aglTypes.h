
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

#ifndef EQ_AGLTYPES_H
#define EQ_AGLTYPES_H

#include <co/base/types.h>

/** @cond IGNORE */
typedef uint32_t CGDirectDisplayID;
typedef struct OpaqueWindowPtr* WindowPtr;
typedef WindowPtr WindowRef;
typedef struct OpaqueEventHandlerRef*   EventHandlerRef;
typedef struct __AGLContextRec* AGLContext;
typedef struct __AGLPixelFormatRec* AGLPixelFormat;
typedef struct __AGLPBufferRec* AGLPbuffer;
typedef struct OpaqueEventRef* EventRef;
typedef struct OpaqueEventQueueRef* EventQueueRef;
/** @endcond */

#endif // EQ_AGLTYPES_H
