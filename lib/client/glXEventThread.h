/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_GLXEVENTTHREAD_H
#define EQ_GLXEVENTTHREAD_H

#include "eventThread.h"

#include <eq/net/connectionSet.h>
#include <eq/net/node.h>

namespace eq
{
    class X11Connection;

    /**
     * The per-node event processing thread for glx pipes.
     */
    class GLXEventThread : public EventThread, public eqNet::Base
    {
    public:
        /** Constructs a new glX event thread. */
        GLXEventThread();

        /** Destructs the glX event thread. */
        virtual ~GLXEventThread(){}
        
        /** @sa eqBase::Thread::init. */
        virtual bool init();

        /** @sa eqBase::Thread::exit. */
        virtual void exit();

        /** @sa eqBase::Thread::run. */
        virtual ssize_t run();

        /** @sa EventThread::addPipe. */
        virtual void addPipe( Pipe* pipe );
        /** @sa EventThread::removePipe. */
        virtual void removePipe( Pipe* pipe );

        /** @sa EventThread::addWindow. */
        virtual void addWindow( Window* window );
        /** @sa EventThread::removeWindow. */
        virtual void removeWindow( Window* window );

    private:
        eqNet::ConnectionSet        _connections;
        eqBase::RefPtr<eqNet::Node> _localNode;

        enum
        {
            EVENT_END = 0,
            APP_END   = 1
        };
        eqBase::RefPtr<eqNet::Connection> _commandConnection[2];


        void _handleEvent();
        void   _handleCommand();
        void   _handleEvent( eqBase::RefPtr<X11Connection> connection );
        int      _getKeyFromXEvent( XEvent& event );

        /** The command functions. */
        eqNet::CommandResult _cmdAddPipe( eqNet::Node*,
                                          const eqNet::Packet* packet );
        eqNet::CommandResult _cmdRemovePipe( eqNet::Node*,
                                             const eqNet::Packet* packet );
        eqNet::CommandResult _cmdAddWindow( eqNet::Node*,
                                            const eqNet::Packet* packet );
        eqNet::CommandResult _cmdRemoveWindow( eqNet::Node*,
                                               const eqNet::Packet* packet );

#ifdef CHECK_THREADSAFETY
        pthread_t _threadID;
#endif
    };
}

#endif // EQ_GLXEVENTTHREAD_H

