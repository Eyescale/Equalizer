/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
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

    class EQ_EXPORT Window : public eqNet::Object
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
        Config* getConfig() const { return (_pipe ? _pipe->getConfig() : NULL);}
        eqBase::RefPtr<eqNet::Node> getServer() const 
            { return ( _pipe ? _pipe->getServer() : NULL );}

        const std::string& getName() const { return _name; }

        /** 
         * Set the X11 drawable ID for this window.
         * 
         * This function should only be called from configInit() or configExit().
         *
         * @param drawable the X11 drawable ID.
         */
        void setXDrawable( XID drawable );

        /**  @return  the X11 drawable ID. */
        XID getXDrawable() const { return _xDrawable; }
        
        /** 
         * Set the GLX rendering context for this window.
         * 
         * This function should only be called from configInit() or configExit().
         *
         * @param drawable the GLX rendering context.
         */
        void setGLXContext( GLXContext context ) { _glXContext = context; }

        /** @return the GLX rendering context. */
        GLXContext getGLXContext() const { return _glXContext; }

        /** 
         * Set the CGL rendering context for this window.
         * 
         * This function should only be called from configInit() or configExit().
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
         * Set the Win32 window handle for this window.
         * 
         * This function should only be called from configInit() or configExit().
         *
         * @param drawable the window handle.
         */
        void setWGLWindowHandle( HWND handle );

        /** @return the Win32 window handle. */
        HWND getWGLWindowHandle() const { return _wglWindowHandle; }
        
        /** 
         * Set the WGL rendering context for this window.
         * 
         * This function should only be called from configInit() or configExit().
         *
         * @param drawable the WGL rendering context.
         */
        void setWGLContext( HGLRC context ) { _wglContext = context; }

        /** @return the WGL rendering context. */
        HGLRC getWGLContext() const { return _wglContext; }

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
        virtual bool configInit( const uint32_t initID );
        virtual bool configInitGLX();
        virtual bool configInitCGL();
        virtual bool configInitWGL();

        /** 
         * Initialize the OpenGL state for this window.
         * 
         * @param initID the init identifier.
         * @return <code>true</code> if the initialization was successful,
         *         <code>false</code> if not.
         */
        virtual bool configInitGL( const uint32_t initID );

        /** 
         * Exit this window.
         */
        virtual bool configExit();
        virtual void configExitGLX();
        virtual void configExitCGL();
        virtual void configExitWGL();

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
        virtual void makeCurrent() const;

        /** Swap the front and back buffer of the window. */
        virtual void swapBuffers() const;

        /** Finish outstanding rendering requests. */
        virtual void finish() const { glFinish(); }

        /** 
         * Process a received event.
         *
         * This function is called from the event thread. The task of this
         * method is to update the window as necessary, and transform the event
         * into an config event to be send to the application using
         * Config::sendEvent().
         * 
         * @param event the received window system event.
         * @param true when the event was handled, false if not.
         */
        virtual bool processEvent( const WindowEvent& event );
        friend class GLXEventThread;
        friend class WGLEventHandler;
        //*}

        /** @name Error information. */
        //@{
        /** 
         * Set a message why the last operation failed.
         * 
         * The message will be transmitted to the originator of the request, for
         * example to Config::init when set from within the configInit method.
         *
         * @param message the error message.
         */
        void setErrorMessage( const std::string& message ) { _error = message; }
        //@}
    private:
        /** Drawable characteristics of this window */
        DrawableConfig _drawableConfig;

        /** The reason for the last error. */
        std::string            _error;

        /** Window-system specific drawable information. */
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

            struct
            {
                HWND  _wglWindowHandle;
                HGLRC _wglContext;
            };
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
        Channel* _findChannel( const uint32_t id );

        bool _setPixelViewport( const PixelViewport& pvp );
        void _setViewport( const Viewport& vp );

        void _send( eqNet::ObjectPacket& packet )
            { eqNet::Object::send( getServer(), packet ); }

        /* The command functions. */
        eqNet::CommandResult _pushCommand( eqNet::Command& command );
        eqNet::CommandResult _cmdCreateChannel( eqNet::Command& command );
        eqNet::CommandResult _cmdDestroyChannel(eqNet::Command& command );
        eqNet::CommandResult _reqConfigInit( eqNet::Command& command );
        eqNet::CommandResult _reqConfigExit( eqNet::Command& command );
        eqNet::CommandResult _reqBarrier( eqNet::Command& command );
        eqNet::CommandResult _reqFinish( eqNet::Command& command );
        eqNet::CommandResult _reqSwap( eqNet::Command& command );
        eqNet::CommandResult _reqStartFrame( eqNet::Command& command );
        eqNet::CommandResult _reqEndFrame( eqNet::Command& command );
    };
}

#endif // EQ_WINDOW_H

