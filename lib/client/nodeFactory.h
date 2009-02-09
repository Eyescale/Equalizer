
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_NODEFACTORY_H
#define EQ_NODEFACTORY_H

#include <eq/client/types.h>
#include <eq/base/base.h>

namespace eq
{
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
        virtual Config* createConfig( ServerPtr parent );

        /** Release a config. */
        virtual void releaseConfig( Config* config );

        /** 
         * Creates a new node.
         * 
         * @return the node.
         */
        virtual Node* createNode( Config* parent );

        /** Release a node. */
        virtual void releaseNode( Node* node );

        /** 
         * Creates a new pipe.
         * 
         * @return the pipe.
         */
        virtual Pipe* createPipe( Node* parent );

        /** Release a pipe. */
        virtual void releasePipe( Pipe* pipe );

        /** 
         * Creates a new window.
         * 
         * @return the window.
         */
        virtual Window* createWindow( Pipe* parent );

        /** Release a window. */
        virtual void releaseWindow( Window* window );

        /** 
         * Creates a new channel.
         * 
         * @return the channel.
         */
        virtual Channel* createChannel( Window* parent );

        /** Release a channel. */
        virtual void releaseChannel( Channel* channel );
        
        virtual ~NodeFactory(){}
    };
}

#endif // EQ_NODEFACTORY_H

