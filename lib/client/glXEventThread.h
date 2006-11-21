/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_GLXEVENTTHREAD_H
#define EQ_GLXEVENTTHREAD_H

#include <eq/client/eventThread.h>

#include <eq/client/event.h>
#include <eq/client/windowEvent.h>
#include <eq/net/command.h>
#include <eq/net/connectionSet.h>

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
        virtual void* run();

        /** @sa EventThread::addPipe. */
        virtual void addPipe( Pipe* pipe );
        /** @sa EventThread::removePipe. */
        virtual void removePipe( Pipe* pipe );

        /** @sa EventThread::addWindow. */
        virtual void addWindow( Window* window );
        /** @sa EventThread::removeWindow. */
        virtual void removeWindow( Window* window );

    private:
        eqBase::Lock _startMutex;

        eqNet::ConnectionSet        _connections;
        eqBase::RefPtr<eqNet::Node> _localNode;

        /** The cache to store the last received command, stored for reuse */
        eqNet::Command _receivedCommand;

        WindowEvent _lastPointerEvent;

        enum
        {
            EVENT_END = 0,
            APP_END   = 1
        };
        eqBase::RefPtr<eqNet::Connection> _commandConnection[2];


        void _handleEvent();
        void   _handleCommand();
        void   _handleEvent( eqBase::RefPtr<X11Connection> connection );
        int32_t  _getButtonState( XEvent& event );
        int32_t  _getButtonAction( XEvent& event );
        void     _computePointerDelta( WindowEvent &event );
        int32_t  _getKey( XEvent& event );

        /** The command functions. */
        eqNet::CommandResult _cmdAddPipe( eqNet::Command& command );
        eqNet::CommandResult _cmdRemovePipe( eqNet::Command& command );
        eqNet::CommandResult _cmdAddWindow( eqNet::Command& command );
        eqNet::CommandResult _cmdRemoveWindow( eqNet::Command& command );

        CHECK_THREAD_DECLARE( _thread );
    };
}

#endif // EQ_GLXEVENTTHREAD_H

