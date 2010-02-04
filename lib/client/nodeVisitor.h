
/* Copyright (c) 2008-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_NODEVISITOR_H
#define EQ_NODEVISITOR_H

#include <eq/client/pipeVisitor.h> // base class

namespace eq
{
    class Node;

    /** A visitor to traverse nodes and children. */
    class NodeVisitor : public PipeVisitor
    {
    public:
        /** Constructs a new NodeVisitor. */
        NodeVisitor(){}
        
        /** Destruct the NodeVisitor */
        virtual ~NodeVisitor(){}

        /** Visit a node on the down traversal. */
        virtual VisitorResult visitPre( Node* node )
            { return visitPre( static_cast< const Node* >( node )); }

        /** Visit a node on the up traversal. */
        virtual VisitorResult visitPost( Node* node )
            { return visitPost( static_cast< const Node* >( node )); }

        /** Visit a node on the down traversal. */
        virtual VisitorResult visitPre( const Node* node )
            { return TRAVERSE_CONTINUE; }

        /** Visit a node on the up traversal. */
        virtual VisitorResult visitPost( const Node* node )
            { return TRAVERSE_CONTINUE; }
    };
}
#endif // EQ_NODEVISITOR_H
