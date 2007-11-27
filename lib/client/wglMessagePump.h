
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WGLMESSAGEPUMP_H
#define EQ_WGLMESSAGEPUMP_H

#include <eq/client/messagePump.h> // base class

namespace eq
{
    /**
     * Processes OS messages on Win32 systems.
     */
    class WGLMessagePump : public MessagePump
    {
    public:
        WGLMessagePump();

        /** Wake up dispatchOneEvent(). */
        virtual void postWakeup();

        /** Get and dispatch all pending system events, non-blocking. */
        virtual void dispatchAll();

        /** Get and dispatch at least one pending system event, blocking. */
        virtual void dispatchOne();

        virtual ~WGLMessagePump();

    private:
        /** Thread ID of the receiver. */
        DWORD _win32ThreadID;

        void _initReceiverQueue();
    };
}

#endif //EQ_WGLMESSAGEPUMP_H
