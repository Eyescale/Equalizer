
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_AGLMESSAGEPUMP_H
#define EQ_AGLMESSAGEPUMP_H

#include <eq/client/messagePump.h> // base class

#ifdef AGL
#  define Cursor CGLCursor   // avoid name clash with X11 'Cursor'
#  include <Carbon/Carbon.h>
#  undef Cursor
#endif

namespace eq
{
    /**
     * Processes OS messages on AGL/Carbon.
     */
    class AGLMessagePump : public MessagePump
    {
    public:
        AGLMessagePump();

        /** Wake up dispatchOneEvent(). */
        virtual void postWakeup();

        /** Get and dispatch all pending system events, non-blocking. */
        virtual void dispatchAll();

        /** Get and dispatch at least one pending system event, blocking. */
        virtual void dispatchOne();

        virtual ~AGLMessagePump() {}

    private:
        EventQueueRef _receiverQueue;

        void _initReceiverQueue();
    };
}

#endif //EQ_AGLMESSAGEPUMP_H
