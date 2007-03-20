/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
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

GLXEventThread GLXEventThread::_thread;

GLXEventThread* GLXEventThread::get()
{
    return &_thread;
}

GLXEventThread::GLXEventThread()
        : _running( false ),
          _requestHandler( true )
{
    registerCommand( CMD_GLXEVENTTHREAD_ADD_PIPE,
      eqNet::CommandFunc<GLXEventThread>( this, &GLXEventThread::_cmdAddPipe ));
    registerCommand( CMD_GLXEVENTTHREAD_REMOVE_PIPE, 
   eqNet::CommandFunc<GLXEventThread>( this, &GLXEventThread::_cmdRemovePipe ));
    registerCommand( CMD_GLXEVENTTHREAD_ADD_WINDOW, 
    eqNet::CommandFunc<GLXEventThread>( this, &GLXEventThread::_cmdAddWindow ));
    registerCommand( CMD_GLXEVENTTHREAD_REMOVE_WINDOW,
 eqNet::CommandFunc<GLXEventThread>( this, &GLXEventThread::_cmdRemoveWindow ));
}

bool GLXEventThread::init()
{
    CHECK_THREAD( _eventThread );
    _commandConnection   = new eqNet::PipeConnection();

    if( !_commandConnection->connect( ))
        return false;

    _connections.addConnection( _commandConnection );
    EQINFO << "Initialized glX event thread" << endl;
    return true;
}

void GLXEventThread::exit()
{
    EQINFO << "Exiting glX event thread" << endl;
    CHECK_THREAD( _eventThread );

    _connections.removeConnection( _commandConnection );
    EQASSERT( _commandConnection->getRefCount() == 1 );

    _commandConnection->close();
    _commandConnection = 0;
    Thread::exit();
}

void GLXEventThread::addPipe( Pipe* pipe )
{
    CHECK_NOT_THREAD( _eventThread );

    _startMutex.set();
    if( isStopped( ))
        start();
    _startMutex.unset();

    GLXEventThreadAddPipePacket packet;
    packet.pipe = pipe;
    _commandConnection->send( packet );
}

void GLXEventThread::removePipe( Pipe* pipe )
{
    CHECK_NOT_THREAD( _eventThread );
    GLXEventThreadRemovePipePacket packet;
    packet.pipe      = pipe;
    packet.requestID = _requestHandler.registerRequest();
    _startMutex.set();
    _commandConnection->send( packet );
    
    const bool stop = _requestHandler.waitRequest( packet.requestID );
    if( stop )
        join();
    CHECK_THREAD_RESET( _eventThread );

    _startMutex.unset();
}

void GLXEventThread::addWindow( Window* window )
{
    CHECK_NOT_THREAD( _eventThread );
    EQASSERT( isRunning( ));

    GLXEventThreadAddWindowPacket packet;
    packet.window = window;
    _commandConnection->send( packet );
}

void GLXEventThread::removeWindow( Window* window )
{
    CHECK_NOT_THREAD( _eventThread );
    EQASSERT( isRunning( ));

    GLXEventThreadRemoveWindowPacket packet;
    packet.window    = window;
    packet.requestID = _requestHandler.registerRequest();

    _commandConnection->send( packet );
    _requestHandler.waitRequest( packet.requestID );
}

//===========================================================================
// Event thread methods
//===========================================================================

void* GLXEventThread::run()
{
    CHECK_THREAD( _eventThread );
    EQINFO << "GLXEventThread running" << endl;

    _running = true;
    while( _running )
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
    return 0;
}

void GLXEventThread::_handleEvent()
{
    CHECK_THREAD( _eventThread );
    RefPtr<eqNet::Connection> connection = _connections.getConnection();

    if( connection == _commandConnection )
    {
        _handleCommand();
        return;
    }
    _handleEvent(
        RefPtr_static_cast<eqNet::Connection, X11Connection>( connection ));
}

void GLXEventThread::_handleCommand()
{
    uint64_t size;
    const bool read = _commandConnection->recv( &size, sizeof( size ));
    if( !read ) // Some systems signal data on dead connections.
        return;

    EQASSERT( size );
    EQASSERT( size < 4096 ); // not a hard error, just sanity check

    _receivedCommand.allocate( 0, 0, size );
    size -= sizeof( size );

    char*      ptr     = (char*)_receivedCommand.getPacket() + sizeof(size);
    const bool gotData = _commandConnection->recv( ptr, size );
    EQASSERT( gotData );

    const eqNet::CommandResult result = invokeCommand( _receivedCommand );
    EQASSERT( result == eqNet::COMMAND_HANDLED );
} 

void GLXEventThread::_handleEvent( RefPtr<X11Connection> connection )
{
    CHECK_THREAD( _eventThread );
    Display*    display = connection->getDisplay();
    const Pipe* pipe    = connection->getUserdata();

    while( XPending( display ))
    {
        WindowEvent event;
        XEvent&     xEvent = event.xEvent;
        XNextEvent( display, &xEvent );
        
        XID            drawable = xEvent.xany.window;
        const uint32_t nWindows = pipe->nWindows();

        event.window   = 0;
        for( uint32_t i=0; i<nWindows; ++i )
        {
            if( pipe->getWindow(i)->getXDrawable() == drawable )
            {
                event.window = pipe->getWindow(i);
                break;
            }
        }
        if( !event.window )
        {
            EQWARN << "Can't match window to received X event" << endl;
            continue;
        }

        switch( xEvent.type )
        {
            case Expose:
                if( xEvent.xexpose.count ) // Only report last expose event
                    continue;
                
                event.type = WindowEvent::EXPOSE;
                break;

            case ConfigureNotify:
            {
                // Get window coordinates from X11, the event data is relative
                // to window parent, but we report pvp relative to root window.
                XWindowAttributes windowAttrs;
                
                XGetWindowAttributes( display, drawable, &windowAttrs );
                
                XID child;
                XTranslateCoordinates( display, drawable, 
                                       RootWindowOfScreen( windowAttrs.screen ),
                                       windowAttrs.x, windowAttrs.y,
                                       &event.resize.x, &event.resize.y,
                                       &child );

                event.type = WindowEvent::RESIZE;
                event.resize.w = windowAttrs.width;
                event.resize.h = windowAttrs.height;
                break;
            }

            // TODO: create channel event?
            case MotionNotify:
                event.type = WindowEvent::POINTER_MOTION;
                event.pointerMotion.x = xEvent.xmotion.x;
                event.pointerMotion.y = xEvent.xmotion.y;
                event.pointerMotion.buttons = _getButtonState( xEvent );
                event.pointerMotion.button  = PTR_BUTTON_NONE;

                _computePointerDelta( event );
                break;

            case DestroyNotify:
                event.type = WindowEvent::CLOSE;
                break;

            case ButtonPress:
                event.type = WindowEvent::POINTER_BUTTON_PRESS;
                event.pointerButtonPress.x = xEvent.xbutton.x;
                event.pointerButtonPress.y = xEvent.xbutton.y;
                event.pointerButtonPress.buttons = _getButtonState( xEvent );
                event.pointerButtonPress.button  = _getButtonAction( xEvent );

                _computePointerDelta( event );
                break;
                
            case ButtonRelease:
                event.type = WindowEvent::POINTER_BUTTON_RELEASE;
                event.pointerButtonRelease.x = xEvent.xbutton.x;
                event.pointerButtonRelease.y = xEvent.xbutton.y;
                event.pointerButtonRelease.buttons = _getButtonState( xEvent );
                event.pointerButtonRelease.button  = _getButtonAction( xEvent );

                _computePointerDelta( event );
                break;
            
            case KeyPress:
                event.type = WindowEvent::KEY_PRESS;
                event.keyPress.key = _getKey( xEvent );
                break;
                
            case KeyRelease:
                event.type = WindowEvent::KEY_RELEASE;
                event.keyPress.key = _getKey( xEvent );
                break;
                
            default:
                event.type = WindowEvent::UNHANDLED;
                EQWARN << "Unhandled X event, type " << xEvent.type << endl;
                break;
        }
        event.window->processEvent( event );
    }
}

uint32_t GLXEventThread::_getButtonState( XEvent& event )
{
    const int xState = event.xbutton.state;
    uint32_t   state  = 0;
    
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

uint32_t GLXEventThread::_getButtonAction( XEvent& event )
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


uint32_t GLXEventThread::_getKey( XEvent& event )
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

eqNet::CommandResult GLXEventThread::_cmdAddPipe( eqNet::Command& command )
{
    CHECK_THREAD( _eventThread );
    const GLXEventThreadAddPipePacket* packet = 
        command.getPacket<GLXEventThreadAddPipePacket>();

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

eqNet::CommandResult GLXEventThread::_cmdRemovePipe( eqNet::Command& command )
{
    const GLXEventThreadRemovePipePacket* packet = 
        command.getPacket<GLXEventThreadRemovePipePacket>();

    Pipe*                     pipe          = packet->pipe;
    RefPtr<X11Connection>     x11Connection = pipe->getXEventConnection();
    RefPtr<eqNet::Connection> connection    = 
        RefPtr_static_cast<X11Connection, eqNet::Connection>( x11Connection );
    
    const bool removed = _connections.removeConnection( connection );
    EQASSERT( removed )
    pipe->setXEventConnection( 0 );
    XCloseDisplay( x11Connection->getDisplay( ));

    EQASSERT( x11Connection->getRefCount() == 2 ); // two local ref ptr's

    EQINFO << "Stopped processing events on display "
           << pipe->getXDisplayString() << endl;

    if( _connections.nConnections() > 1 )        
        _requestHandler.serveRequest( packet->requestID, (void*)false );
    else
    {
        _requestHandler.serveRequest( packet->requestID, (void*)true );
        EQINFO << "GLXEventThread finished" << endl;
        EQASSERT( _connections.nConnections() == 1 );

        _running = false;
    }

    return eqNet::COMMAND_HANDLED;    
}

eqNet::CommandResult GLXEventThread::_cmdAddWindow( eqNet::Command& command )
{
    const GLXEventThreadAddWindowPacket* packet = 
        command.getPacket<GLXEventThreadAddWindowPacket>();

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

eqNet::CommandResult GLXEventThread::_cmdRemoveWindow( eqNet::Command& command )
{
    const GLXEventThreadRemoveWindowPacket* packet = 
        command.getPacket<GLXEventThreadRemoveWindowPacket>();

    Window*               window        = packet->window;
    Pipe*                 pipe          = window->getPipe();
    RefPtr<X11Connection> x11Connection = pipe->getXEventConnection();
    Display*              display       = x11Connection->getDisplay();
    XID                   drawable      = window->getXDrawable();

    EQINFO << "Stop collecting events for window  " << window->getName()
           << endl;
    
    XSelectInput( display, drawable, 0l );
    XFlush( display );

    _requestHandler.serveRequest( packet->requestID, 0 );
    return eqNet::COMMAND_HANDLED;    
}
