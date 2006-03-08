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
#ifdef WIN32
#include <stdlib.h>
#include <windows.h>
#include "win32_x11.h"
#include "win32_glx.h"
#else
#  include <GL/glx.h>
#endif
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
         * Set the window's pixel viewport wrt its parent pipe.
         *
         * Updates the fractional viewport accordingly.
         * 
         * @param pvp the viewport in pixels.
         */
        void setPixelViewport( const PixelViewport& pvp );
        
        /** 
         * @return the window's pixel viewport
         */
        const PixelViewport& getPixelViewport() const { return _pvp; }

        /** 
         * Set the window's fractional viewport wrt its parent pipe.
         *
         * Updates the pixel viewport accordingly.
         * 
         * @param vp the fractional viewport.
         */
        void setViewport( const Viewport& vp );

        /** 
         * @return the window's fractional viewport.
         */
        const Viewport& getViewport() const { return _vp; }

        /**
         * @name Callbacks
         *
         * The callbacks are called by Equalizer during rendering to execute
         * various actions.
         */
        //@{

        /** 
         * Initialise this window.
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

        /**
         * Swap the front and back buffer of the window.
         */
        virtual void swap();

        /**
         * Finish outstanding rendering requests.
         */
        virtual void finish() { glFinish(); }
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

        /** The fractional viewport wrt the pipe. */
        eq::Viewport      _vp;

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
        eqNet::CommandResult _reqSwap( eqNet::Node* node,
                                       const eqNet::Packet* packet );
        eqNet::CommandResult _reqSwapWithBarrier( eqNet::Node* node,
                                                  const eqNet::Packet* packet );
    };
}

#endif // EQ_WINDOW_H

