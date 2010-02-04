
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

#ifndef EQ_PIPEVISITOR_H
#define EQ_PIPEVISITOR_H

#include <eq/client/windowVisitor.h> // base class

namespace eq
{
    class Pipe;

    /** A visitor to traverse pipes and children. */
    class PipeVisitor : public WindowVisitor
    {
    public:
        /** Constructs a new PipeVisitor. */
        PipeVisitor(){}
        
        /** Destruct the PipeVisitor */
        virtual ~PipeVisitor(){}

        /** Visit a pipe on the down traversal. */
        virtual VisitorResult visitPre( Pipe* pipe )
            { return visitPre( static_cast< const Pipe* >( pipe )); }

        /** Visit a pipe on the up traversal. */
        virtual VisitorResult visitPost( Pipe* pipe )
            { return visitPost( static_cast< const Pipe* >( pipe )); }

        /** Visit a pipe on the down traversal. */
        virtual VisitorResult visitPre( const Pipe* pipe )
            { return TRAVERSE_CONTINUE; }

        /** Visit a pipe on the up traversal. */
        virtual VisitorResult visitPost( const Pipe* pipe )
            { return TRAVERSE_CONTINUE; }
    };
}
#endif // EQ_PIPEVISITOR_H
