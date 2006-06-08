/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_EVENTTHREAD_H
#define EQ_EVENTTHREAD_H

#include <eq/base/thread.h>
#include <eq/net/connectionSet.h>

namespace eq
{
    /**
     * The per-node event processing thread.
     */
    class EventThread : public eqBase::Thread
    {
    public:
        /** 
         * Constructs a new event thread.
         */
        EventThread();

        /**
         * Destructs the event thread.
         */
        virtual ~EventThread();
        
        /** @sa eqBase::Thread::init. */
        virtual bool init();

        /** @sa eqBase::Thread::run. */
        virtual ssize_t run();

    private:
        eqNet::ConnectionSet       _connections;
        eqBase::RefPtr<Connection> _commandConnection;
    };
}

#endif // EQ_EVENTTHREAD_H

