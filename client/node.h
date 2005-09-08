
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_NODE_H
#define EQ_NODE_H

#include <eq/net/node.h>

namespace eq
{
    class Node : protected eqNet::Node
    {
    public:
        /** 
         * Returns the local node instance.

         * @return the local node instance.
         */
        static Node* getLocalNode() { return _localNode; }

    private:
        static Node* _localNode;

        bool listen( eqBase::RefPtr<eqNet::Connection> connection )
            { return eqNet::Node::listen( connection ); }
        bool stopListening(){ return eqNet::Node::stopListening(); }

        friend bool eq::initLocalNode( int argc, char** argv );
        friend bool eq::exitLocalNode();
    };
}

#endif // EQ_NODE_H

