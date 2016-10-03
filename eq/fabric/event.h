
/* Copyright (c) 2006-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQFABRIC_EVENT_H
#define EQFABRIC_EVENT_H

#include <eq/fabric/types.h>
#include <lunchbox/bitOperation.h>

namespace eq
{
namespace fabric
{
/**
 * Base event structure to report window system and other events.
 *
 * The originator typically contains the co::Object identifier of the entity
 * emitting the event.
 */
struct Event
{
    Event() : serial(0), time(0) {}

    uint32_t serial; //!< unique originator serial number
    int64_t time; //!< The config time when the event was created
    uint128_t originator; //!< The identifier of the entity emitting the event
};

/** Print the event to the given output stream. @version 1.0 */
inline std::ostream& operator << ( std::ostream& os, const Event& event )
    { return os << event.originator; }
}
}

namespace lunchbox
{
template<> inline void byteswap( eq::fabric::Event& value )
{
    byteswap( value.serial );
    byteswap( value.time );
    byteswap( value.originator );
}
}

#endif // EQFABRIC_EVENT_H
