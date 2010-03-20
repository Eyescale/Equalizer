
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

#include "segment.h"

#include "canvas.h"
#include "config.h"

#include <eq/fabric/leafVisitor.h>
#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

using namespace eq::base;

namespace eq
{

Segment::Segment()
        : _canvas( 0 )
{
}

Segment::~Segment()
{
    EQASSERT( _canvas == 0 );
}

void Segment::serialize( net::DataOStream& os, const uint64_t dirtyBits )
{
    Frustum::serialize( os, dirtyBits );

    if( dirtyBits & DIRTY_VIEWPORT )
        os << _vp;
}

void Segment::deserialize( net::DataIStream& is, const uint64_t dirtyBits )
{
    Frustum::deserialize( is, dirtyBits );

    if( dirtyBits & DIRTY_VIEWPORT )
        is >> _vp;
}

Config* Segment::getConfig()
{
    EQASSERT( _canvas );
    return _canvas ? _canvas->getConfig() : 0;
}

const Config* Segment::getConfig() const
{
    EQASSERT( _canvas );
    return _canvas ? _canvas->getConfig() : 0;
}

VisitorResult Segment::accept( SegmentVisitor& visitor )
{
    return visitor.visit( this );
}

VisitorResult Segment::accept( SegmentVisitor& visitor ) const
{
    return visitor.visit( this );
}

}
