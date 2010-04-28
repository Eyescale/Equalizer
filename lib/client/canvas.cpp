
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "canvas.h"

#include "config.h"
#include "global.h"
#include "layout.h"
#include "nodeFactory.h"
#include "segment.h"

#include <eq/fabric/elementVisitor.h>
#include <eq/net/dataIStream.h>
#include <eq/net/dataOStream.h>

namespace eq
{
typedef fabric::Canvas< Config, Canvas, Segment, Layout > Super;

Canvas::Canvas( Config* parent )
        : Super( parent )
{
}


Canvas::~Canvas()
{
    EQASSERT( getSegments().empty( ));
}

void Canvas::_unmap()
{
    EQASSERT( !isMaster( ));

    Config* config = getConfig();
    NodeFactory* nodeFactory = Global::getNodeFactory();
    const SegmentVector& segments = getSegments();

    while( !segments.empty( ))
    {
        Segment* segment = _segments.back();
        EQASSERT( segment->getID() != EQ_ID_INVALID );
        EQASSERT( !segment->isMaster( ));

        config->unmapObject( segment );
        _removeSegment( segment );
        nodeFactory->releaseSegment( segment );
    }

    config->unmapObject( this );
}

}


#include "server.h"
#include "../fabric/canvas.ipp"
template class eq::fabric::Canvas< eq::Config, eq::Canvas, eq::Segment,
                                   eq::Layout >;
/** @cond IGNORE */
template std::ostream& eq::fabric::operator << ( std::ostream&,
                                                 const eq::Super& );
/** @endcond */
