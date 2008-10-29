/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_EVENTHANDLER_H
#define EQ_EVENTHANDLER_H

#include <eq/client/event.h>
#include <eq/client/windowSystem.h>

#include <eq/base/hash.h>

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
         * Deregister a pipe for event handling.
         * 
         * @param pipe the pipe.
         */
        virtual void deregisterPipe( Pipe* pipe ) = 0;
        
    protected:
        /** Constructs a new event handler. */
        EventHandler() : _lastEventWindow( 0 ) {}

        /** Destructs the event handler. */
        virtual ~EventHandler(){}

        /** Compute the mouse move delta from the previous pointer event. */
        void _computePointerDelta( const Window* window, Event& event );

        /** Find and set the rendering context at the mouse position. */
        void _getRenderContext( const Window* window, Event& event );

        /** The previous pointer event to compute mouse movement deltas. */
        Event _lastPointerEvent;

        /** The window of the previous pointer event. */
        const Window* _lastEventWindow;

    private:
    };
}

#endif // EQ_EVENTHANDLER_H

