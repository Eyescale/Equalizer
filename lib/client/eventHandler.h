
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_EVENTHANDLER_H
#define EQ_EVENTHANDLER_H

#include <eq/client/event.h>         // member
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

