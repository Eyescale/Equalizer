
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
static std::string _statisticNames[Statistic::ALL] =
{
    "NO EVENT",
    "clear",
    "draw",
    "finish draw",
    "assemble",
    "wait frame",
    "readback",
    "finish",
    "throttle",
    "barrier",
    "swap",
    "pipe idle",
    "transmit",
    "compress",
    "receive",
    "start frame",
    "finish frame",
    "wait finish"
};

static Vector3f _statisticColors[Statistic::ALL] = 
{
    Vector3f( 0.f, 0.f, 0.f ),
    Vector3f( .5f, 1.0f, .5f ), // clear
    Vector3f( 0.f, 1.0f, 0.f ), // draw
    Vector3f( 0.f, .5f, 0.f ), // draw finish
    Vector3f( 1.0f, 1.0f, 0.f ),  // assemble
    Vector3f( 1.0f, 0.f, 0.f ), // wait frame
    Vector3f( 1.0f, .5f, .5f ), // readback
    Vector3f( 1.0f, 1.0f, 0.f ), // finish
    Vector3f( 1.0f, 0.f, 1.f ), // throttle
    Vector3f( 1.0f, 0.f, 0.f ), // swap barrier
    Vector3f( 1.f, 1.f, 1.f ), // swap
    Vector3f( 1.f, 1.f, 1.f ), // pipe idle
    Vector3f( 0.f, 0.f, 1.0f ), // transmit
    Vector3f( .7f, .7f, 1.f ), // compress
    Vector3f( .7f, 1.f, .7f ), // decompress
    Vector3f( .5f, 1.0f, .5f ), // start frame
    Vector3f( .5f, .5f, .5f ), // finish frame
    Vector3f( 1.0f, 0.f, 0.f ) // wait finish frame
};
}

const std::string& Statistic::getName( const Type type )
{
    EQASSERT( sizeof( _statisticNames ) / sizeof( std::string ) == ALL );

    return _statisticNames[ type ];
}

const Vector3f& Statistic::getColor( const Type type )
{
    EQASSERT( sizeof( _statisticColors ) / sizeof( Vector3f ) == ALL );

    return _statisticColors[ type ];
}

std::ostream& operator << ( std::ostream& os, const Statistic& event )
{
    os << event.resourceName << ": " << Statistic::getName( event.type )
       << ' ' << event.frameNumber << ' ' << event.task << ' ' 
       << event.startTime << " - " << event.endTime;
    return os;
}

}
