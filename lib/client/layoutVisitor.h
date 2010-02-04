
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

#ifndef EQ_LAYOUTVISITOR_H
#define EQ_LAYOUTVISITOR_H

#include <eq/client/viewVisitor.h> // base class

namespace eq
{
    class Layout;

    /** A visitor to traverse layouts and children. */
    class LayoutVisitor : public ViewVisitor
    {
    public:
        /** Constructs a new LayoutVisitor. */
        LayoutVisitor(){}
        
        /** Destruct the LayoutVisitor */
        virtual ~LayoutVisitor(){}

        /** Visit a layout on the down traversal. */
        virtual VisitorResult visitPre( Layout* layout )
            { return visitPre( static_cast< const Layout* >( layout )); }

        /** Visit a layout on the up traversal. */
        virtual VisitorResult visitPost( Layout* layout )
            { return visitPost( static_cast< const Layout* >( layout )); }

        /** Visit a layout on the down traversal. */
        virtual VisitorResult visitPre( const Layout* layout )
            { return TRAVERSE_CONTINUE; }

        /** Visit a layout on the up traversal. */
        virtual VisitorResult visitPost( const Layout* layout )
            { return TRAVERSE_CONTINUE; }
    };
}
#endif // EQ_LAYOUTVISITOR_H
