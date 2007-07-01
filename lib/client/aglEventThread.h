/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_AGLEVENTTHREAD_H
#define EQ_AGLEVENTTHREAD_H

#include <eq/client/eventHandler.h>

#include <eq/client/event.h>
#include <eq/net/command.h>
#include <eq/net/connectionSet.h>

namespace eq
{
    class X11Connection;

    /**
     * The per-node event processing thread for agl windows.
     */
    class AGLEventThread : public EventHandler, public eqBase::Thread
    {
    public:
        static AGLEventThread* get();

        /** @sa eqBase::Thread::init. */
        virtual bool init();

        /** @sa eqBase::Thread::exit. */
        virtual void exit();

        /** @sa eqBase::Thread::run. */
        virtual void* run();

        /** @sa EventHandler::addWindow. */
        virtual void addWindow( Window* window );
        /** @sa EventHandler::removeWindow. */
        virtual void removeWindow( Window* window );

    private:
        static AGLEventThread _thread;

        eqBase::Lock _startMutex;
        uint32_t     _used;

        /** Constructs a new agl event thread. */
        AGLEventThread();

        /** Destructs the agl event thread. */
        virtual ~AGLEventThread(){}
        
        static pascal OSStatus _handleEventUPP( EventHandlerCallRef nextHandler,
                                                EventRef event, void* userData);
        CHECK_THREAD_DECLARE( _eventThread );
    };
}

#endif // EQ_AGLEVENTTHREAD_H

