
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

#ifndef EQSERVER_NODEVISITOR_H
#define EQSERVER_NODEVISITOR_H

#include "pipeVisitor.h"

namespace eq
{
namespace server
{
    class Node;

    /**
     * A visitor to traverse non-const nodes and children.
     */
    class NodeVisitor : public PipeVisitor
    {
    public:
        /** Constructs a new NodeVisitor. */
        NodeVisitor(){}
        
        /** Destruct the NodeVisitor */
        virtual ~NodeVisitor(){}

        /** Visit a node on the down traversal. */
        virtual VisitorResult visitPre( Node* node )
            { return TRAVERSE_CONTINUE; }

        /** Visit a node on the up traversal. */
        virtual VisitorResult visitPost( Node* node )
            { return TRAVERSE_CONTINUE; }
    };

    /**
     * A visitor to traverse const nodes and children.
     */
    class ConstNodeVisitor : public ConstPipeVisitor
    {
    public:
        /** Constructs a new NodeVisitor. */
        ConstNodeVisitor(){}
        
        /** Destruct the NodeVisitor */
        virtual ~ConstNodeVisitor(){}

        /** Visit a node on the down traversal. */
        virtual VisitorResult visitPre( const Node* node )
            { return TRAVERSE_CONTINUE; }

        /** Visit a node on the up traversal. */
        virtual VisitorResult visitPost( const Node* node )
            { return TRAVERSE_CONTINUE; }
    };
}
}
#endif // EQSERVER_NODEVISITOR_H
