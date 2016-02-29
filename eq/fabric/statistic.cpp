
/* Copyright (c) 2009-2016, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <vmmlib/vector.hpp>
#include <string>

#ifdef _WIN32
#  define bzero( ptr, size ) memset( ptr, 0, size );
#else
#  include <strings.h>
#endif

namespace eq
{
namespace fabric
{
namespace
{
struct StatisticData
{
    const Statistic::Type type;
    const std::string name;
    const Vector3f color;
};

static StatisticData _statisticData[] =
{{ Statistic::NONE,
   "NO EVENT",     Vector3f( 0.f, 0.f, 0.f ) },
 { Statistic::CHANNEL_CLEAR,
   "clear",        Vector3f( .5f, 1.0f, .5f ) },
 { Statistic::CHANNEL_DRAW,
   "draw",         Vector3f( 0.f, .9f, 0.f ) },
 { Statistic::CHANNEL_DRAW_FINISH,
   "finish draw",  Vector3f( 0.f, .5f, 0.f ) },
 { Statistic::CHANNEL_ASSEMBLE,
   "assemble",     Vector3f( 1.0f, 1.0f, 0.f ) },
 { Statistic::CHANNEL_FRAME_WAIT_READY,
   "wait frame",   Vector3f( 1.0f, 0.f, 0.f ) },
 { Statistic::CHANNEL_READBACK,
   "readback",     Vector3f( 1.0f, .5f, .5f ) },
 { Statistic::CHANNEL_ASYNC_READBACK,
   "readback",     Vector3f( 1.0f, .5f, .5f ) },
 { Statistic::CHANNEL_VIEW_FINISH,
   "view finish",  Vector3f( 1.f, 0.f, 1.0f ) },
 { Statistic::CHANNEL_FRAME_TRANSMIT,
   "transmit",     Vector3f( 0.f, 0.f, 1.0f ) },
 { Statistic::CHANNEL_FRAME_COMPRESS,
   "compress",     Vector3f( 0.f, .7f, 1.f ) },
 { Statistic::CHANNEL_FRAME_WAIT_SENDTOKEN,
   "wait send token", Vector3f( 1.f, 0.f, 0.f ) },
 { Statistic::WINDOW_FINISH,
   "finish",       Vector3f( 1.0f, 1.0f, 0.f ) },
 { Statistic::WINDOW_THROTTLE_FRAMERATE,
   "throttle",     Vector3f( 1.0f, 0.f, 1.f ) },
 { Statistic::WINDOW_SWAP_BARRIER,
   "barrier",      Vector3f( 1.0f, 0.f, 0.f ) },
 { Statistic::WINDOW_SWAP,
   "swap",         Vector3f( 1.f, 1.f, 1.f ) },
 { Statistic::WINDOW_FPS,
   "FPS",          Vector3f( 1.f, 1.f, 1.f ) },
 { Statistic::PIPE_IDLE,
   "pipe idle",    Vector3f( 1.f, 1.f, 1.f ) },
 { Statistic::NODE_FRAME_DECOMPRESS,
   "decompress",   Vector3f( 0.f, .7f, 1.f ) },
 { Statistic::CONFIG_START_FRAME,
   "start frame",  Vector3f( .5f, 1.0f, .5f ) },
 { Statistic::CONFIG_FINISH_FRAME,
   "finish frame", Vector3f( .5f, .5f, .5f ) },
 { Statistic::CONFIG_WAIT_FINISH_FRAME,
   "wait finish",  Vector3f( 1.0f, 0.f, 0.f ) },
 { Statistic::ALL,
   "ALL EVENTS",   Vector3f( 0.0f, 0.f, 0.f ) }} ;
}

const std::string& Statistic::getName( const Type type )
{
    LBASSERTINFO( _statisticData[ type ].type == type, int( type ));
    return _statisticData[ type ].name;
}

const Vector3f& Statistic::getColor( const Type type )
{
    LBASSERTINFO( _statisticData[ type ].type == type, type );
    return _statisticData[ type ].color;
}

std::ostream& operator << ( std::ostream& os, const Statistic::Type& type )
{
    os << Statistic::getName( type );
    return os;
}

std::ostream& operator << ( std::ostream& os, const Statistic& event )
{
    os << event.resourceName << ": " << event.type << ' ' << event.frameNumber
       << ' ' << event.task << ' ' << event.startTime << " - " << event.endTime
       << ' ' << event.idleTime << '/' << event.totalTime;
    return os;
}

}
}
