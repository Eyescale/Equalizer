/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_EVENTHANDLER_H
#define EQ_EVENTHANDLER_H

#include <eq/client/windowEvent.h>
#include <eq/client/windowSystem.h>

#include <eq/base/hash.h>
#include <eq/base/thread.h>

namespace eq
{
    class Pipe;
    class Window;

    /**
     * Base class for window system-specific event handlers
     */
    class EQ_EXPORT EventHandler
    {
    public:
        /** 
         * Register a pipe for event handling and return the appropriate event
         * handler.
         * 
         * @param pipe the pipe.
         * @return the event handler for the pipe, can be 0.
         */
        static EventHandler* registerPipe( Pipe* pipe );

        /** 
         * Register a window for event handling and return the appropriate event
         * handler.
         * 
         * @param window the window.
         * @return the event handler for the window.
         */
        static EventHandler* registerWindow( Window* window );
        
        /** 
         * Deregister a pipe for event handling.
         * 
         * @param pipe the pipe.
         */
        virtual void deregisterPipe( Pipe* pipe ) = 0;
        
        /** 
         * Deregister a window for event handling.
         * 
         * @param window the window.
         */
        virtual void deregisterWindow( Window* window ) = 0;

    protected:
        /** Constructs a new event thread. */
        EventHandler(){}

        /** Destructs the event thread. */
        virtual ~EventHandler(){}

        /** Compute the mouse move delta from the previous pointer event. */
        void _computePointerDelta( WindowEvent& event );

        /** Find and set the rendering context at the mouse position. */
        void _getRenderContext( WindowEvent& event );

        static bool _processEvent( Window* window, const WindowEvent& event );

        /** The previous pointer event to compute mouse movement deltas. */
        WindowEvent _lastPointerEvent;
    private:
    };
}

#endif // EQ_EVENTHANDLER_H

