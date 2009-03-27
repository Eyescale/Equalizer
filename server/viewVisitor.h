
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

#ifndef EQSERVER_VIEWVISITOR_H
#define EQSERVER_VIEWVISITOR_H

#include "visitorResult.h"  // enum

namespace eq
{
namespace server
{
    class View;

    /**
     * A visitor to traverse non-const views.
     */
    class ViewVisitor
    {
    public:
        /** Constructs a new ViewVisitor. */
        ViewVisitor(){}
        
        /** Destruct the ViewVisitor */
        virtual ~ViewVisitor(){}

        /** Visit a view. */
        virtual VisitorResult visit( View* view )
            { return TRAVERSE_CONTINUE; }
    };

    /**
     * A visitor to traverse const views.
     */
    class ConstViewVisitor
    {
    public:
        /** Constructs a new ViewVisitor. */
        ConstViewVisitor(){}
        
        /** Destruct the ViewVisitor */
        virtual ~ConstViewVisitor(){}

        /** Visit a view. */
        virtual VisitorResult visit( const View* view )
            { return TRAVERSE_CONTINUE; }
    };
}
}
#endif // EQSERVER_VIEWVISITOR_H
