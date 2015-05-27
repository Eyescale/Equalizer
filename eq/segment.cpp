
/* Copyright (c) 2009-2015, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "segment.h"

#include "canvas.h"
#include "channel.h"
#include "config.h"
#include "pipe.h"
#include "server.h"
#include "window.h"

#pragma clang diagnostic ignored "-Wunused-private-field" // _impl is unused

namespace eq
{
typedef fabric::Segment< Canvas, Segment, Channel > Super;

Segment::Segment( Canvas* parent )
        : Super( parent )
        , _impl( 0 )
{
}

Segment::~Segment()
{
}

Config* Segment::getConfig()
{
    LBASSERT( getCanvas() );
    return getCanvas() ? getCanvas()->getConfig() : 0;
}

const Config* Segment::getConfig() const
{
    LBASSERT( getCanvas() );
    return getCanvas() ? getCanvas()->getConfig() : 0;
}

ServerPtr Segment::getServer()
{
    Canvas* canvas = getCanvas();
    LBASSERT( canvas );
    return ( canvas ? canvas->getServer() : 0 );
}

}

#include <eq/fabric/segment.ipp>
template class eq::fabric::Segment< eq::Canvas, eq::Segment, eq::Channel >;
/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                                                 const eq::Super& );
/** @endcond */
