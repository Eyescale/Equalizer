
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#ifndef EQSERVER_SEGMENTVISITOR_H
#define EQSERVER_SEGMENTVISITOR_H

#include "visitorResult.h"  // enum

namespace eq
{
namespace server
{
    class Segment;

    /**
     * A visitor to traverse non-const segments.
     */
    class SegmentVisitor
    {
    public:
        /** Constructs a new SegmentVisitor. */
        SegmentVisitor(){}
        
        /** Destruct the SegmentVisitor */
        virtual ~SegmentVisitor(){}

        /** Visit a segment. */
        virtual VisitorResult visit( Segment* segment )
            { return TRAVERSE_CONTINUE; }
    };

    /**
     * A visitor to traverse const segments.
     */
    class ConstSegmentVisitor
    {
    public:
        /** Constructs a new SegmentVisitor. */
        ConstSegmentVisitor(){}
        
        /** Destruct the SegmentVisitor */
        virtual ~ConstSegmentVisitor(){}

        /** Visit a segment. */
        virtual VisitorResult visit( const Segment* segment )
            { return TRAVERSE_CONTINUE; }
    };
}
}
#endif // EQSERVER_SEGMENTVISITOR_H
