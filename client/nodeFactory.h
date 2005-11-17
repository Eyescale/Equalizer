
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_NODEFACTORY_H
#define EQ_NODEFACTORY_H

#include <eq/config.h>
#include <eq/node.h>
#include <eq/base/base.h>

namespace eq
{
    class NodeFactory;
    class Server;

    NodeFactory* createNodeFactory();

    class NodeFactory
    {
    public:
        /**
         * @name Factory methods
         *
         * These methods are used to instanciate the various entities on a node.
         */
        //@{
        /** 
         * Creates a new config.
         * 
         * @return the config.
         */
        virtual Config* createConfig( const uint id, Server* parent)
            { return new eq::Config( id, parent ); }

        /** 
         * Creates a new node.
         * 
         * @return the node.
         */
        virtual Node* createNode(){ return new eq::Node; }
        //@}
        
        virtual ~NodeFactory(){}
    };
}

#endif // EQ_NODEFACTORY_H

