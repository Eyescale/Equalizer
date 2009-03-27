
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQSERVER_CONSTCOMPOUNDVISITOR_H
#define EQSERVER_CONSTCOMPOUNDVISITOR_H

#include "visitorResult.h"  // enum

namespace eq
{
namespace server
{
    class Compound;

    /**
     * A visitor to traverse a const compound tree.
     */
    class ConstCompoundVisitor
    {
    public:
        /** Constructs a new CompoundVisitor. */
        ConstCompoundVisitor(){}
        
        /** Destruct the CompoundVisitor */
        virtual ~ConstCompoundVisitor(){}

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
#endif // EQSERVER_CONSTCOMPOUNDVISITOR_H
