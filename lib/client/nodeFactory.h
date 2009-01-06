
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
     * The node factory is a per-node singleton used to create and release
     * Equalizer resource instances.
     *
     * The instances have to be subclasses of the corresponding Equalizer
     * classes, and are used to selectively override task methods and store
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

        /** Release a config. */
        virtual void releaseConfig( Config* config ) { delete config; }

        /** 
         * Creates a new node.
         * 
         * @return the node.
         */
        virtual Node* createNode( Config* parent ){ return new Node( parent ); }

        /** Release a node. */
        virtual void releaseNode( Node* node ) { delete node; }

        /** 
         * Creates a new pipe.
         * 
         * @return the pipe.
         */
        virtual Pipe* createPipe( Node* parent ){ return new Pipe( parent ); }

        /** Release a pipe. */
        virtual void releasePipe( Pipe* pipe ) { delete pipe; }

        /** 
         * Creates a new window.
         * 
         * @return the window.
         */
        virtual Window* createWindow( Pipe* parent )
            { return new Window( parent ); }

        /** Release a window. */
        virtual void releaseWindow( Window* window ) { delete window; }

        /** 
         * Creates a new channel.
         * 
         * @return the channel.
         */
        virtual Channel* createChannel( Window* parent )
            { return new Channel( parent ); }

        /** Release a channel. */
        virtual void releaseChannel( Channel* channel ) { delete channel; }
        
        virtual ~NodeFactory(){}
    };
}

#endif // EQ_NODEFACTORY_H

