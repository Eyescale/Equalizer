
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
    class Server;

    /**
     * The node factory is a per-node singleton used to create Equalizer
     * resource instances.
     *
     * The instances have to be subclasses of the corresponding Equalizer
     * classes, and can be used to selectively override task methods and store
     * additional, application-specific data.
     */
    class EQ_EXPORT NodeFactory
    {
    public:
        /** 
         * Creates a new config.
         * 
         * @return the config.
         */
        virtual Config* createConfig( base::RefPtr< Server > parent )
            { return new Config( parent ); }

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
        
        virtual ~NodeFactory(){}
    };
}

#endif // EQ_NODEFACTORY_H

