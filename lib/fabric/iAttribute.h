
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef EQFABRIC_IATTRIBUTE_H
#define EQFABRIC_IATTRIBUTE_H

#include <eq/base/base.h>

namespace eq
{
namespace fabric
{
    /** Possible values for integer attributes */
    enum IAttribute
    {
        UNDEFINED  = -0xfffffff, //!< Undefined value
        RGBA32F    = -13, //!< Float32 framebuffer (Window::IATTR_PLANES_COLOR)
        RGBA16F    = -12, //!< Float16 framebuffer (Window::IATTR_PLANES_COLOR)
        FBO        = -11, //!< FBO drawable (Window::IATTR_HINT_DRAWABLE)
        LOCAL_SYNC = -10, //!< Full local sync (Node::IATTR_THREAD_MODEL)
        DRAW_SYNC  = -9,  //!< Local draw sync (Node::IATTR_THREAD_MODEL)
        ASYNC      = -8,  //!< No local sync (Node::IATTR_THREAD_MODEL)
        PBUFFER    = -7,  //!< PBuffer drawable (Window::IATTR_HINT_DRAWABLE)
        WINDOW     = -6,  //!< Window drawable (Window::IATTR_HINT_DRAWABLE)
        VERTICAL   = -5,  //!< Vertical load-balancing
        QUAD       = -4,  //!< Quad-buffered stereo decomposition
        ANAGLYPH   = -3,  //!< Anaglyphic stereo decomposition
        /** 
         * Nicest statisics gathering (Window::IATTR_HINT_STATISTICS,
         * Channel::IATTR_HINT_STATISTICS)
         */
        NICEST     = -2,
        AUTO       = -1,  //!< Automatic selection (various attributes)
        OFF        = 0,   //!< disabled (various attributes)
        ON         = 1,   //!< enabled (various attributes)
        /** 
         * Fastest statisics gathering (Window::IATTR_HINT_STATISTICS,
         * Channel::IATTR_HINT_STATISTICS)
         */
        FASTEST    = ON,
        HORIZONTAL = ON   //!< Horizontal load-balancing
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                          const IAttribute value );
}
}

#endif // EQFABRIC_IATTRIBUTE_H

