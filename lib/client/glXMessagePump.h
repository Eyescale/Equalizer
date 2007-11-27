
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_GLXMESSAGEPUMP_H
#define EQ_GLXMESSAGEPUMP_H

#include <eq/client/messagePump.h> // base class

#include <eq/net/connectionSet.h>

namespace eq
{
    /**
     * Implements a message pump for the X11 window system.
     */
    class GLXMessagePump : public MessagePump
    {
    public:
        GLXMessagePump();

        /** Wake up dispatchOne(). */
        virtual void postWakeup();

        /** Get and dispatch all pending system events, non-blocking. */
        virtual void dispatchAll();

        /** Get and dispatch at least one pending system event, blocking. */
        virtual void dispatchOne();
        
        virtual ~GLXMessagePump() {}

    private:
        eqNet::ConnectionSet* _wakeupSet;
    };
}

#endif //EQ_GLXMESSAGEPUMP_H
