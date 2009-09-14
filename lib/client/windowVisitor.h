
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_WINDOWVISITOR_H
#define EQ_WINDOWVISITOR_H

#include <eq/client/channelVisitor.h> // base class

namespace eq
{
    class Window;

    /**
     * A visitor to traverse non-const windows and children.
     */
    class WindowVisitor : public ChannelVisitor
    {
    public:
        /** Construct a new WindowVisitor. */
        WindowVisitor(){}
        
        /** Destruct the WindowVisitor */
        virtual ~WindowVisitor(){}

        /** Visit a window on the down traversal. */
        virtual VisitorResult visitPre( Window* window )
            { return TRAVERSE_CONTINUE; }

        /** Visit a window on the up traversal. */
        virtual VisitorResult visitPost( Window* window )
            { return TRAVERSE_CONTINUE; }
    };
}
#endif // EQ_WINDOWVISITOR_H
