
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_MESSAGEPUMP_H
#define EQ_MESSAGEPUMP_H

#include <eq/base/base.h>

#ifdef Darwin
#  define Cursor CGLCursor   // avoid name clash with X11 'Cursor'
#  include <Carbon/Carbon.h>
#  undef Cursor
#endif

namespace eqBase
{
    /**
     * Implements an OS-agnostic way to process OS messages/events.
     *
     * Not implemented on Unix-like systems, where a (X11) message pump is not
     * needed.
     */
    class MessagePump
    {
    public:
        MessagePump();

        /** Wake up dispatchOneEvent(). */
        void postWakeup();

        /** Get and dispatch all pending system events, non-blocking */
        void dispatchAll();

        /** Get and dispatch one pending system event, blocking */
        void dispatchOne();

    private:

        union
        {
#ifdef WIN32
            /** Thread ID of the receiver. */
            DWORD _win32ThreadID;
#endif
#ifdef Darwin
            EventQueueRef _receiverQueue;
#endif
            char _fillDummy[32];
        };

        void _initReceiverQueue();
    };
}

#endif //EQ_MESSAGEPUMP_H
