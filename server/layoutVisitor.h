
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

#ifndef EQSERVER_LAYOUTVISITOR_H
#define EQSERVER_LAYOUTVISITOR_H

#include "viewVisitor.h"

namespace eq
{
namespace server
{
    class Layout;

    /**
     * A visitor to traverse non-const layouts and children.
     */
    class LayoutVisitor : public ViewVisitor
    {
    public:
        /** Constructs a new LayoutVisitor. */
        LayoutVisitor(){}
        
        /** Destruct the LayoutVisitor */
        virtual ~LayoutVisitor(){}

        /** Visit a layout on the down traversal. */
        virtual VisitorResult visitPre( Layout* layout )
            { return TRAVERSE_CONTINUE; }

        /** Visit a layout on the up traversal. */
        virtual VisitorResult visitPost( Layout* layout )
            { return TRAVERSE_CONTINUE; }
    };

    /**
     * A visitor to traverse const layouts and children.
     */
    class ConstLayoutVisitor : public ConstViewVisitor
    {
    public:
        /** Constructs a new LayoutVisitor. */
        ConstLayoutVisitor(){}
        
        /** Destruct the LayoutVisitor */
        virtual ~ConstLayoutVisitor(){}

        /** Visit a layout on the down traversal. */
        virtual VisitorResult visitPre( const Layout* layout )
            { return TRAVERSE_CONTINUE; }

        /** Visit a layout on the up traversal. */
        virtual VisitorResult visitPost( const Layout* layout )
            { return TRAVERSE_CONTINUE; }
    };
}
}
#endif // EQSERVER_LAYOUTVISITOR_H
