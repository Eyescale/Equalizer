/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WINDOW_H
#define EQ_WINDOW_H

#include "commands.h"
#include "pipe.h"
#include "pixelViewport.h"

#include <eq/net/base.h>
#include <eq/net/object.h>

#ifdef GLX
#  include <GL/glx.h>
#endif
#ifdef CGL
#  include <OpenGL/OpenGL.h>
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

#ifdef GLX
        /** 
         * Set the X11 drawable ID for this window.
         * 
         * This function should only be called from init() or exit().
         *
         * @param drawable the X11 drawable ID.
         */
        void setXDrawable( XID drawable );

        /** 
         * Returns the X11 drawable ID. 
         * @return  the X11 drawable ID. 
         */
        XID getXDrawable() const { return _xDrawable; }
        
        /** 
         * Set the GLX rendering context for this window.
         * 
         * This function should only be called from init() or exit().
         *
         * @param drawable the GLX rendering context.
         */
        void setGLXContext( GLXContext context ) { _glXContext = context; }

        /** 
         * Returns the GLX rendering context.
         * @return the GLX rendering context. 
         */
        GLXContext getGLXContext() const { return _glXContext; }
#endif
#ifdef CGL
        /** 
         * Set the CGL rendering context for this window.
         * 
         * This function should only be called from init() or exit().
         *
         * @param drawable the CGL rendering context.
         */
        void setCGLContext( CGLContextObj context );

        /** 
         * Returns the CGL rendering context.
         * @return the CGL rendering context. 
         */
        CGLContextObj getCGLContext() const { return _cglContext; }
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
        bool initGLX();
        bool initCGL();

        /** 
         * Exit this window.
         */
        virtual void exit();
        void exitGLX();
        void exitCGL();
        //@}

    private:
#ifdef GLX
        /** The drawable ID of the window. */
        XID        _xDrawable;
        /** The glX rendering context. */
        GLXContext _glXContext;
#endif
#ifdef CGL
        /** The CGL context. */
        CGLContextObj _cglContext;
#endif

        /** The parent node. */
        friend class Pipe;
        Pipe*        _pipe;

        /** The channels of this window. */
        std::vector<Channel*>     _channels;

        /** The pixel viewport wrt the pipe. */
        eq::PixelViewport _pvp;

        void _addChannel( Channel* channel );
        void _removeChannel( Channel* channel );

        /* The command functions. */
        eqNet::CommandResult _pushRequest( eqNet::Node* node,
                                           const eqNet::Packet* packet );
        eqNet::CommandResult _cmdCreateChannel( eqNet::Node* node,
                                                const eqNet::Packet* packet);
        eqNet::CommandResult _cmdDestroyChannel(eqNet::Node* node,
                                                const eqNet::Packet* packet);
        eqNet::CommandResult _reqInit( eqNet::Node* node,
                                       const eqNet::Packet* packet );
        eqNet::CommandResult _reqExit( eqNet::Node* node,
                                       const eqNet::Packet* packet );
    };
}

#endif // EQ_WINDOW_H

