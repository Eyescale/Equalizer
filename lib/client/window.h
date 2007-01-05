/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WINDOW_H
#define EQ_WINDOW_H

#include <eq/client/commands.h>
#include <eq/client/pipe.h>
#include <eq/client/pixelViewport.h>
#include <eq/net/base.h>
#include <eq/net/object.h>

#ifdef Darwin
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

namespace eq
{
    class Channel;
    class WindowEvent;

    class Window : public eqNet::Object
    {
    public:
        /** Stores current drawable characteristics. */
        struct DrawableConfig
        {
            bool doublebuffered;
            bool stereo;
        };
        
        /** 
         * Constructs a new window.
         */
        Window();

        /** @name Data Access */
        //*{
        /** 
         * Returns the pipe of this window.
         * 
         * @return the pipe of this window. 
         */
        Pipe* getPipe() const { return _pipe; }
        Node* getNode() const 
            { return ( _pipe ? _pipe->getNode() : NULL );}
        eqBase::RefPtr<eqNet::Node> getServer() const 
            { return ( _pipe ? _pipe->getServer() : NULL );}

        const std::string& getName() const { return _name; }

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
         * @return the window's fractional viewport.
         */
        const Viewport& getViewport() const { return _vp; }
        //*}

        /**
         * @name Callbacks
         *
         * The callbacks are called by Equalizer during rendering to execute
         * various actions.
         */
        //*{

        /** 
         * Initialize this window.
         * 
         * @param initID the init identifier.
         */
        virtual bool init( const uint32_t initID );
        virtual bool initGLX();
        virtual bool initCGL();

        /** 
         * Initialize the OpenGL state for this window.
         * 
         * @param initID the init identifier.
         * @return <code>true</code> if the initialization was successful,
         *         <code>false</code> if not.
         */
        virtual bool initGL( const uint32_t initID );

        /** 
         * Exit this window.
         */
        virtual bool exit();
        void exitGLX();
        void exitCGL();

        /**
         * Start rendering a frame.
         *
         * Called once at the beginning of each frame, to do per-frame updates
         * of window-specific data.
         *
         * @param frameID the per-frame identifier.
         * @sa Config::beginFrame()
         */
        virtual void startFrame( const uint32_t frameID ) {}

        /**
         * End rendering a frame.
         *
         * Called once at the end of each frame, to do per-frame updates
         * of window-specific data.
         *
         * @param frameID the per-frame identifier.
         */
        virtual void endFrame( const uint32_t frameID ) {}

        /** Make the window's drawable and context current. */
        virtual void makeCurrent();

        /** Swap the front and back buffer of the window. */
        virtual void swapBuffers();

        /** Finish outstanding rendering requests. */
        virtual void finish() { glFinish(); }

        /** 
         * Process a received event.
         *
         * This function is called from the event thread. The task of this
         * method is to update the window as necessary, and transform the event
         * into an config event to be send to the application using
         * Config::sendEvent().
         * 
         * @param event the received window system event.
         */
        virtual void processEvent( const WindowEvent& event );
        //*}

        /**
         * @name Attributes
         */
        //*{
        // Note: also update string array initialization in window.cpp
        enum IAttribute
        {
            IATTR_HINT_STEREO,
            IATTR_HINT_DOUBLEBUFFER,
            IATTR_HINT_FULLSCREEN,
            IATTR_HINT_DECORATION,
            IATTR_PLANES_COLOR,
            IATTR_PLANES_ALPHA,
            IATTR_PLANES_DEPTH,
            IATTR_PLANES_STENCIL,
            IATTR_ALL
        };
        
        int32_t  getIAttribute( const IAttribute attr ) const
            { return _iAttributes[attr]; }
        static const std::string&  getIAttributeString( const IAttribute attr )
            { return _iAttributeStrings[attr]; }
        //*}

        const DrawableConfig& getDrawableConfig() const
            { return _drawableConfig; }
    protected:
        /**
         * Destructs the window.
         */
        virtual ~Window();

    private:
        /** Drawable characteristics of this window */
        DrawableConfig _drawableConfig;

        union
        {
            struct
            {
                /** The X11 drawable ID of the window. */
                XID        _xDrawable;
                /** The glX rendering context. */
                GLXContext _glXContext;
            };

            /** The CGL context. */
            CGLContextObj _cglContext;

            char _windowFill[32];
        };

        /** The parent node. */
        friend class Pipe;
        Pipe*        _pipe;

        /** The name. */
        std::string    _name;

        /** Integer attributes. */
        int32_t _iAttributes[IATTR_ALL];
        /** String representation of integer attributes. */
        static std::string _iAttributeStrings[IATTR_ALL];

        /** The channels of this window. */
        std::vector<Channel*>     _channels;

        /** The pixel viewport wrt the pipe. */
        eq::PixelViewport _pvp;

        /** The fractional viewport wrt the pipe. */
        eq::Viewport      _vp;

        void _addChannel( Channel* channel );
        void _removeChannel( Channel* channel );

        bool _setPixelViewport( const PixelViewport& pvp );
        void _setViewport( const Viewport& vp );

        void _send( eqNet::ObjectPacket& packet )
            { eqNet::Object::send( getServer(), packet ); }

        /* The command functions. */
        eqNet::CommandResult _pushCommand( eqNet::Command& command );
        eqNet::CommandResult _cmdCreateChannel( eqNet::Command& command );
        eqNet::CommandResult _cmdDestroyChannel(eqNet::Command& command );
        eqNet::CommandResult _reqInit( eqNet::Command& command );
        eqNet::CommandResult _reqExit( eqNet::Command& command );
        eqNet::CommandResult _reqBarrier( eqNet::Command& command );
        eqNet::CommandResult _reqFinish( eqNet::Command& command );
        eqNet::CommandResult _reqSwap( eqNet::Command& command );
        eqNet::CommandResult _reqStartFrame( eqNet::Command& command );
        eqNet::CommandResult _reqEndFrame( eqNet::Command& command );
    };
}

#endif // EQ_WINDOW_H

