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
    protected:
        /** Constructs a new event thread. */
        EventHandler(){}

        /** Destructs the event thread. */
        virtual ~EventHandler(){}

        /** Compute the mouse move delta from the previous pointer event. */
        void _computePointerDelta( WindowEvent &event );

        /** The previous pointer event to compute mouse movement deltas. */
        WindowEvent _lastPointerEvent;
    private:
    };
}

#endif // EQ_EVENTHANDLER_H

