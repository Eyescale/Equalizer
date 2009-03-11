
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_SERVERVISITOR_H
#define EQSERVER_SERVERVISITOR_H

#include "configVisitor.h"        // base class

namespace eq
{
namespace server
{
    class Server;

    /**
     * A visitor to traverse non-const servers and children.
     */
    class ServerVisitor : public ConfigVisitor
    {
    public:
        /** Constructs a new ServerVisitor. */
        ServerVisitor(){}
        
        /** Destruct the ServerVisitor */
        virtual ~ServerVisitor(){}

        /** Visit a server on the down traversal. */
        virtual VisitorResult visitPre( Server* server )
            { return TRAVERSE_CONTINUE; }

        /** Visit a server on the up traversal. */
        virtual VisitorResult visitPost( Server* server )
            { return TRAVERSE_CONTINUE; }
    };

    /**
     * A visitor to traverse const servers and children.
     */
    class ConstServerVisitor : public ConstConfigVisitor
    {
    public:
        /** Constructs a new ServerVisitor. */
        ConstServerVisitor(){}
        
        /** Destruct the ServerVisitor */
        virtual ~ConstServerVisitor(){}

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
