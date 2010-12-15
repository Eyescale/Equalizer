
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

#ifndef EQSERVER_SERVERVISITOR_H
#define EQSERVER_SERVERVISITOR_H

#include "configVisitor.h"        // base class

namespace eq
{
namespace server
{
    class Server;

    /** A visitor to traverse servers and children. */
    class ServerVisitor : public ConfigVisitor
    {
    public:
        /** Constructs a new ServerVisitor. */
        ServerVisitor(){}
        
        /** Destruct the ServerVisitor */
        virtual ~ServerVisitor(){}

        /** Visit a server on the down traversal. */
        virtual VisitorResult visitPre( Server* server )
            { return visitPre( static_cast< const Server* >( server )); }

        /** Visit a server on the up traversal. */
        virtual VisitorResult visitPost( Server* server )
            { return visitPost( static_cast< const Server* >( server )); }

        /** Visit a server on the down traversal. */
        virtual VisitorResult visitPre( const Server* server )
            { return TRAVERSE_CONTINUE; }

        /** Visit a server on the up traversal. */
        virtual VisitorResult visitPost( const Server* server )
            { return TRAVERSE_CONTINUE; }
    };
}
}
#endif // EQSERVER_SERVERVISITOR_H
