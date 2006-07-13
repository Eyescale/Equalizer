/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_EVENTTHREAD_H
#define EQ_EVENTTHREAD_H

#include <eq/client/windowSystem.h>
#include <eq/base/thread.h>

namespace eq
{
    class Pipe;
    class Window;

    /**
     * The per-node event processing thread.
     */
    class EventThread : public eqBase::Thread
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

    private:
        static EventThread* _threads[WINDOW_SYSTEM_ALL];
    };
}

#endif // EQ_EVENTTHREAD_H

