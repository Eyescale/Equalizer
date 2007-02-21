
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "pipe.h"

#include "commands.h"
#include "eventHandler.h"
#include "global.h"
#include "nodeFactory.h"
#include "packets.h"
#include "X11Connection.h"
#include "window.h"

#include <eq/net/command.h>

#include <sstream>

using namespace eq;
using namespace eqBase;
using namespace std;

Pipe::Pipe()
        : _node( 0 ),
          _currentWindow( 0 ),
          _windowSystem( WINDOW_SYSTEM_NONE ),
          _display( EQ_UNDEFINED_UINT32 ),
          _screen( EQ_UNDEFINED_UINT32 )
{
    registerCommand( CMD_PIPE_CREATE_WINDOW,
                   eqNet::CommandFunc<Pipe>( this, &Pipe::_cmdCreateWindow ));
    registerCommand( CMD_PIPE_DESTROY_WINDOW, 
                  eqNet::CommandFunc<Pipe>( this, &Pipe::_cmdDestroyWindow ));
    registerCommand( CMD_PIPE_INIT, 
                     eqNet::CommandFunc<Pipe>( this, &Pipe::_cmdInit ));
    registerCommand( REQ_PIPE_INIT,
                     eqNet::CommandFunc<Pipe>( this, &Pipe::_reqInit ));
    registerCommand( CMD_PIPE_EXIT, 
                     eqNet::CommandFunc<Pipe>( this, &Pipe::pushCommand ));
    registerCommand( REQ_PIPE_EXIT,
                     eqNet::CommandFunc<Pipe>( this, &Pipe::_reqExit ));
    registerCommand( CMD_PIPE_UPDATE,
                     eqNet::CommandFunc<Pipe>( this, &Pipe::_cmdUpdate ));
    registerCommand( REQ_PIPE_UPDATE,
                     eqNet::CommandFunc<Pipe>( this, &Pipe::_reqUpdate ));
    registerCommand( CMD_PIPE_FRAME_SYNC,
                     eqNet::CommandFunc<Pipe>( this, &Pipe::pushCommand ));
    registerCommand( REQ_PIPE_FRAME_SYNC,
                     eqNet::CommandFunc<Pipe>( this, &Pipe::_reqFrameSync ));

    _thread = new PipeThread( this );

#ifdef GLX
    _xDisplay         = 0;
    _xEventConnection = 0;
#endif
#ifdef CGL
    _cglDisplayID = 0;
#endif
#ifdef WGL
    _dc       = 0;
    _dcDelete = false;
#endif
}

Pipe::~Pipe()
{
    delete _thread;
    _thread = 0;
}

void Pipe::_addWindow( Window* window )
{
    _windows.push_back( window );
    window->_pipe = this;
}

void Pipe::_removeWindow( Window* window )
{
    vector<Window*>::iterator iter = find( _windows.begin(), _windows.end(),
                                           window );
    EQASSERT( iter != _windows.end( ))
    
    _windows.erase( iter );
    window->_pipe = 0;
}

eq::Window* Pipe::_findWindow( const uint32_t id )
{
    for( vector<Window*>::const_iterator i = _windows.begin(); 
         i != _windows.end(); ++i )
    {
        Window* window = *i;
        if( window->getID() == id )
            return window;
    }
    return 0;
}

bool Pipe::supportsWindowSystem( const WindowSystem windowSystem ) const
{
    switch( windowSystem )
    {
#ifdef GLX
        case WINDOW_SYSTEM_GLX: return true;
#endif
#ifdef CGL
        case WINDOW_SYSTEM_CGL: return true;
#endif
#ifdef WGL
        case WINDOW_SYSTEM_WGL: return true;
#endif
        default:                return false;
    }
}

WindowSystem Pipe::selectWindowSystem() const
{
    for( WindowSystem i=WINDOW_SYSTEM_NONE; i<WINDOW_SYSTEM_ALL; 
         i = (WindowSystem)((int)i+1) )
    {
        if( supportsWindowSystem( i ))
            return i;
    }
    EQASSERTINFO( 0, "No supported window system found" );
    return WINDOW_SYSTEM_NONE;
}

void Pipe::setXDisplay( Display* display )
{
#ifdef GLX
    _xDisplay = display; 

    if( display )
    {
#ifndef NDEBUG
        // somewhat reduntant since it is a global handler
        XSetErrorHandler( eq::Pipe::XErrorHandler );
#endif

        string       displayString = DisplayString( display );
        const size_t colonPos      = displayString.find( ':' );
        if( colonPos != string::npos )
        {
            const string displayNumberString = displayString.substr(colonPos+1);
            const uint32_t displayNumber = atoi( displayNumberString.c_str( ));
            
            if( _display != EQ_UNDEFINED_UINT32 && displayNumber != _display )
                EQWARN << "Display mismatch: provided display connection uses"
                   << " display " << displayNumber
                   << ", but pipe has display " << _display << endl;

            _display = displayNumber;
        
            if( _screen != EQ_UNDEFINED_UINT32 &&
                DefaultScreen( display ) != (int)_screen )
                
                EQWARN << "Screen mismatch: provided display connection uses"
                       << " default screen " << DefaultScreen( display ) 
                       << ", but pipe has screen " << _screen << endl;
            
            _screen  = DefaultScreen( display );
        }
    }

    if( _pvp.isValid( ))
        return;

    if( display )
    {
        _pvp.x    = 0;
        _pvp.y    = 0;
        _pvp.w = DisplayWidth(  display, DefaultScreen( display ));
        _pvp.h = DisplayHeight( display, DefaultScreen( display ));
    }
    else
        _pvp.reset();
#endif
}

void Pipe::setXEventConnection( RefPtr<X11Connection> display )
{
    _xEventConnection = display; 
}

int Pipe::XErrorHandler( Display* display, XErrorEvent* event )
{
#ifdef GLX
    EQERROR << disableFlush;
    EQERROR << "X Error occured: " << disableHeader << indent;

    char buffer[256];
    XGetErrorText( display, event->error_code, buffer, 256);

    EQERROR << buffer << endl;
    EQERROR << "Major opcode: " << (int)event->request_code << endl;
    EQERROR << "Minor opcode: " << (int)event->minor_code << endl;
    EQERROR << "Error code: " << (int)event->error_code << endl;
    EQERROR << "Request serial: " << event->serial << endl;
    EQERROR << "Current serial: " << NextRequest( display ) - 1 << endl;

    switch( event->error_code )
    {
        case BadValue:
            EQERROR << "  Value: " << event->resourceid << endl;
            break;

        case BadAtom:
            EQERROR << "  AtomID: " << event->resourceid << endl;
            break;

        default:
            EQERROR << "  ResourceID: " << event->resourceid << endl;
            break;
    }
    EQERROR << enableFlush << exdent << enableHeader;
#endif // GLX

    return 0;
}

void Pipe::setCGLDisplayID( CGDirectDisplayID id )
{
#ifdef CGL
    _cglDisplayID = id; 

    if( _pvp.isValid( ))
        return;

    if( id )
    {
        const CGRect displayRect = CGDisplayBounds( id );
        _pvp.x = (int32_t)displayRect.origin.x;
        _pvp.y = (int32_t)displayRect.origin.y;
        _pvp.w = (int32_t)displayRect.size.width;
        _pvp.h = (int32_t)displayRect.size.height;
    }
    else
        _pvp.reset();
#endif
}

void Pipe::setDC( HDC dc, const bool deleteDC )
{
#ifdef WGL
    _dc       = dc; 
    _dcDelete = deleteDC;

    if( _pvp.isValid( ))
        return;

    if( dc )
    {
        _pvp.x = 0;
        _pvp.y = 0;
        _pvp.w = GetDeviceCaps( dc, HORZRES );
        _pvp.h = GetDeviceCaps( dc, VERTRES );
    }
    else
        _pvp.reset();
#endif
}

void* Pipe::_runThread()
{
    EQINFO << "Entered pipe thread" << endl;
    Config* config = getConfig();
    EQASSERT( config );

    while( _thread->isRunning( ))
    {
        eqNet::Command* command = _commandQueue.pop();
        switch( config->dispatchCommand( *command ))
        {
            case eqNet::COMMAND_HANDLED:
            case eqNet::COMMAND_DISCARD:
                break;

            case eqNet::COMMAND_ERROR:
                EQERROR << "Error handling command packet" << endl;
                abort();

            case eqNet::COMMAND_PUSH:
                EQUNIMPLEMENTED;
            case eqNet::COMMAND_PUSH_FRONT:
                EQUNIMPLEMENTED;
            case eqNet::COMMAND_REDISPATCH:
                EQUNIMPLEMENTED;
        }
    }

    EQUNREACHABLE; // exited from _reqExit
    return EXIT_SUCCESS;
}

void Pipe::testMakeCurrentWindow( const Window* window )
{
    if( _currentWindow == window )
        return;

    _currentWindow = window;
    window->makeCurrent();
    return;
}

Frame* Pipe::getFrame( const uint32_t id, const uint32_t version )
{
    Frame* frame = _frames[id];

    if( !frame )
    {
        eqNet::Session* session = getSession();
        frame = new Frame( this );

        const bool mapped = session->mapObject( frame, id );
        EQASSERT( mapped );
        _frames[id] = frame;
    }
    
    frame->sync( version );
    return frame;
}

void Pipe::_flushFrames()
{
    eqNet::Session* session = getSession();

    for( eqNet::IDHash< Frame* >::const_iterator i = _frames.begin(); 
         i != _frames.end(); ++ i )
    {
        Frame* frame = i->second;
        session->unmapObject( frame );
        delete frame;
    }
    _frames.clear();
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
eqNet::CommandResult Pipe::_cmdCreateWindow(  eqNet::Command& command  )
{
    const PipeCreateWindowPacket* packet = 
        command.getPacket<PipeCreateWindowPacket>();
    EQINFO << "Handle create window " << packet << endl;

    Window* window = Global::getNodeFactory()->createWindow();
    
    getConfig()->attachObject( window, packet->windowID );
    _addWindow( window );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Pipe::_cmdDestroyWindow(  eqNet::Command& command  )
{
    const PipeDestroyWindowPacket* packet =
        command.getPacket<PipeDestroyWindowPacket>();
    EQINFO << "Handle destroy window " << packet << endl;

    Window* window = _findWindow( packet->windowID );
    EQASSERT( window );

    _removeWindow( window );

    Config* config = getConfig();
    config->detachObject( window );
    delete window;

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Pipe::_cmdInit( eqNet::Command& command )
{
    const PipeInitPacket* packet = command.getPacket<PipeInitPacket>();
    EQINFO << "handle pipe init (recv) " << packet << endl;

    EQASSERT( _thread->isStopped( ));
    _thread->start();

    return pushCommand( command );
}

eqNet::CommandResult Pipe::_reqInit( eqNet::Command& command )
{
    const PipeInitPacket* packet = command.getPacket<PipeInitPacket>();
    EQINFO << "handle pipe init (pipe) " << packet << endl;
    
    _display      = packet->display;
    _screen       = packet->screen;
    _pvp          = packet->pvp;
    _windowSystem = selectWindowSystem();
    _error.clear();

    PipeInitReplyPacket reply( packet );
    reply.result  = init( packet->initID );

    RefPtr<eqNet::Node> node = command.getNode();
    if( !reply.result )
    {
        send( node, reply, _error );
        return eqNet::COMMAND_HANDLED;
    }

    switch( _windowSystem )
    {
#ifdef GLX
        case WINDOW_SYSTEM_GLX:
            if( !_xDisplay )
            {
                EQERROR << "init() did not set a valid display connection" 
                        << endl;
                reply.result = false;
                send( node, reply );
                return eqNet::COMMAND_HANDLED;
            }

            // TODO: gather and send back display information
            EQINFO << "Using display " << DisplayString( _xDisplay ) << endl;
            break;
#endif
#ifdef CGL
        case WINDOW_SYSTEM_CGL:
            if( !_cglDisplayID )
            {
                EQERROR << "init() did not set a valid display id" << endl;
                reply.result = false;
                send( node, reply );
                return eqNet::COMMAND_HANDLED;
            }
                
            // TODO: gather and send back display information
            EQINFO << "Using display " << _display << endl;
            break;
#endif
#ifdef WGL
        case WINDOW_SYSTEM_WGL:
            if( !_dc )
            {
                EQERROR << "init() did not set a valid device context" << endl;
                reply.result = false;
                send( node, reply );
                return eqNet::COMMAND_HANDLED;
            }
                
            // TODO: gather and send back display information
            EQINFO << "Using dc " << _dc << endl;
            break;
#endif

        default: EQUNIMPLEMENTED;
    }

    reply.pvp = _pvp;
    send( node, reply );

    EventHandler* thread = EventHandler::get( _windowSystem );
    thread->addPipe( this );

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Pipe::_reqExit( eqNet::Command& command )
{
    EventHandler* thread = EventHandler::get( _windowSystem );
    thread->removePipe( this );

    const PipeExitPacket* packet = command.getPacket<PipeExitPacket>();
    EQINFO << "handle pipe exit " << packet << endl;

    exit();
    _flushFrames();

    PipeExitReplyPacket reply( packet );
    send( command.getNode(), reply );

    // cleanup
    _commandQueue.flush();
    
    // exit thread
    EQINFO << "Leaving pipe thread" << endl;
    _thread->exit( EXIT_SUCCESS );
    EQUNREACHABLE;
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Pipe::_cmdUpdate( eqNet::Command& command )
{
    _frameClocks.push_back( Clock( ));
    return pushCommand( command );
}

eqNet::CommandResult Pipe::_reqUpdate( eqNet::Command& command )
{
    const PipeUpdatePacket* packet = command.getPacket<PipeUpdatePacket>();
    EQVERB << "handle pipe update " << packet << endl;
    EQASSERT( !_frameClocks.empty( ));
    
    _frameClock = _frameClocks.front();
    _frameClocks.pop_front();

    startFrame( packet->frameID );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Pipe::_reqFrameSync( eqNet::Command& command )
{
    const PipeFrameSyncPacket* packet =command.getPacket<PipeFrameSyncPacket>();
    EQVERB << "handle pipe frame sync " << packet << endl;

    endFrame( packet->frameID );
    
    PipeFrameSyncReplyPacket reply;
    reply.nStatEvents = _statEvents.size();
    reply.sessionID   = getSession()->getID();
    reply.objectID    = getID();
    reply.frameNumber = packet->frameNumber;

    command.getNode()->send( reply, _statEvents );
    _statEvents.clear();
    return eqNet::COMMAND_HANDLED;
}

//---------------------------------------------------------------------------
// pipe-thread methods
//---------------------------------------------------------------------------
bool Pipe::init( const uint32_t initID )
{
    switch( _windowSystem )
    {
        case WINDOW_SYSTEM_GLX:
            return initGLX();

        case WINDOW_SYSTEM_CGL:
            return initCGL();

        case WINDOW_SYSTEM_WGL:
            return initWGL();

        default:
            EQERROR << "Unknown windowing system: " << _windowSystem << endl;
            setErrorMessage( "Unknown windowing system" );
            return false;
    }
}

bool Pipe::initGLX()
{
#ifdef GLX
    const std::string displayName  = getXDisplayString();
    const char*       cDisplayName = ( displayName.length() == 0 ? 
                                       0 : displayName.c_str( ));
    Display*          xDisplay     = XOpenDisplay( cDisplayName );
            
    if( !xDisplay )
    {
        ostringstream msg;
        msg << "Can't open display: " << XDisplayName( displayName.c_str( ));
        setErrorMessage( msg.str( ));
        return false;
    }
    
    setXDisplay( xDisplay );
    EQINFO << "Opened X display " << xDisplay << ", screen " << _screen << endl;
    return true;
#else
    setErrorMessage( "Library compiled without GLX support" );
    return false;
#endif
}

std::string Pipe::getXDisplayString()
{
    ostringstream  stringStream;
    const uint32_t display = getDisplay();
    const uint32_t screen  = getScreen();
    
    if( display != EQ_UNDEFINED_UINT32 )
    { 
        if( screen == EQ_UNDEFINED_UINT32 )
            stringStream << ":" << display;
        else
            stringStream << ":" << display << "." << screen;
    }
    else if( screen != EQ_UNDEFINED_UINT32 )
        stringStream << ":0." << screen;
    
    return stringStream.str();
}

bool Pipe::initCGL()
{
#ifdef CGL
    const uint32_t    display   = getScreen();
    CGDirectDisplayID displayID = CGMainDisplayID();

    if( display != EQ_UNDEFINED_UINT32 )
    {
        CGDirectDisplayID displayIDs[display+1];
        CGDisplayCount    nDisplays;

        if( CGGetOnlineDisplayList( display+1, displayIDs, &nDisplays ) !=
            kCGErrorSuccess )
        {
            ostringstream msg;
            msg << "Can't get display identifier for display " << display;
            setErrorMessage( msg.str( ));
            return false;
        }

        if( nDisplays <= display )
        {
            ostringstream msg;
            msg << "Can't get display identifier for display " << display 
                << ", not enough displays for this system";
            setErrorMessage( msg.str( ));
            return false;
        }

        displayID = displayIDs[display];
    }

    setCGLDisplayID( displayID );
    EQINFO << "Using CGL displayID " << displayID << endl;
    return true;
#else
    return false;
#endif
}

#ifdef WGL
struct MonitorEnumCBData
{
    uint32_t num;
    Pipe*    pipe;
};

static BOOL CALLBACK monitorEnumCB( HMONITOR hMonitor, HDC hdcMonitor,
                                    LPRECT lprcMonitor, LPARAM dwData )
{
    MonitorEnumCBData* data = reinterpret_cast<MonitorEnumCBData*>( dwData );
    Pipe*              pipe = data->pipe;

    if( data->num < pipe->getScreen( ))
    {
        ++data->num;
        return TRUE; // continue
    }

    MONITORINFOEX monitorInfo;
    monitorInfo.cbSize = sizeof( MONITORINFOEX );
    if( !GetMonitorInfo( hMonitor, &monitorInfo ))
        return FALSE;

    HDC dc = CreateDC( monitorInfo.szDevice, 0, 0, 0 );
    pipe->setDC( dc, true );
    return FALSE;
}
#endif

bool Pipe::initWGL()
{
#ifdef WGL
    const uint32_t screen = getScreen();

    if( screen == EQ_UNDEFINED_UINT32 )
    {
        HDC dc = GetDC( 0 );
        setDC( dc, false );
        return ( dc != 0 );
    }

    // find monitor
    MonitorEnumCBData data = { 0, this };
    EnumDisplayMonitors( 0, 0, monitorEnumCB,reinterpret_cast<LPARAM>( &data ));
    return ( getDC() != 0 );
#else
    return false;
#endif
}

bool Pipe::exit()
{
    switch( _windowSystem )
    {
        case WINDOW_SYSTEM_GLX:
            exitGLX();
            return true;

        case WINDOW_SYSTEM_CGL:
            exitCGL();
            return true;

        case WINDOW_SYSTEM_WGL:
            exitWGL();
            return true;

        default:
            EQWARN << "Unknown windowing system: " << _windowSystem << endl;
            return false;
    }
}

void Pipe::exitGLX()
{
#ifdef GLX
    Display* xDisplay = getXDisplay();
    if( !xDisplay )
        return;

    setXDisplay( 0 );
    XCloseDisplay( xDisplay );
    EQINFO << "Closed X display " << xDisplay << endl;
#endif
}

void Pipe::exitCGL()
{
#ifdef CGL
    setCGLDisplayID( 0 );
    EQINFO << "Reset X CGL displayID " << endl;
#endif
}

void Pipe::exitWGL()
{
#ifdef WGL
    HDC dc = getDC();
    if( needsDCDelete( ))
        DeleteDC( dc );

    setDC( 0, false );
    EQINFO << "Reset Win32 device context " << endl;
#endif
}
