
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_NODEFACTORY_H
#define EQ_NODEFACTORY_H

#include <eq/base/base.h>
#include <eq/channel.h>
#include <eq/config.h>
#include <eq/node.h>
#include <eq/pipe.h>
#include <eq/window.h>

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
        virtual Config* createConfig()
            { return new eq::Config(); }

        /** 
         * Creates a new node.
         * 
         * @return the node.
         */
        virtual Node* createNode(){ return new eq::Node; }

        /** 
         * Creates a new pipe.
         * 
         * @return the pipe.
         */
        virtual Pipe* createPipe(){ return new eq::Pipe; }

        /** 
         * Creates a new window.
         * 
         * @return the window.
         */
        virtual Window* createWindow(){ return new eq::Window; }

        /** 
         * Creates a new channel.
         * 
         * @return the channel.
         */
        virtual Channel* createChannel(){ return new eq::Channel; }
        //@}
        
        virtual ~NodeFactory(){}
    };
}

#endif // EQ_NODEFACTORY_H

