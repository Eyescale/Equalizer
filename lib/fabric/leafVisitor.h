
/* Copyright (c) 2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQFABRIC_CHANNELVISITOR_H
#define EQFABRIC_CHANNELVISITOR_H

#include <eq/fabric/visitorResult.h>  // enum

namespace eq
{
namespace fabric
{
    /** A visitor to traverse leaf nodes of a graph. */
    template< class T > class LeafVisitor
    {
    public:
        /** Constructs a new leaf visitor. @version 1.0 */
        LeafVisitor(){}
        
        /** Destruct the leaf visitor. @version 1.0 */
        virtual ~LeafVisitor(){}

        /** Visit a leaf node. @version 1.0 */
        virtual VisitorResult visit( T* leaf )
            { return visit( static_cast< const T* >( leaf )); }

        /** Visit a leaf node during a const traversal. @version 1.0 */
        virtual VisitorResult visit( const T* leaf )
            { return TRAVERSE_CONTINUE; }
    };
}
}
#endif // EQFABRIC_CHANNELVISITOR_H
