
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQSERVER_COMPOUNDVISITOR_H
#define EQSERVER_COMPOUNDVISITOR_H

#include "visitorResult.h"  // enum

namespace eq
{
namespace server
{
    class Compound;

    /**
     * A visitor to traverse a non-const compound tree.
     */
    class CompoundVisitor
    {
    public:
        /** Constructs a new CompoundVisitor. */
        CompoundVisitor(){}
        
        /** Destruct the CompoundVisitor */
        virtual ~CompoundVisitor(){}

        /** Visit a non-leaf compound on the down traversal. */
        virtual VisitorResult visitPre( Compound* compound )
            { return visit( compound ); }
        /** Visit a leaf compound. */
        virtual VisitorResult visitLeaf( Compound* compound )
            { return visit( compound ); }
        /** Visit a non-leaf compound on the up traversal. */
        virtual VisitorResult visitPost( Compound* compound )
            { return visitPost( static_cast< const Compound* >( compound )); }

        /** Visit every compound on the down traversal. */
        virtual VisitorResult visit( Compound* compound )
            { return visit( static_cast< const Compound* >( compound )); }


        /** Visit a non-leaf compound on the down traversal. */
        virtual VisitorResult visitPre( const Compound* compound )
            { return visit( compound ); }
        /** Visit a leaf compound. */
        virtual VisitorResult visitLeaf( const Compound* compound )
            { return visit( compound ); }
        /** Visit a non-leaf compound on the up traversal. */
        virtual VisitorResult visitPost( const Compound* compound )
            { return TRAVERSE_CONTINUE; }

        /** Visit every compound on the down traversal. */
        virtual VisitorResult visit( const Compound* compound )
            { return TRAVERSE_CONTINUE; }
    };
}
}
#endif // EQSERVER_COMPOUNDVISITOR_H
