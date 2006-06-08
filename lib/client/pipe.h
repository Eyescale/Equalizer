
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PIPE_H
#define EQ_PIPE_H

#include "commands.h"

#include "node.h"
#include "pixelViewport.h"

#include <eq/base/thread.h>
#include <eq/net/base.h>
#include <eq/net/object.h>
#include <eq/net/requestQueue.h>

#ifdef GLX
#ifdef WIN32
#  include "win32_x11.h"
#  include "win32_glx.h"
#else
#  include <X11/Xlib.h>
#  include <GL/glx.h>
#endif
#endif
#ifdef CGL
#  if defined(i386) // WAR compile error
#    undef Status 
#  endif 
#  define Cursor CGLCursor // avoid name clash with X11 'Cursor'
#  include <ApplicationServices/ApplicationServices.h>
#  include <OpenGL/OpenGL.h>
#  undef Cursor
#endif

#ifndef GLX
typedef void Display;
#endif
#ifndef CGL
typedef int32_t CGDirectDisplayID;
#endif

namespace eq
{
    class Window;

    enum WindowSystem
    {
        WINDOW_SYSTEM_NONE = 0, // must be first
        WINDOW_SYSTEM_GLX,
        WINDOW_SYSTEM_CGL,
        WINDOW_SYSTEM_ALL      // must be last
    };

    class Pipe : public eqNet::Object
    {
    public:
        /** 
         * Constructs a new pipe.
         */
        Pipe();

        /**
         * Destructs the pipe.
         */
        virtual ~Pipe();

        Node* getNode() const { return _node; }
        Config* getConfig() const { return (_node ? _node->getConfig() : NULL);}

        /** 
         * Gets a window.
         * 
         * @param index the window's index. 
         * @return the window.
         */
        Window* getWindow( const uint32_t index ) const
            { return _windows[index]; }

        /** 
         * @return the pipe's pixel viewport
         */
        const PixelViewport& getPixelViewport() const { return _pvp; }

        /** 
         * Returns the display number of this pipe.
         * 
         * The display number identifies the X server for systems using the
         * X11/GLX window system, or the number of the display for the CGL
         * window system. It has no meaning on Windows systems.
         *
         * @return the display number of this pipe, or 
         *         <code>EQ_UNDEFINED_UINT</code>.
         */
        uint32_t getDisplay() const { return _display; }

        /** 
         * Returns the screen number of this pipe.
         * 
         * The screen number identifies the X screen for systems using the
         * X11/GLX window system. Normally the screen identifies a graphics
         * adapter. On Windows systems it identifies the graphics adapter. It
         * has no meaning for CGL window systems.
         *
         * @return the screen number of this pipe, or 
         *         <code>EQ_UNDEFINED_UINT</code>.
         */
        uint32_t getScreen() const { return _screen; }

        /** 
         * Tests wether a particular windowing system is supported by this pipe
         * and all its windows.
         * 
         * @param system the window system to test.
         * @return <code>true</code> if the window system is supported,
         *         <code>false</code> if not.
         */
        virtual bool supportsWindowSystem( const WindowSystem system ) const;

        /** 
         * Return the window system to be used by this pipe.
         * 
         * This function determines which of the supported windowing systems is
         * used by this pipe instance. 
         * 
         * @return the window system currently used by this pipe.
         */
        virtual WindowSystem selectWindowSystem() const;

        /** 
         * Return the window system used by this pipe. 
         * 
         * The return value is quaranteed to be constant for an initialised
         * pipe, that is, the window system is determined usign
         * selectWindowSystem() before the pipe init method is executed.
         * 
         * @return the window system used by this pipe.
         */
        WindowSystem getWindowSystem() const { return _windowSystem; }

        /** 
         * Set the X11 display connection for this pipe.
         * 
         * This function should only be called from init() or exit(). Updates
         * the pixel viewport.
         *
         * @param display the X11 display connection for this pipe.
         */
        void setXDisplay( Display* display );

        /** 
         * Returns the X11 display connection for this pipe.
         * @return the X11 display connection for this pipe. 
         */
        Display* getXDisplay() const { return _xDisplay; }

        /** 
         * Set the CGL display ID for this pipe.
         * 
         * This function should only be called from init() or exit().
         *
         * @param id the CGL display ID for this pipe.
         */
        void setCGLDisplayID( CGDirectDisplayID id );

        /** 
         * Returns the CGL display ID for this pipe.
         * @return the CGL display ID for this pipe.
         */
        CGDirectDisplayID getCGLDisplayID() const { return _cglDisplayID; }

        /**
         * @name Callbacks
         *
         * The callbacks are called by Equalizer during rendering to execute
         * various actions.
         */
        //@{

        /** 
         * Initialises this pipe.
         * 
         * @param initID the init identifier.
         */
        virtual bool init( const uint32_t initID );
        bool initGLX();
        bool initCGL();

        /** 
         * Exit this pipe.
         */
        virtual bool exit();
        void exitGLX();
        void exitCGL();

        /**
         * Start rendering a frame.
         *
         * Called once at the beginning of each frame, to do per-frame updates
         * of pipe-specific data, for example updating the rendering engine.
         *
         * @param frameID the per-frame identifier.
         * @sa Config::beginFrame()
         */
        virtual void startFrame( const uint32_t frameID ) {}

        /**
         * End rendering a frame.
         *
         * Called once at the end of each frame, to do per-frame updates
         * of pipe-specific data, for example updating the rendering engine.
         *
         * @param frameID the per-frame identifier.
         */
        virtual void endFrame( const uint32_t frameID ) {}
        //@}

        /** 
         * Push a request to the pipe thread to be handled from there.
         * 
         * @param node the node sending the packet.
         * @param packet the command packet.
         */
        eqNet::CommandResult pushRequest( eqNet::Node* node, 
                                          const eqNet::Packet* packet )
            {_requestQueue.push( node, packet ); return eqNet::COMMAND_HANDLED;}

    private:
        /** The parent node. */
        friend class Node;
        Node*       _node;

        /** The windows of this pipe. */
        std::vector<Window*>     _windows;

        /** The current window system. */
        WindowSystem _windowSystem;

        /** The size (and location) of the pipe. */
        PixelViewport _pvp;

        union
        {
#ifdef GLX
            /** The X11 display connection. */
            Display* _xDisplay;
#endif
#ifdef CGL
            CGDirectDisplayID _cglDisplayID;
#endif
            char _fill[8];
        };

        /** The display (GLX, CGL) or ignored (Win32). */
        uint32_t _display;

        /** The screen (GLX), adapter (Win32) or ignored (CGL). */
        uint32_t _screen;

        void _addWindow( Window* window );
        void _removeWindow( Window* window );

        /** The pipe thread. */
        class PipeThread : public eqBase::Thread
        {
        public:
            PipeThread( Pipe* pipe ) 
                    : eqBase::Thread( Thread::PTHREAD ),
                      _pipe( pipe )
                {}
            
            virtual ssize_t run(){ return _pipe->_runThread(); }

        private:
            Pipe* _pipe;
        };
        PipeThread* _thread;

        ssize_t _runThread();

        /** The receiver->pipe thread request queue. */
        eqNet::RequestQueue    _requestQueue;

        /* The command functions. */
        eqNet::CommandResult _cmdCreateWindow( eqNet::Node* node,
                                               const eqNet::Packet* packet );
        eqNet::CommandResult _cmdDestroyWindow( eqNet::Node* node,
                                                const eqNet::Packet* packet );
        eqNet::CommandResult _cmdInit( eqNet::Node* node,
                                       const eqNet::Packet* packet );
        eqNet::CommandResult _reqInit( eqNet::Node* node,
                                       const eqNet::Packet* packet );
        eqNet::CommandResult _cmdExit( eqNet::Node* node, 
                                       const eqNet::Packet* packet );
        eqNet::CommandResult _reqExit( eqNet::Node* node,
                                       const eqNet::Packet* packet );
        eqNet::CommandResult _reqUpdate( eqNet::Node* node,
                                         const eqNet::Packet* packet );
        eqNet::CommandResult _reqFrameSync( eqNet::Node* node,
                                            const eqNet::Packet* packet );
    };
}

#endif // EQ_PIPE_H

