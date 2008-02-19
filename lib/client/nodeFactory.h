
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_NODEFACTORY_H
#define EQ_NODEFACTORY_H

#include <eq/base/base.h>
#include <eq/client/channel.h>
#include <eq/client/config.h>
#include <eq/client/node.h>
#include <eq/client/pipe.h>
#include <eq/client/window.h>

namespace eq
{
    class NodeFactory;
    class Server;

    class EQ_EXPORT NodeFactory
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
        virtual Config* createConfig( eqBase::RefPtr< Server > parent )
            { return new eq::Config( parent ); }

        /** 
         * Creates a new node.
         * 
         * @return the node.
         */
        virtual Node* createNode( Config* parent ){ return new Node( parent ); }

        /** 
         * Creates a new pipe.
         * 
         * @return the pipe.
         */
        virtual Pipe* createPipe( Node* parent ){ return new Pipe( parent ); }

        /** 
         * Creates a new window.
         * 
         * @return the window.
         */
        virtual Window* createWindow( Pipe* parent )
            { return new Window( parent ); }

        /** 
         * Creates a new channel.
         * 
         * @return the channel.
         */
        virtual Channel* createChannel( Window* parent )
            { return new Channel( parent ); }
        //@}
        
        virtual ~NodeFactory(){}
    };
}

#endif // EQ_NODEFACTORY_H

