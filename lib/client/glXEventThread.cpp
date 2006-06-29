/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "glXEventThread.h"

#include "X11Connection.h"
#include "commands.h"
#include "event.h"
#include "packets.h"
#include "pipe.h"
#include "window.h"
#include "windowEvent.h"
#include <eq/net/pipeConnection.h>

#include <X11/keysym.h>

using namespace eq;
using namespace eqBase;
using namespace std;

GLXEventThread::GLXEventThread()
        : eqNet::Base( CMD_GLXEVENTTHREAD_ALL, true ),
          _lastPointerWindow( NULL )
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

            // TODO: compute delta since last event,
            // create channel event
            case MotionNotify:
                event.type = WindowEvent::TYPE_POINTER_MOTION;
                event.pointerMotion.x = xEvent.xmotion.x;
                event.pointerMotion.y = xEvent.xmotion.y;
                event.pointerButtonRelease.buttons = _getButtonState( xEvent );
                event.pointerButtonRelease.button  = PTR_BUTTON_NONE;
                if( _lastPointerWindow == window )
                    _computePointerDelta( event.pointerMotion );
                _lastPointerEvent  = event.pointerMotion;
                _lastPointerWindow = window;
                break;

            case ButtonPress:
                event.type = WindowEvent::TYPE_POINTER_BUTTON_PRESS;
                event.pointerButtonPress.x = xEvent.xbutton.x;
                event.pointerButtonPress.y = xEvent.xbutton.y;
                event.pointerButtonPress.buttons = _getButtonState( xEvent );
                event.pointerButtonPress.button  = _getButtonAction( xEvent );

                if( _lastPointerWindow == window )
                {   // Reuse dx, dy from last (MotionNotify) event
                    event.pointerButtonRelease.dx = _lastPointerEvent.dx;
                    event.pointerButtonRelease.dy = _lastPointerEvent.dy;
                }
                _lastPointerEvent  = event.pointerButtonPress;
                _lastPointerWindow = window;
                break;
                
            case ButtonRelease:
                event.type = WindowEvent::TYPE_POINTER_BUTTON_RELEASE;
                event.pointerButtonRelease.x = xEvent.xbutton.x;
                event.pointerButtonRelease.y = xEvent.xbutton.y;
                event.pointerButtonRelease.buttons = _getButtonState( xEvent );
                event.pointerButtonRelease.button  = _getButtonAction( xEvent );

                if( _lastPointerWindow == window )
                {   // Reuse dx, dy from last (MotionNotify) event
                    event.pointerButtonRelease.dx = _lastPointerEvent.dx;
                    event.pointerButtonRelease.dy = _lastPointerEvent.dy;
                }
                _lastPointerEvent  = event.pointerButtonRelease;
                _lastPointerWindow = window;
                break;
            
            case KeyPress:
                event.type = WindowEvent::TYPE_KEY_PRESS;
                event.keyPress.key = _getKey( xEvent );
                break;
                
            case KeyRelease:
                event.type = WindowEvent::TYPE_KEY_RELEASE;
                event.keyPress.key = _getKey( xEvent );
                break;
                
            default:
                EQWARN << "Unhandled X event" << endl;
                continue;
        }
        window->processEvent( event );
    }
}

int32_t GLXEventThread::_getButtonState( XEvent& event )
{
    const int xState = event.xbutton.state;
    int32_t   state  = 0;
    
    if( xState & Button1Mask ) state |= PTR_BUTTON1;
    if( xState & Button2Mask ) state |= PTR_BUTTON2;
    if( xState & Button3Mask ) state |= PTR_BUTTON3;
    if( xState & Button4Mask ) state |= PTR_BUTTON4;
    if( xState & Button5Mask ) state |= PTR_BUTTON5;
    
    switch( event.type )
    {   // state is state before event
        case ButtonPress:
            state |= _getButtonAction( event );
            break;

        case ButtonRelease:
            state &= ~_getButtonAction( event );
            break;

        default:
            break;
    }
    return state;
}

int32_t GLXEventThread::_getButtonAction( XEvent& event )
{
    switch( event.xbutton.button )
    {    
        case Button1: return PTR_BUTTON1;
        case Button2: return PTR_BUTTON2;
        case Button3: return PTR_BUTTON3;
        case Button4: return PTR_BUTTON4;
        case Button5: return PTR_BUTTON5;
        default: return PTR_BUTTON_NONE;
    }
}

void GLXEventThread::_computePointerDelta( PointerEvent &event )
{
    event.dx = event.x - _lastPointerEvent.x;
    event.dy = event.y - _lastPointerEvent.y;
}

int32_t GLXEventThread::_getKey( XEvent& event )
{
    const KeySym key = XKeycodeToKeysym( event.xany.display, 
                                         event.xkey.keycode, 0);
    switch( key )
    {
        case XK_Escape:    return KC_ESCAPE;    
        case XK_BackSpace: return KC_BACKSPACE; 
        case XK_Return:    return KC_RETURN;    
        case XK_Tab:       return KC_TAB;       
        case XK_Home:      return KC_HOME;       
        case XK_Left:      return KC_LEFT;       
        case XK_Up:        return KC_UP;         
        case XK_Right:     return KC_RIGHT;      
        case XK_Down:      return KC_DOWN;       
        case XK_Page_Up:   return KC_PAGE_UP;    
        case XK_Page_Down: return KC_PAGE_DOWN;  
        case XK_End:       return KC_END;        
        case XK_F1:        return KC_F1;         
        case XK_F2:        return KC_F2;         
        case XK_F3:        return KC_F3;         
        case XK_F4:        return KC_F4;         
        case XK_F5:        return KC_F5;         
        case XK_F6:        return KC_F6;         
        case XK_F7:        return KC_F7;         
        case XK_F8:        return KC_F8;         
        case XK_F9:        return KC_F9;         
        case XK_F10:       return KC_F10;        
        case XK_F11:       return KC_F11;        
        case XK_F12:       return KC_F12;        
        case XK_F13:       return KC_F13;        
        case XK_F14:       return KC_F14;        
        case XK_F15:       return KC_F15;        
        case XK_F16:       return KC_F16;        
        case XK_F17:       return KC_F17;        
        case XK_F18:       return KC_F18;        
        case XK_F19:       return KC_F19;        
        case XK_F20:       return KC_F20;        
        case XK_Shift_L:   return KC_SHIFT_L;    
        case XK_Shift_R:   return KC_SHIFT_R;    
        case XK_Control_L: return KC_CONTROL_L;  
        case XK_Control_R: return KC_CONTROL_R;  
        case XK_Alt_L:     return KC_ALT_L;      
        case XK_Alt_R:     return KC_ALT_R;
            
        default: 
            // 'Useful' Latin1 characters
            if( (key >= XK_space && key <= XK_asciitilde ) ||
                (key >= XK_nobreakspace && key <= XK_ydiaeresis))
                return key;

            EQWARN << "Unrecognized X11 key code " << key << endl;
            return KC_VOID;
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

    if( _lastPointerWindow == window )
        _lastPointerWindow = NULL;

    _requestHandler.serveRequest( packet->requestID, NULL );
    return eqNet::COMMAND_HANDLED;    
}
