/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_EVENTTHREAD_H
#define EQ_EVENTTHREAD_H

#include <eq/client/windowEvent.h>
#include <eq/client/windowSystem.h>

#include <eq/base/hash.h>
#include <eq/base/thread.h>

namespace eq
{
    class Pipe;
    class Window;

    /**
     * The per-node event processing thread.
     * TODO: rename to EventHandler since WGL does not use a thread
     */
    class EQ_EXPORT EventThread
    {
    public:
        static EventThread* get( const WindowSystem windowSystem );

        virtual void addPipe( Pipe* pipe ) = 0;
        virtual void removePipe( Pipe* pipe ) = 0;

        virtual void addWindow( Window* window ) = 0;
        virtual void removeWindow( Window* window ) = 0;

    protected:
        /** Constructs a new event thread. */
        EventThread(){}

        /** Destructs the event thread. */
        virtual ~EventThread(){}

        /** Compute the mouse move delta from the previous pointer event. */
        void _computePointerDelta( WindowEvent &event );

        /** The previous pointer event to compute mouse movement deltas. */
        WindowEvent _lastPointerEvent;
    private:
        static EventThread* _handlers[WINDOW_SYSTEM_ALL];
    };
}

#endif // EQ_EVENTTHREAD_H

