
/* Copyright (c) 2010-2012, Stefan Eilemann <eile@eyescale.ch>
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

#include <eq/fabric/api.h>
#include <lunchbox/thread.h>
#include <iostream>

namespace eq
{
namespace fabric
{
    /** Possible values for integer attributes */
    enum IAttribute
    {
        UNDEFINED  = -0xfffffff, //!< Undefined value
        SOCKET = lunchbox::Thread::SOCKET, //!< CPU thread affinity: -64k..-1024
        CORE = lunchbox::Thread::CORE, //!< Core thread affinity: 1..oo
        SOCKET_MAX = lunchbox::Thread::SOCKET_MAX, //!< Highes bindable CPU
        RELATIVE_TO_OBSERVER = -17, //!< focal convergence relative to observer
        RELATIVE_TO_ORIGIN   = -16, //!< focal convergence relative to origin
        FIXED      = -15, //!< config or observer focus fixed on wall/projection
        RGBA32F    = -14, //!< Float32 framebuffer (WindowSettings::IATTR_PLANES_COLOR)
        RGBA16F    = -13, //!< Float16 framebuffer (WindowSettings::IATTR_PLANES_COLOR)
        FBO        = -12, //!< FBO drawable (WindowSettings::IATTR_HINT_DRAWABLE)
        LOCAL_SYNC = -11, //!< Full local sync (Node::IATTR_THREAD_MODEL)
        DRAW_SYNC  = -10, //!< Local draw sync (Node::IATTR_THREAD_MODEL)
        ASYNC      = -9,  //!< No local sync (Node::IATTR_THREAD_MODEL)
        PBUFFER    = -8,  //!< PBuffer drawable (WindowSettings::IATTR_HINT_DRAWABLE)
        WINDOW     = -7,  //!< Window drawable (WindowSettings::IATTR_HINT_DRAWABLE)
        VERTICAL   = -6,  //!< Vertical load-balancing
        QUAD       = -5,  //!< Quad-buffered stereo decomposition
        ANAGLYPH   = -4,  //!< Anaglyphic stereo decomposition
        PASSIVE    = -3,  //!< Passive stereo rendering
        /**
         * Nicest statistics gathering (WindowSettings::IATTR_HINT_STATISTICS,
         * Channel::IATTR_HINT_STATISTICS)
         */
        NICEST     = -2,
        AUTO       = -1,  //!< Automatic selection (various attributes)
        OFF        = 0,   //!< disabled (various attributes)
        ON         = 1,   //!< enabled (various attributes)
        /**
         * Fastest statistics gathering (WindowSettings::IATTR_HINT_STATISTICS,
         * Channel::IATTR_HINT_STATISTICS)
         */
        FASTEST    = ON,
        HORIZONTAL = ON    //!< Horizontal load-balancing
    };

    EQFABRIC_API std::ostream& operator << ( std::ostream& os,
                                             const IAttribute value );
}
}

#endif // EQFABRIC_IATTRIBUTE_H
