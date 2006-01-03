
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_PIPE_H
#define EQ_PIPE_H

#include "commands.h"

#include "node.h"

#include <eq/base/thread.h>
#include <eq/net/base.h>
#include <eq/net/object.h>
#include <eq/net/requestQueue.h>

#ifdef GLX
#  include <X11/Xlib.h>
#endif

namespace eq
{
    class Window;

    enum WindowSystem
    {
        WINDOW_SYSTEM_NONE, // must be first
        WINDOW_SYSTEM_CGL,
        WINDOW_SYSTEM_GLX,
        WINDOW_SYSTEM_ALL   // must be last
    };

    class Pipe : public eqNet::Base, public eqNet::Object
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

        /** 
         * Returns the config of this pipe.
         * 
         * @return the config of this pipe. 
         */
        Config* getConfig() const { return (_node ? _node->getConfig() : NULL);}

        /** 
         * Returns the display number of this pipe.
         * 
         * The display number identifies the X server for systems using the
         * X11/GLX window system. It has no meaning on Windows systems.
         *
         * @return the display number of this pipe, or 
         *         <code>EQ_UNDEFINED_UINT</code>.
         */
        uint getDisplay() const { return _display; }

        /** 
         * Returns the screen number of this pipe.
         * 
         * The screen number identifies the X screen for systems using the
         * X11/GLX window system. Normally the screen identifies a graphics
         * adapter. One Windows systems it identifies the graphics adapter.
         *
         * @return the screen number of this pipe, or 
         *         <code>EQ_UNDEFINED_UINT</code>.
         */
        uint getScreen() const { return _screen; }

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
         * Returns the window system currently used by this pipe.
         * 
         * This function determines which of the supported windowing systems is
         * used by this pipe instance. Its return value has to be static for a
         * given instance.
         * 
         * @return the window system currently used by this pipe.
         */
        virtual WindowSystem getWindowSystem() const;

#ifdef GLX
        /** 
         * Set the X11 display connection for this pipe.
         * 
         * This function should only be called from init() or exit().
         *
         * @param display the X11 display connection for this pipe.
         */
        void setXDisplay( Display* display ) { _xDisplay = display; }

        /** 
         * Returns the X11 display connection for this pipe.
         * @return the X11 display connection for this pipe. 
         */
        Display* getXDisplay() const { return _xDisplay; }
#endif

        /**
         * @name Callbacks
         *
         * The callbacks are called by Equalizer during rendering to execute
         * various actions.
         */
        //@{

        /** 
         * Initialises this pipe.
         */
        virtual bool init();
        bool initGLX();
        bool initCGL();

        /** 
         * Exit this pipe.
         */
        virtual void exit();
        void exitGLX();
        void exitCGL();
        //@}

        /** 
         * Push a request to the pipe thread to be handled from there.
         * 
         * @param node the node sending the packet.
         * @param packet the command packet.
         */
        void pushRequest( eqNet::Node* node, const eqNet::Packet* packet )
            { _requestQueue.push( node, packet ); }

    private:
        /** The parent node. */
        friend class Node;
        Node*       _node;

        /** The windows of this pipe. */
        std::vector<Window*>     _windows;

        
        /** The display (GLX) or ignored (Win32). */
        uint _display;

        /** The screen (GLX) or adapter (Win32). */
        uint _screen;

#ifdef GLX
        /** The X11 display connection. */
        Display* _xDisplay;
#endif

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

        /** The command functions. */
        void _cmdCreateWindow( eqNet::Node* node, const eqNet::Packet* packet );
        void _cmdDestroyWindow( eqNet::Node* node, const eqNet::Packet* packet);
        void _cmdInit( eqNet::Node* node, const eqNet::Packet* packet );
        void _reqInit( eqNet::Node* node, const eqNet::Packet* packet );
        void _cmdExit( eqNet::Node* node, const eqNet::Packet* packet );
        void _reqExit( eqNet::Node* node, const eqNet::Packet* packet );
    };
}

#endif // EQ_PIPE_H

