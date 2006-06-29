/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "glXEventThread.h"

#include <eq/client/X11Connection.h>
#include <eq/client/commands.h>
#include <eq/client/packets.h>
#include <eq/client/pipe.h>
#include <eq/client/window.h>
#include <eq/client/windowEvent.h>
#include <eq/net/pipeConnection.h>

using namespace eq;
using namespace eqBase;
using namespace std;

GLXEventThread::GLXEventThread()
        : eqNet::Base( CMD_GLXEVENTTHREAD_ALL, true )
{
    registerCommand( CMD_GLXEVENTTHREAD_ADD_PIPE, this, 
                     reinterpret_cast<CommandFcn>(
                         &eq::GLXEventThread::_cmdAddPipe ));
    registerCommand( CMD_GLXEVENTTHREAD_REMOVE_PIPE, this, 
                     reinterpret_cast<CommandFcn>(
                         &eq::GLXEventThread::_cmdRemovePipe ));
    registerCommand( CMD_GLXEVENTTHREAD_ADD_WINDOW, this, 
                     reinterpret_cast<CommandFcn>(
                         &eq::GLXEventThread::_cmdAddWindow ));
    registerCommand( CMD_GLXEVENTTHREAD_REMOVE_WINDOW, this, 
                     reinterpret_cast<CommandFcn>(
                         &eq::GLXEventThread::_cmdRemoveWindow ));
    CHECK_THREAD_INIT( _threadID );
}

bool GLXEventThread::init()
{
    CHECK_THREAD( _threadID );
    eqNet::PipeConnection*    pipeConnection = new eqNet::PipeConnection();
    RefPtr<eqNet::Connection> connection     = pipeConnection;

    if( !connection->connect( ))
        return false;

    _commandConnection[EVENT_END] = connection;
    _commandConnection[APP_END]   = pipeConnection->getChildEnd();

    _connections.addConnection( _commandConnection[EVENT_END] );
    return true;
}

void GLXEventThread::exit()
{
    CHECK_THREAD( _threadID );
    _commandConnection[EVENT_END]->close();
    _commandConnection[APP_END]->close();
    
    _commandConnection[EVENT_END] = NULL;
    _commandConnection[APP_END]   = NULL;
    EventThread::exit();
}

void GLXEventThread::addPipe( Pipe* pipe )
{
    CHECK_NOT_THREAD( _threadID );
    if( isStopped( ))
    {
        _localNode = eqNet::Node::getLocalNode();
        start();
    }

    GLXEventThreadAddPipePacket packet;
    packet.pipe = pipe;
    _commandConnection[APP_END]->send( packet );
}

void GLXEventThread::removePipe( Pipe* pipe )
{
    CHECK_NOT_THREAD( _threadID );
    GLXEventThreadRemovePipePacket packet;
    packet.pipe      = pipe;
    packet.requestID = _requestHandler.registerRequest();
    _commandConnection[APP_END]->send( packet );
    
    const bool stop = _requestHandler.waitRequest( packet.requestID );
    if( stop )
    {
        join();
        _localNode = NULL;
    }
}

void GLXEventThread::addWindow( Window* window )
{
    CHECK_NOT_THREAD( _threadID );
    EQASSERT( isRunning( ));

    GLXEventThreadAddWindowPacket packet;
    packet.window = window;
    _commandConnection[APP_END]->send( packet );
}

void GLXEventThread::removeWindow( Window* window )
{
    CHECK_NOT_THREAD( _threadID );
    EQASSERT( isRunning( ));

    GLXEventThreadRemoveWindowPacket packet;
    packet.window    = window;
    packet.requestID = _requestHandler.registerRequest();

    _commandConnection[APP_END]->send( packet );
    _requestHandler.waitRequest( packet.requestID );
}

//===========================================================================
// Event thread methods
//===========================================================================

ssize_t GLXEventThread::run()
{
    CHECK_THREAD( _threadID );
    EQINFO << "GLXEventThread running" << endl;

    eqNet::Node::setLocalNode( _localNode );
    while( true )
    {
        const eqNet::ConnectionSet::Event event = _connections.select( );
        switch( event )
        {
            case eqNet::ConnectionSet::EVENT_CONNECT:
            case eqNet::ConnectionSet::EVENT_TIMEOUT:   
                EQUNREACHABLE;
                break;

            case eqNet::ConnectionSet::EVENT_DISCONNECT:
            {
                RefPtr<eqNet::Connection> connection = 
                    _connections.getConnection();
                _connections.removeConnection( connection );
                EQERROR << "Display connection shut down" << endl;
                break;
            }
            
            case eqNet::ConnectionSet::EVENT_DATA:      
                _handleEvent();
                break;
                
            case eqNet::ConnectionSet::EVENT_ERROR:      
                EQWARN << "Error during select" << endl;
                break;
                
            default:
                EQUNIMPLEMENTED;
        }
    }
}

void GLXEventThread::_handleEvent()
{
    CHECK_THREAD( _threadID );
    RefPtr<eqNet::Connection> connection = _connections.getConnection();

    if( connection == _commandConnection[EVENT_END] )
    {
        _handleCommand();
        return;
    }
    _handleEvent( RefPtr_static_cast<X11Connection, eqNet::Connection>(
                      connection ));
}

void GLXEventThread::_handleCommand()
{
    uint64_t size;
    const uint64_t read = _commandConnection[EVENT_END]->recv( &size, 
                                                               sizeof( size ));
    if( read == 0 ) // Some systems signal data on dead connections.
        return;

    EQASSERT( read == sizeof( size ));
    EQASSERT( size );
    EQASSERT( size < 4096 );

    eqNet::Packet* packet = (eqNet::Packet*)alloca( size );
    packet->size   = size;
    size -= sizeof( size );

    char*      ptr     = (char*)packet + sizeof(size);
    const bool gotData = _commandConnection[EVENT_END]->recv( ptr, size );
    EQASSERT( gotData );

    const eqNet::CommandResult result = handleCommand( NULL, packet );
    EQASSERT( result == eqNet::COMMAND_HANDLED );
} 

void GLXEventThread::_handleEvent( RefPtr<X11Connection> connection )
{
    CHECK_THREAD( _threadID );
    Display*    display = connection->getDisplay();
    const Pipe* pipe    = connection->getUserdata();

    while( XPending( display ))
    {
        WindowEvent event;
        XEvent&     xEvent = event.xEvent;
        XNextEvent( display, &xEvent );
        
        XID            drawable = xEvent.xany.window;
        Window*        window   = NULL;
        const uint32_t nWindows = pipe->nWindows();

        for( uint32_t i=0; i<nWindows; ++i )
        {
            if( pipe->getWindow(i)->getXDrawable() == drawable )
            {
                window = pipe->getWindow(i);
                break;
            }
        }
        if( !window )
        {
            EQWARN << "Can't match window to received X event" << endl;
            continue;
        }

        switch( xEvent.type )
        {
            case Expose:
                if( xEvent.xexpose.count ) // Only report last expose event
                    continue;
                
                event.type = WindowEvent::TYPE_EXPOSE;
                break;

            case ConfigureNotify:
            {
                // Get window coordinates from X11, the event data is relative
                // to window parent, but we need pvp relative to root window.
                XWindowAttributes windowAttrs;
                
                XGetWindowAttributes( display, drawable, &windowAttrs );
                
                XID child;
                XTranslateCoordinates( display, drawable, 
                                       RootWindowOfScreen( windowAttrs.screen ),
                                       windowAttrs.x, windowAttrs.y,
                                       &event.resize.x, &event.resize.y,
                                       &child );

                event.type = WindowEvent::TYPE_RESIZE;
                event.resize.w = windowAttrs.width;
                event.resize.h = windowAttrs.height;
                break;
            }

            // TODO: identify mouse button, compute delta since last event,
            // create channel event
            case MotionNotify:
                event.type = WindowEvent::TYPE_MOUSE_MOTION;
                event.mouseMotion.x = xEvent.xmotion.x;
                event.mouseMotion.y = xEvent.xmotion.y;
                break;

            case ButtonPress:
                event.type = WindowEvent::TYPE_MOUSE_BUTTON_PRESS;
                event.mouseButtonPress.x = xEvent.xbutton.x;
                event.mouseButtonPress.y = xEvent.xbutton.y;
                break;
                
            case ButtonRelease:
                event.type = WindowEvent::TYPE_MOUSE_BUTTON_RELEASE;
                event.mouseButtonPress.x = xEvent.xbutton.x;
                event.mouseButtonPress.y = xEvent.xbutton.y;
                break;
            
            case KeyPress:
                event.type = WindowEvent::TYPE_KEY_PRESS;
                event.keyPress.key = XKeycodeToKeysym( display, 
                                                       xEvent.xkey.keycode, 0 );
                break;
                
            case KeyRelease:
                event.type = WindowEvent::TYPE_KEY_RELEASE;
                event.keyPress.key = XKeycodeToKeysym( display, 
                                                       xEvent.xkey.keycode, 0 );
                break;
                
            default:
                EQWARN << "Unhandled X event" << endl;
                continue;
        }
        window->processEvent( event );
    }
}

eqNet::CommandResult GLXEventThread::_cmdAddPipe( eqNet::Node*,
                                                  const eqNet::Packet* pkg )
{
    CHECK_THREAD( _threadID );
    GLXEventThreadAddPipePacket* packet = (GLXEventThreadAddPipePacket*)pkg;

    Pipe*        pipe        = packet->pipe;
    const string displayName = pipe->getXDisplayString();
    Display*     display     = XOpenDisplay( displayName.c_str( ));

    if( !display )
    {
        EQERROR << "Can't open display: " << XDisplayName( displayName.c_str( ))
                << endl;
        return eqNet::COMMAND_HANDLED;
    }
    EQINFO << "Start processing events on display " << displayName << endl;

    X11Connection*            x11Connection = new X11Connection( display );
    RefPtr<eqNet::Connection> connection    = x11Connection;

    x11Connection->setUserdata( pipe );
    pipe->setXEventConnection( x11Connection );
    _connections.addConnection( connection );

    return eqNet::COMMAND_HANDLED;    
}

eqNet::CommandResult GLXEventThread::_cmdRemovePipe( eqNet::Node*,
                                                     const eqNet::Packet* pkg )
{
    GLXEventThreadRemovePipePacket* packet = 
        (GLXEventThreadRemovePipePacket*)pkg;

    Pipe*                     pipe          = packet->pipe;
    RefPtr<X11Connection>     x11Connection = pipe->getXEventConnection();
    
    _connections.removeConnection(RefPtr_static_cast<eqNet::Connection,
                                  X11Connection>( x11Connection ));
    pipe->setXEventConnection( NULL );
    XCloseDisplay( x11Connection->getDisplay( ));

    EQINFO << "Stopped processing events on display "
           << pipe->getXDisplayString() << endl;

    if( _connections.nConnections() > 1 )        
        _requestHandler.serveRequest( packet->requestID, (void*)false );
    else
    {
        _requestHandler.serveRequest( packet->requestID, (void*)true );
        EQINFO << "GLXEventThread exiting" << endl;
        exit();
    }

    return eqNet::COMMAND_HANDLED;    
}

eqNet::CommandResult GLXEventThread::_cmdAddWindow( eqNet::Node*,
                                                  const eqNet::Packet* pkg )
{
    GLXEventThreadAddWindowPacket* packet = (GLXEventThreadAddWindowPacket*)pkg;

    Window*               window        = packet->window;
    Pipe*                 pipe          = window->getPipe();
    RefPtr<X11Connection> x11Connection = pipe->getXEventConnection();
    Display*              display       = x11Connection->getDisplay();
    XID                   drawable      = window->getXDrawable();
    long                  eventMask     = StructureNotifyMask | ExposureMask |
        KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonPressMask | 
        ButtonReleaseMask;

    EQINFO << "Start collecting events for window @" << (void*)window << "('"
           << window->getName() << "')" << endl;
    
    XSelectInput( display, drawable, eventMask );
    XFlush( display );

    return eqNet::COMMAND_HANDLED;    
}

eqNet::CommandResult GLXEventThread::_cmdRemoveWindow( eqNet::Node*,
                                                     const eqNet::Packet* pkg )
{
    GLXEventThreadRemoveWindowPacket* packet = 
        (GLXEventThreadRemoveWindowPacket*)pkg;

    Window*               window        = packet->window;
    Pipe*                 pipe          = window->getPipe();
    RefPtr<X11Connection> x11Connection = pipe->getXEventConnection();
    Display*              display       = x11Connection->getDisplay();
    XID                   drawable      = window->getXDrawable();

    EQINFO << "Stop collecting events for window  " << window->getName()
           << endl;
    
    XSelectInput( display, drawable, 0l );
    XFlush( display );

    _requestHandler.serveRequest( packet->requestID, NULL );
    return eqNet::COMMAND_HANDLED;    
}
