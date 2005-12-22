/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WINDOW_H
#define EQ_WINDOW_H

#include "commands.h"
#include "pipe.h"

#include <eq/net/base.h>
#include <eq/net/object.h>

#ifdef X11
#  include <GL/glx.h>
#endif

namespace eq
{
    class Channel;

    class Window : public eqNet::Base, public eqNet::Object
    {
    public:
        /** 
         * Constructs a new window.
         */
        Window();

        /**
         * Destructs the window.
         */
        virtual ~Window();

        /** 
         * Returns the pipe of this window.
         * 
         * @return the pipe of this window. 
         */
        Pipe* getPipe() const { return _pipe; }

#ifdef X11
        /** 
         * Returns the X11 display connection for this window.
         * @return the X11 display connection for this window. 
         */
        Display* getXDisplay() const 
            { return ( _pipe ? _pipe->getXDisplay() : NULL ); }

        /** 
         * Set the X11 drawable ID for this window.
         * 
         * This function should only be called from init() or exit().
         *
         * @param drawable the X11 drawable ID.
         */
        void setXDrawable( XID drawable ) { _xDrawable = drawable; }

        /** 
         * Set the GLX rendering context for this window.
         * 
         * This function should only be called from init() or exit().
         *
         * @param drawable the GLX rendering context.
         */
        void setGLXContext( GLXContext context ) { _glXContext = context; }
#endif

        /** 
         * Returns the config of this window.
         * 
         * @return the config of this window. 
         */
        Config* getConfig() const { return (_pipe ? _pipe->getConfig() : NULL);}

        /**
         * @name Callbacks
         *
         * The callbacks are called by Equalizer during rendering to execute
         * various actions.
         */
        //@{

        /** 
         * Initialises this window.
         */
        virtual bool init();

        /** 
         * Exit this window.
         */
        virtual void exit(){}
        //@}

    private:
        /** The parent node. */
        friend class Pipe;
        Pipe*        _pipe;

        /** The channels of this window. */
        std::vector<Channel*>     _channels;

#ifdef X11
        /** The drawable ID of the window. */
        XID        _xDrawable;
        /** The glX rendering context. */
        GLXContext _glXContext;
#endif


        void _addChannel( Channel* channel );
        void _removeChannel( Channel* channel );

        /** The command functions. */
        void _pushRequest( eqNet::Node* node, const eqNet::Packet* packet );
        void _cmdCreateChannel( eqNet::Node* node, const eqNet::Packet* packet);
        void _cmdDestroyChannel(eqNet::Node* node, const eqNet::Packet* packet);
        void _reqInit( eqNet::Node* node, const eqNet::Packet* packet );
        void _reqExit( eqNet::Node* node, const eqNet::Packet* packet );
    };
}

#endif // EQ_WINDOW_H

