
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "statistic.h"

#include <string>

#ifdef WIN32
#  define bzero( ptr, size ) memset( ptr, 0, size );
#else
#  include <strings.h>
#endif

namespace eq
{
namespace
{
/** String representation of statistic event types. */
static std::string _statEventTypeNames[Statistic::TYPE_ALL] =
{
    "NO EVENT",
    "channel clear ",
    "channel draw",
    "channel finishdraw",
    "channel assemble",
    "channel readback",
    "channel wait frame",
    "window finish",
    "window swap barrier",
    "window swap buffer",
    "window throttle framerate",
    "pipe idle",
    "node transmit"
    "node compress",
    "node decompress",
    "config start frame",
    "config finish frame",
    "config wait finish"
};
}

std::ostream& operator << ( std::ostream& os, const Statistic& event )
{
    os << event.resourceName << ": " << _statEventTypeNames[ event.type ]
       << ' ' << event.frameNumber << ' ' << event.task << ' ' 
       << event.startTime << " - " << event.endTime;
    return os;
}

}
