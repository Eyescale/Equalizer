
/* Copyright (c) 2008-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 *                    2010, Cedric Stalder  <cedric.stalder@gmail.com>
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

#ifndef EQFABRIC_ELEMENTVISITOR_H
#define EQFABRIC_ELEMENTVISITOR_H

#include <eq/fabric/leafVisitor.h> // base class

namespace eq 
{
namespace fabric
{
    /** A visitor to traverse non-leaf elements and their children in a tree. */
    template< class T, class C > class ElementVisitor : public C
    {
    public:
        /** Construct a new visitor. */
        ElementVisitor(){}
        
        /** Destruct the visitor. */
        virtual ~ElementVisitor(){}  

        /** Visit an element on the down traversal. */
        virtual VisitorResult visitPre( T* element )
            { return visitPre( static_cast< const T* >( element )); }

        /** Visit an element on the up traversal. */
        virtual VisitorResult visitPost( T* element )
            { return visitPost( static_cast< const T* >( element )); }

        /** Visit an element on a const down traversal. */
        virtual VisitorResult visitPre( const T* )
            { return TRAVERSE_CONTINUE; }

        /** Visit an element on a const up traversal. */
        virtual VisitorResult visitPost( const T* )
            { return TRAVERSE_CONTINUE; }
    };
}
}
#endif // EQFABRIC_ELEMENTVISITOR_H
