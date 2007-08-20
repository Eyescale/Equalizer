
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "pipe.h"

#include "commands.h"
#include "eventHandler.h"
#include "global.h"
#include "log.h"
#include "nodeFactory.h"
#include "packets.h"
#include "X11Connection.h"
#include "window.h"
#ifdef GLX
#  include "glXEventThread.h"
#endif
#ifdef WGL
#  include "wglEventHandler.h"
#endif

#include <eq/net/command.h>
#include <sstream>

using namespace eq;
using namespace eqBase;
using namespace std;

#ifdef WIN32
#  define bzero( ptr, size ) memset( ptr, 0, size );
#endif

Pipe::Pipe()
        : _eventHandler( 0 ),
          _node( 0 ),
          _currentGLWindow( 0 ),
          _windowSystem( WINDOW_SYSTEM_NONE ),
          _port( EQ_UNDEFINED_UINT32 ),
          _device( EQ_UNDEFINED_UINT32 ),
          _thread( 0 ),
          _commandQueue( 0 )
{
    bzero( _displayFill, sizeof( _displayFill ));
    registerCommand( CMD_PIPE_CREATE_WINDOW,
                   eqNet::CommandFunc<Pipe>( this, &Pipe::_cmdCreateWindow ));
    registerCommand( CMD_PIPE_DESTROY_WINDOW, 
                  eqNet::CommandFunc<Pipe>( this, &Pipe::_cmdDestroyWindow ));
    registerCommand( CMD_PIPE_CONFIG_INIT, 
                     eqNet::CommandFunc<Pipe>( this, &Pipe::_cmdConfigInit ));
    registerCommand( REQ_PIPE_CONFIG_INIT,
                     eqNet::CommandFunc<Pipe>( this, &Pipe::_reqConfigInit ));
    registerCommand( CMD_PIPE_CONFIG_EXIT, 
                     eqNet::CommandFunc<Pipe>( this, &Pipe::pushCommand ));
    registerCommand( REQ_PIPE_CONFIG_EXIT,
                     eqNet::CommandFunc<Pipe>( this, &Pipe::_reqConfigExit ));
    registerCommand( CMD_PIPE_FRAME_START,
                     eqNet::CommandFunc<Pipe>( this, &Pipe::_cmdFrameStart ));
    registerCommand( REQ_PIPE_FRAME_START,
                     eqNet::CommandFunc<Pipe>( this, &Pipe::_reqFrameStart ));
    registerCommand( CMD_PIPE_FRAME_FINISH,
                     eqNet::CommandFunc<Pipe>( this, &Pipe::pushCommand ));
    registerCommand( REQ_PIPE_FRAME_FINISH,
                     eqNet::CommandFunc<Pipe>( this, &Pipe::_reqFrameFinish ));
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
#ifdef AGL
        case WINDOW_SYSTEM_AGL: return true;
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
            
            if( _port != EQ_UNDEFINED_UINT32 && displayNumber != _port )
                EQWARN << "Display mismatch: provided display connection uses"
                   << " display " << displayNumber
                   << ", but pipe has port " << _port << endl;

            if( _device != EQ_UNDEFINED_UINT32 &&
                DefaultScreen( display ) != (int)_device )
                
                EQWARN << "Screen mismatch: provided display connection uses"
                       << " default screen " << DefaultScreen( display ) 
                       << ", but pipe has screen " << _device << endl;
            
            //_port = displayNumber;
            //_device  = DefaultScreen( display );
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
        _pvp.invalidate();
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

void Pipe::setCGDisplayID( CGDirectDisplayID id )
{
#ifdef AGL
    _cgDisplayID = id; 

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
        _pvp.invalidate();
#endif
}

void* Pipe::_runThread()
{
    EQINFO << "Entered pipe thread" << endl;
    Config* config = getConfig();
    EQASSERT( config );
    EQASSERT( _commandQueue );

    while( _thread->isRunning( ))
    {
        eqNet::Command* command = _commandQueue->pop();
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
            case eqNet::COMMAND_REDISPATCH:
                EQUNIMPLEMENTED;
        }
    }

    EQUNREACHABLE; // exited from _reqConfigExit
    return EXIT_SUCCESS;
}

eqNet::CommandResult Pipe::pushCommand( eqNet::Command& command )
{
    if( !_thread )
        return eqNet::COMMAND_PUSH; // handled by main thread

    // else
    _commandQueue->push( command ); 
    return eqNet::COMMAND_HANDLED;
}

void Pipe::testMakeCurrentWindow( const Window* window )
{
    if( _currentGLWindow == window )
        return;

    _currentGLWindow = window;
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

void Pipe::waitExit()
{
    if( !_thread )
        return;

    _thread->join();

    delete _thread;
    delete _commandQueue;
    _thread       = 0;
    _commandQueue = 0;
}

//---------------------------------------------------------------------------
// pipe-thread methods
//---------------------------------------------------------------------------
bool Pipe::configInit( const uint32_t initID )
{
    switch( _windowSystem )
    {
        case WINDOW_SYSTEM_GLX:
            return configInitGLX();

        case WINDOW_SYSTEM_AGL:
            return configInitAGL();

        case WINDOW_SYSTEM_WGL:
            return configInitWGL();

        default:
            EQERROR << "Unknown windowing system: " << _windowSystem << endl;
            setErrorMessage( "Unknown windowing system" );
            return false;
    }
}

bool Pipe::configInitGLX()
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
    EQINFO << "Opened X display " << xDisplay << ", device " << _device << endl;
    return true;
#else
    setErrorMessage( "Library compiled without GLX support" );
    return false;
#endif
}

std::string Pipe::getXDisplayString()
{
    ostringstream  stringStream;
    
    if( _port != EQ_UNDEFINED_UINT32 )
    { 
        if( _device == EQ_UNDEFINED_UINT32 )
            stringStream << ":" << _port;
        else
            stringStream << ":" << _port << "." << _device;
    }
    else if( _device != EQ_UNDEFINED_UINT32 )
        stringStream << ":0." << _device;
    else if( !getenv( "DISPLAY" ))
        stringStream <<  ":0";

    return stringStream.str();
}

bool Pipe::configInitAGL()
{
#ifdef AGL
    CGDirectDisplayID displayID = CGMainDisplayID();

    if( _device != EQ_UNDEFINED_UINT32 )
    {
        CGDirectDisplayID displayIDs[_device+1];
        CGDisplayCount    nDisplays;

        if( CGGetOnlineDisplayList( _device+1, displayIDs, &nDisplays ) !=
            kCGErrorSuccess )
        {
            ostringstream msg;
            msg << "Can't get display identifier for display " << _device;
            setErrorMessage( msg.str( ));
            return false;
        }

        if( nDisplays <= _device )
        {
            ostringstream msg;
            msg << "Can't get display identifier for display " << _device
                << ", not enough displays in this system";
            setErrorMessage( msg.str( ));
            return false;
        }

        displayID = displayIDs[_device];
    }

    setCGDisplayID( displayID );
    EQINFO << "Using CG displayID " << displayID << endl;
    return true;
#else
    return false;
#endif
}

bool Pipe::configInitWGL()
{
#ifdef WGL
    if( _pvp.isValid( ))
        return true;

    HDC                  dc;
    PFNWGLDELETEDCNVPROC deleteDC;
    if( !createAffinityDC( dc, deleteDC ))
        return false;
    
    if( dc ) // createAffinityDC did set up pvp
    {
        deleteDC( dc );
        return true;
    }
    // else don't use affinity dc
    dc = GetDC( 0 );
    EQASSERT( dc );

    _pvp.x = 0;
    _pvp.y = 0;
    _pvp.w = GetDeviceCaps( dc, HORZRES );
    _pvp.h = GetDeviceCaps( dc, VERTRES );
    return true;
#else
    return false;
#endif
}

bool Pipe::configExit()
{
    switch( _windowSystem )
    {
        case WINDOW_SYSTEM_GLX:
            configExitGLX();
            return true;

        case WINDOW_SYSTEM_AGL:
            configExitAGL();
            return true;

        case WINDOW_SYSTEM_WGL:
            configExitWGL();
            return true;

        default:
            EQWARN << "Unknown windowing system: " << _windowSystem << endl;
            return false;
    }
}

void Pipe::configExitGLX()
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

void Pipe::configExitAGL()
{
#ifdef AGL
    setCGDisplayID( 0 );
    EQINFO << "Reset CG displayID " << endl;
#endif
}

void Pipe::configExitWGL()
{
#ifdef WGL
    _pvp.invalidate();
#endif
}

void Pipe::initEventHandling()
{
    EQASSERT( !_eventHandler );
    _eventHandler = EventHandler::registerPipe( this );
}

void Pipe::exitEventHandling()
{
    if( _eventHandler )
        _eventHandler->deregisterPipe( this );
    _eventHandler = 0;
}

void Pipe::releaseFrame( const uint32_t frameNumber )
{ 
    _node->addStatEvents( frameNumber, _statEvents );
    _statEvents.clear();
    _finishedFrame = frameNumber; 
    EQLOG( LOG_TASKS ) << "---- Released Frame --- " << frameNumber << endl;
}

bool Pipe::createAffinityDC( HDC& affinityDC, PFNWGLDELETEDCNVPROC& deleteProc )
{
#ifdef WGL
    affinityDC = 0;
    if( _device == EQ_UNDEFINED_UINT32 )
        return true;

    //----- Create and make current a temporary GL context to get proc address
    //      of WGL_NV_gpu_affinity functions.

    // window class
    ostringstream className;
    className << "TMP" << (void*)this;
    const string& classStr = className.str();
                                  
    HINSTANCE instance = GetModuleHandle( 0 );
    WNDCLASS  wc       = { 0 };
    wc.lpfnWndProc   = WGLEventHandler::wndProc;    
    wc.hInstance     = instance; 
    wc.hIcon         = LoadIcon( NULL, IDI_WINLOGO );
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
    wc.lpszClassName = classStr.c_str();       

    if( !RegisterClass( &wc ))
    {
        setErrorMessage( "Can't register temporary window class: " +
                         getErrorString( GetLastError( )));
        return false;
    }

    // window
    DWORD windowStyleEx = WS_EX_APPWINDOW;
    DWORD windowStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW;

    HWND hWnd = CreateWindowEx( windowStyleEx,
                                wc.lpszClassName, "TMP",
                                windowStyle, 0, 0, 1, 1,
                                0, 0, // parent, menu
                                instance, 0 );

    if( !hWnd )
    {
        setErrorMessage( "Can't create temporary window: " +
                         getErrorString( GetLastError( )));
        UnregisterClass( classStr.c_str(),  instance );
        return false;
    }

    HDC                   dc  = GetDC( hWnd );
    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize        = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_DRAW_TO_WINDOW |
                       PFD_SUPPORT_OPENGL;

    int pf = ChoosePixelFormat( dc, &pfd );
    if( pf == 0 )
    {
        setErrorMessage( "Can't find temporary pixel format: " +
                         getErrorString( GetLastError( )));
        DestroyWindow( hWnd );
        UnregisterClass( classStr.c_str(),  instance );
        return false;
    }
 
    if( !SetPixelFormat( dc, pf, &pfd ))
    {
        setErrorMessage( "Can't set pixel format: " + 
                        getErrorString( GetLastError( )));
        ReleaseDC( hWnd, dc );
        DestroyWindow( hWnd );
        UnregisterClass( classStr.c_str(),  instance );
        return false;
    }

    // context
    HGLRC context = wglCreateContext( dc );
    if( !context )
    {
         setErrorMessage( "Can't create temporary OpenGL context: " + 
                          getErrorString( GetLastError( )));
        ReleaseDC( hWnd, dc );
        DestroyWindow( hWnd );
        UnregisterClass( classStr.c_str(),  instance );
        return false;
    }

    wglMakeCurrent( dc, context );

    PFNWGLENUMGPUSNVPROC enumGPUs = (PFNWGLENUMGPUSNVPROC)
        ( wglGetProcAddress( "wglEnumGpusNV" ));
    PFNWGLCREATEAFFINITYDCNVPROC createAffinityDC = 
        (PFNWGLCREATEAFFINITYDCNVPROC)
            ( wglGetProcAddress( "wglCreateAffinityDCNV" ));
    PFNWGLENUMGPUDEVICESNVPROC enumGPUDevices = 
        (PFNWGLENUMGPUDEVICESNVPROC)
            ( wglGetProcAddress( "wglEnumGpuDevicesNV" ));
    deleteProc = (PFNWGLDELETEDCNVPROC)
        ( wglGetProcAddress( "wglDeleteDCNV" ));

    wglDeleteContext( context );
    ReleaseDC( hWnd, dc );
    DestroyWindow( hWnd );
    UnregisterClass( classStr.c_str(),  instance );

    if( !enumGPUs || !createAffinityDC || !deleteProc )
    {
        EQWARN << "WGL_NV_gpu_affinity unsupported, ignoring device setting"
               << endl;
        return true;
    }

    HGPUNV hGPU[2] = { 0 };
    hGPU[1] = 0;
    if( !enumGPUs( _device, hGPU ))
    {
        setErrorMessage( "Can't enumerate GPU #" + _device );
        return false;
    }

    // setup pvp
    if( !_pvp.isValid() && enumGPUDevices )
    {
        GPU_DEVICE device;
        const bool found = enumGPUDevices( hGPU[0], 0, &device );
        EQASSERT( found );

        const RECT& rect = device.rcVirtualScreen;
        _pvp.x = rect.left;
        _pvp.y = rect.top;
        _pvp.w = rect.right  - rect.left;
        _pvp.h = rect.bottom - rect.top; 
    }

    affinityDC = createAffinityDC( hGPU );
    if( !affinityDC )
    {
        setErrorMessage( "Can't create affinity DC: " +
                         getErrorString( GetLastError( )));
        return false;
    }
    return true;
#else
    return 0;
#endif
}

void Pipe::addStatEvent( StatEvent& event )
{
    event.endTime = _frameClock.getTimef();
    _statEvents.push_back( event );
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

eqNet::CommandResult Pipe::_cmdConfigInit( eqNet::Command& command )
{
    const PipeConfigInitPacket* packet = 
        command.getPacket<PipeConfigInitPacket>();
    EQINFO << "handle pipe configInit (recv) " << packet << endl;

    EQASSERT( !_thread );
    EQASSERT( !_commandQueue );

    if( packet->threaded )
    {
#ifdef WIN32
        if( useMessagePump( )) 
            _commandQueue = new eq::CommandQueue;
        else
#endif
            _commandQueue = new eqNet::CommandQueue;

        _thread = new PipeThread( this );
        _thread->start();
    }

    return pushCommand( command );
}

eqNet::CommandResult Pipe::_reqConfigInit( eqNet::Command& command )
{
    const PipeConfigInitPacket* packet = 
        command.getPacket<PipeConfigInitPacket>();
    EQLOG( LOG_TASKS ) << "TASK configInit " << getName() <<  " " << packet 
                       << endl;
    
    _name         = packet->name;
    _port         = packet->port;
    _device       = packet->device;
    _pvp          = packet->pvp;

    PipeConfigInitReplyPacket reply( packet );
    _node->waitInitialized();

    _windowSystem = selectWindowSystem();
    _error.clear();
    reply.result  = configInit( packet->initID );

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
                EQERROR << "configInit() did not set a display connection" 
                        << endl;
                reply.result = false;
                send( node, reply, _error );
                return eqNet::COMMAND_HANDLED;
            }

            // TODO: gather and send back display information
            EQINFO << "Using display " << DisplayString( _xDisplay ) << endl;
            break;
#endif
#ifdef AGL
        case WINDOW_SYSTEM_AGL:
            if( !_cgDisplayID )
            {
                EQERROR << "configInit() did not set a display id" << endl;
                reply.result = false;
                send( node, reply, _error );
                return eqNet::COMMAND_HANDLED;
            }
                
            // TODO: gather and send back display information
            EQINFO << "Using port " << _port << endl;
            break;
#endif
#ifdef WGL
        case WINDOW_SYSTEM_WGL:
            if( !_pvp.isValid( ))
            {
                EQERROR << "configInit() did not setup pixel viewport" << endl;
                reply.result = false;
                send( node, reply, _error );
                return eqNet::COMMAND_HANDLED;
            }
            break;
#endif

        default: EQUNIMPLEMENTED;
    }

    initEventHandling();
    _initialized = true;

    reply.pvp = _pvp;
    send( node, reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Pipe::_reqConfigExit( eqNet::Command& command )
{
    const PipeConfigExitPacket* packet = 
        command.getPacket<PipeConfigExitPacket>();
    EQLOG( LOG_TASKS ) << "TASK configExit " << getName() <<  " " << packet 
                       << endl;

    if( _initialized.get( ))
        exitEventHandling();

    configExit();

    _initialized = false;
    _flushFrames();

    PipeConfigExitReplyPacket reply( packet );
    send( command.getNode(), reply );

    // exit thread
    if( _thread )
    {
        // cleanup
        _commandQueue->flush();

        EQINFO << "Leaving pipe thread" << endl;
        _thread->exit( EXIT_SUCCESS );
        EQUNREACHABLE;
    }
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Pipe::_cmdFrameStart( eqNet::Command& command )
{
    _frameClockMutex.set();
    _frameClocks.push_back( Clock( ));
    _frameClockMutex.unset();
    return pushCommand( command );
}

eqNet::CommandResult Pipe::_reqFrameStart( eqNet::Command& command )
{
    const PipeFrameStartPacket* packet = 
        command.getPacket<PipeFrameStartPacket>();
    EQVERB << "handle pipe frame start " << packet << endl;

    _frameClockMutex.set();
    EQASSERT( !_frameClocks.empty( ));

    _frameClock = _frameClocks.front();
    _frameClocks.pop_front();
    _frameClockMutex.unset();

    _grabFrame( packet->frameNumber );
    EQLOG( LOG_TASKS ) << "---- Grabbed Frame ---- " << packet->frameNumber
                     << endl;
    frameStart( packet->frameID, packet->frameNumber );

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Pipe::_reqFrameFinish( eqNet::Command& command )
{
    const PipeFrameFinishPacket* packet =
        command.getPacket<PipeFrameFinishPacket>();
    EQVERB << "handle pipe frame sync " << packet << endl;

    frameFinish( packet->frameID, packet->frameNumber );
    EQLOG( LOG_TASKS ) << "---- Finished Frame --- " << _finishedFrame.get()
                       << endl;
    EQASSERTINFO( _finishedFrame >= packet->frameNumber, 
                  "Pipe::frameFinish() did not release frame " 
                  << packet->frameNumber );

    return eqNet::COMMAND_HANDLED;
}
