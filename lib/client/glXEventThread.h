/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_GLXEVENTTHREAD_H
#define EQ_GLXEVENTTHREAD_H

#include <eq/client/eventHandler.h>

#include <eq/client/event.h>
#include <eq/net/command.h>
#include <eq/net/connectionSet.h>

namespace eq
{
    class X11Connection;

    /**
     * The per-node event processing thread for glx pipes.
     */
    class GLXEventThread : public EventHandler, public eqBase::Thread, 
                           public eqNet::Base
    {
    public:
        static GLXEventThread* get();

        /** @sa eqBase::Thread::init. */
        virtual bool init();

        /** @sa eqBase::Thread::exit. */
        virtual void exit();

        /** @sa eqBase::Thread::run. */
        virtual void* run();

        /** @sa EventHandler::registerPipe. */
        void registerPipe( Pipe* pipe );

        /** @sa EventHandler::deregisterPipe. */
        virtual void deregisterPipe( Pipe* pipe );

        /** @sa EventHandler::registerWindow. */
        void registerWindow( Window* window );
        /** @sa EventHandler::deregisterWindow. */
        virtual void deregisterWindow( Window* window );

    private:
        static GLXEventThread _thread;

        eqBase::Lock _startMutex;
        bool         _running;

        eqNet::ConnectionSet        _connections;
        eqBase::RefPtr<eqNet::Node> _localNode;

        /** Registers request packets waiting for a return value. */
        eqBase::RequestHandler _requestHandler;

        /** The cache to store the last received command, stored for reuse */
        eqNet::Command _receivedCommand;

        /** Application->Event thread command connection. */
        eqBase::RefPtr<eqNet::Connection> _commandConnection;

        /** Constructs a new glX event thread. */
        GLXEventThread();

        /** Destructs the glX event thread. */
        virtual ~GLXEventThread(){}
        
        void _handleEvent();
        void   _handleCommand();
        void   _handleEvent( eqBase::RefPtr<X11Connection> connection );
        uint32_t  _getButtonState( XEvent& event );
        uint32_t  _getButtonAction( XEvent& event );
        uint32_t  _getKey( XEvent& event );

        /** The command functions. */
        eqNet::CommandResult _cmdRegisterPipe( eqNet::Command& command );
        eqNet::CommandResult _cmdDeregisterPipe( eqNet::Command& command );
        eqNet::CommandResult _cmdRegisterWindow( eqNet::Command& command );
        eqNet::CommandResult _cmdDeregisterWindow( eqNet::Command& command );

        CHECK_THREAD_DECLARE( _eventThread );
    };
}

#endif // EQ_GLXEVENTTHREAD_H

