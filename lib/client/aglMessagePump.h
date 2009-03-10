
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_AGLMESSAGEPUMP_H
#define EQ_AGLMESSAGEPUMP_H

#include <eq/client/messagePump.h>  // base class
#include <eq/client/windowSystem.h> // EventQueueRef definition

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

        virtual ~AGLMessagePump();

    private:
        EventQueueRef _receiverQueue;
        EventRef      _wakeupEvent;
        const bool    _needGlobalLock;

        void _initReceiverQueue();
    };
}

#endif //EQ_AGLMESSAGEPUMP_H
