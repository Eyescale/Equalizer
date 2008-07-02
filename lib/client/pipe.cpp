
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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
#  include "glXEventHandler.h"
#endif
#ifdef WGL
#  include "wglEventHandler.h"
#endif

#include <eq/net/command.h>
#include <sstream>

using namespace eqBase;
using namespace std;
using eqNet::CommandFunc;

namespace eq
{
#ifdef WIN32
#  define bzero( ptr, size ) memset( ptr, 0, size );
#endif

Pipe::Pipe( Node* parent )
        : _eventHandler( 0 )
        , _node( parent )
        , _windowSystem( WINDOW_SYSTEM_NONE )
        , _wglewContext( new WGLEWContext )       
        , _port( EQ_UNDEFINED_UINT32 )
        , _device( EQ_UNDEFINED_UINT32 )
        , _state( STATE_STOPPED )
        , _currentFrame( 0 )
        , _frameTime( 0 )
        , _thread( 0 )
        , _pipeThreadQueue( 0 )
{
    bzero( _pipeFill, sizeof( _pipeFill ));
    parent->_addPipe( this );
    EQINFO << " New eq::Pipe @" << (void*)this << endl;
}

Pipe::~Pipe()
{
    _node->_removePipe( this );
    delete _thread;
    _thread = 0;

    delete _wglewContext;
    _wglewContext = 0;
}

void Pipe::attachToSession( const uint32_t id, const uint32_t instanceID, 
                                 eqNet::Session* session )
{
    eqNet::Object::attachToSession( id, instanceID, session );
    
    eqNet::CommandQueue* queue = getPipeThreadQueue();

    registerCommand( CMD_PIPE_CONFIG_INIT, 
                     CommandFunc<Pipe>( this, &Pipe::_cmdConfigInit ), queue );
    registerCommand( CMD_PIPE_CONFIG_EXIT, 
                     CommandFunc<Pipe>( this, &Pipe::_cmdConfigExit ), queue );
    registerCommand( CMD_PIPE_CREATE_WINDOW,
                     CommandFunc<Pipe>( this, &Pipe::_cmdCreateWindow ),
                     queue );
    registerCommand( CMD_PIPE_DESTROY_WINDOW, 
                     CommandFunc<Pipe>( this, &Pipe::_cmdDestroyWindow ),
                     queue );
    registerCommand( CMD_PIPE_FRAME_START,
                     CommandFunc<Pipe>( this, &Pipe::_cmdFrameStart ), queue );
    registerCommand( CMD_PIPE_FRAME_FINISH,
                     CommandFunc<Pipe>( this, &Pipe::_cmdFrameFinish ), queue );
    registerCommand( CMD_PIPE_FRAME_DRAW_FINISH, 
                     CommandFunc<Pipe>( this, &Pipe::_cmdFrameDrawFinish ),
                     queue );
    registerCommand( CMD_PIPE_STOP_THREAD, 
                     CommandFunc<Pipe>( this, &Pipe::_cmdStopThread ), queue );
    registerCommand( CMD_PIPE_FRAME_START_CLOCK,
                     CommandFunc<Pipe>( this, &Pipe::_cmdFrameStartClock ), 0 );
}

void Pipe::_addWindow( Window* window )
{
    EQASSERT( window->getPipe() == this );
    _windows.push_back( window );
}

void Pipe::_removeWindow( Window* window )
{
    WindowVector::iterator iter = find( _windows.begin(), _windows.end(),
                                        window );
    EQASSERT( iter != _windows.end( ))
    
    _windows.erase( iter );
}

eq::Window* Pipe::_findWindow( const uint32_t id )
{
    for( WindowVector::const_iterator i = _windows.begin(); 
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

void Pipe::_setupCommandQueue()
{
    EQASSERT( _windowSystem != WINDOW_SYSTEM_NONE );

    // Switch the client's message pump for non-threaded and AGL pipes
    if( !_thread || _windowSystem == WINDOW_SYSTEM_AGL )
    {
        RefPtr< Client > client = getClient();
        client->setWindowSystem( _windowSystem );
        return;
    }
    
    EQASSERT( _pipeThreadQueue );
    
    if( useMessagePump( ))
    {
        _pipeThreadQueue->setWindowSystem( _windowSystem );
        EQINFO << "Pipe message pump set up for " << _windowSystem << endl;
    }
}

void Pipe::setXDisplay( Display* display )
{
#ifdef GLX
    if( _xDisplay == display )
		return;

    if( _xDisplay )
        exitEventHandler();
    _xDisplay = display; 

    if( display )
    {
        initEventHandler();
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

#ifndef NDEBUG
    if( getenv( "EQ_ABORT_WAIT" ))
    {
        EQERROR << "Caught X Error, entering infinite loop for debugging" 
                << endl;
        while( true ) ;
    }
#endif

#endif // GLX

    return 0;
}

void Pipe::setCGDisplayID( CGDirectDisplayID id )
{
#ifdef AGL
    if( _cgDisplayID == id )
        return;

    if( _cgDisplayID )
        exitEventHandler();
    _cgDisplayID = id; 
    if( _cgDisplayID )
        initEventHandler();

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
    CHECK_THREAD( _pipeThread );

    Config* config = getConfig();
    EQASSERT( config );
    EQASSERT( _pipeThreadQueue );

    while( _thread->isRunning( ))
    {
        eqNet::Command* command = _pipeThreadQueue->pop();
        switch( config->invokeCommand( *command ))
        {
            case eqNet::COMMAND_HANDLED:
            case eqNet::COMMAND_DISCARD:
                break;

            case eqNet::COMMAND_ERROR:
                EQERROR << "Error handling command packet" << endl;
                abort();
            default:
                EQERROR << "Unknown command result" << endl;
                abort();
        }
        _pipeThreadQueue->release( command );
    }

    EQUNREACHABLE; // since we are exited from _cmdConfigExit
    return EXIT_SUCCESS;
}

eqNet::CommandQueue* Pipe::getPipeThreadQueue()
{
    if( !_thread )
        return _node->getNodeThreadQueue();

    return _pipeThreadQueue;
}

Frame* Pipe::getFrame( const eqNet::ObjectVersion& frameVersion, const Eye eye )
{
    Frame* frame = _frames[ frameVersion.id ];

    if( !frame )
    {
        eqNet::Session* session = getSession();
        frame = new Frame();

        const bool mapped = session->mapObject( frame, frameVersion.id );
        EQASSERT( mapped );
        _frames[ frameVersion.id ] = frame;
    }
    
    frame->sync( frameVersion.version );

    const eqNet::ObjectVersion& data = frame->getDataVersion( eye );
    EQASSERT( data.id != EQ_ID_INVALID );
    FrameData* frameData = getNode()->getFrameData( data ); 
    EQASSERT( frameData );

    frame->setData( frameData );
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

void Pipe::startThread()
{
    _pipeThreadQueue = new CommandQueue;
    _thread          = new PipeThread( this );

    _thread->start();
}

void Pipe::joinThread()
{
    if( !_thread )
        return;

    _thread->join();
    delete _thread;
    _thread = 0;

    delete _pipeThreadQueue;
    _pipeThreadQueue = 0;
}

//---------------------------------------------------------------------------
// pipe-thread methods
//---------------------------------------------------------------------------
void Pipe::_configInitWGLEW()
{
    CHECK_THREAD( _pipeThread );
    if( _windowSystem != WINDOW_SYSTEM_WGL )
        return;

#ifdef WGL
    //----- Create and make current a temporary GL context to initialize WGLEW

    // window class
    ostringstream className;
    className << "TMP" << (void*)this;
    const string& classStr = className.str();
                                  
    HINSTANCE instance = GetModuleHandle( 0 );
    WNDCLASS  wc       = { 0 };
    wc.lpfnWndProc   = WGLEventHandler::wndProc;    
    wc.hInstance     = instance; 
    wc.hIcon         = LoadIcon( 0, IDI_WINLOGO );
    wc.hCursor       = LoadCursor( 0, IDC_ARROW );
    wc.lpszClassName = classStr.c_str();       

    if( !RegisterClass( &wc ))
    {
        EQWARN << "Can't register temporary window class: " 
               << getErrorString( GetLastError( )) << endl;
        return;
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
        EQWARN << "Can't create temporary window: "
               << getErrorString( GetLastError( )) << endl;
        UnregisterClass( classStr.c_str(),  instance );
        return;
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
        EQWARN << "Can't find temporary pixel format: "
               << getErrorString( GetLastError( )) << endl;
        DestroyWindow( hWnd );
        UnregisterClass( classStr.c_str(),  instance );
        return;
    }
 
    if( !SetPixelFormat( dc, pf, &pfd ))
    {
        EQWARN << "Can't set pixel format: " << getErrorString( GetLastError( ))
               << endl;
        ReleaseDC( hWnd, dc );
        DestroyWindow( hWnd );
        UnregisterClass( classStr.c_str(),  instance );
        return;
    }

    // context
    HGLRC context = wglCreateContext( dc );
    if( !context )
    {
         EQWARN << "Can't create temporary OpenGL context: " 
                << getErrorString( GetLastError( )) << endl;
        ReleaseDC( hWnd, dc );
        DestroyWindow( hWnd );
        UnregisterClass( classStr.c_str(),  instance );
        return;
    }

    HDC   oldDC      = wglGetCurrentDC();
    HGLRC oldContext = wglGetCurrentContext();

    wglMakeCurrent( dc, context );

    const GLenum result = wglewInit();
    if( result != GLEW_OK )
        EQWARN << "Pipe WGLEW initialization failed with error " << result 
               << endl;
    else
        EQINFO << "Pipe WGLEW initialization successful" << endl;

    wglDeleteContext( context );
    ReleaseDC( hWnd, dc );
    DestroyWindow( hWnd );
    UnregisterClass( classStr.c_str(),  instance );

    wglMakeCurrent( oldDC, oldContext );
#endif
}

bool Pipe::configInit( const uint32_t initID )
{
    CHECK_THREAD( _pipeThread );
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
    setErrorMessage( "Client library compiled without GLX support" );
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
    setErrorMessage( "Client library compiled without AGL support" );
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
    setErrorMessage( "Client library compiled without WGL support" );
    return false;
#endif
}

bool Pipe::configExit()
{
    CHECK_THREAD( _pipeThread );
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

void Pipe::frameStart( const uint32_t frameID, const uint32_t frameNumber ) 
{ 
    CHECK_THREAD( _pipeThread );
    getNode()->waitFrameStarted( frameNumber );
    startFrame( frameNumber );
}

void Pipe::initEventHandler()
{
    EQASSERT( !_eventHandler );
    _eventHandler = EventHandler::registerPipe( this );
}

void Pipe::exitEventHandler()
{
    if( _eventHandler )
        _eventHandler->deregisterPipe( this );
    _eventHandler = 0;
}

void Pipe::startFrame( const uint32_t frameNumber )
{ 
    CHECK_THREAD( _pipeThread );
    _currentFrame = frameNumber; 
    EQLOG( LOG_TASKS ) << "---- Started Frame ---- " << frameNumber << endl;
}

void Pipe::releaseFrame( const uint32_t frameNumber )
{ 
    CHECK_THREAD( _pipeThread );
    _finishedFrame = frameNumber; 
    EQLOG( LOG_TASKS ) << "---- Finished Frame --- " << frameNumber << endl;
}

void Pipe::releaseFrameLocal( const uint32_t frameNumber )
{ 
    CHECK_THREAD( _pipeThread );
    _unlockedFrame = frameNumber; 
    EQLOG( LOG_TASKS ) << "---- Unlocked Frame --- " << frameNumber << endl;
}

bool Pipe::createAffinityDC( HDC& affinityDC, PFNWGLDELETEDCNVPROC& deleteProc )
{
#ifdef WGL
    affinityDC = 0;
    if( _device == EQ_UNDEFINED_UINT32 )
        return true;

    if( !WGLEW_NV_gpu_affinity )
    {
        EQWARN <<"WGL_NV_gpu_affinity unsupported, ignoring pipe device setting"
               << endl;
        return true;
    }

    HGPUNV hGPU[2] = { 0 };
    hGPU[1] = 0;
    if( !wglEnumGpusNV( _device, hGPU ))
    {
		stringstream error;
		error << "Can't enumerate GPU #" << _device;
        setErrorMessage( error.str( ));
        return false;
    }

    // setup pvp
    if( !_pvp.isValid( ))
    {
        GPU_DEVICE device;
        device.cb = sizeof( device );
        const bool found = wglEnumGpuDevicesNV( hGPU[0], 0, &device );
        EQASSERT( found );

		if( device.Flags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP )
		{
			const RECT& rect = device.rcVirtualScreen;
			_pvp.x = rect.left;
			_pvp.y = rect.top;
			_pvp.w = rect.right  - rect.left;
			_pvp.h = rect.bottom - rect.top; 
		}
		else
		{
			_pvp.x = 0;
			_pvp.y = 0;
			_pvp.w = 4096;
			_pvp.h = 4096;
	    }
    }

    affinityDC = wglCreateAffinityDCNV( hGPU );
    if( !affinityDC )
    {
        setErrorMessage( "Can't create affinity DC: " +
                         getErrorString( GetLastError( )));
        return false;
    }

    deleteProc = wglDeleteDCNV;
    return true;
#else
    return 0;
#endif
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
eqNet::CommandResult Pipe::_cmdCreateWindow(  eqNet::Command& command  )
{
    const PipeCreateWindowPacket* packet = 
        command.getPacket<PipeCreateWindowPacket>();
    EQINFO << "Handle create window " << packet << endl;

    Window* window = Global::getNodeFactory()->createWindow( this );
    getConfig()->attachObject( window, packet->windowID );
    
    EQASSERT( !_windows.empty( ));
    if( window != _windows[0] )
        window->setSharedContextWindow( _windows[0] );

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Pipe::_cmdDestroyWindow(  eqNet::Command& command  )
{
    const PipeDestroyWindowPacket* packet =
        command.getPacket<PipeDestroyWindowPacket>();
    EQINFO << "Handle destroy window " << packet << endl;

    Window* window = _findWindow( packet->windowID );
    EQASSERT( window );

    Config* config = getConfig();
    config->detachObject( window );

    delete window;
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Pipe::_cmdConfigInit( eqNet::Command& command )
{
    CHECK_THREAD( _pipeThread );
    const PipeConfigInitPacket* packet = 
        command.getPacket<PipeConfigInitPacket>();
    EQLOG( LOG_TASKS ) << "TASK pipe config init " << packet << endl;
    
    _name         = packet->name;
    _port         = packet->port;
    _device       = packet->device;
    _pvp          = packet->pvp;
    
    _currentFrame  = 0;
    _finishedFrame = 0;
    _unlockedFrame = 0;

    PipeConfigInitReplyPacket reply;
    _node->waitInitialized();
    _state = STATE_INITIALIZING;

    _windowSystem = selectWindowSystem();
    _setupCommandQueue();

    _error.clear();
    _configInitWGLEW();
    reply.result  = configInit( packet->initID );
    EQLOG( LOG_TASKS ) << "TASK pipe config init reply " << &reply << endl;

    eqNet::NodePtr node = command.getNode();
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
    _state = STATE_RUNNING;

    reply.pvp = _pvp;
    send( node, reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Pipe::_cmdConfigExit( eqNet::Command& command )
{
    CHECK_THREAD( _pipeThread );
    const PipeConfigExitPacket* packet = 
        command.getPacket<PipeConfigExitPacket>();
    EQLOG( LOG_TASKS ) << "TASK configExit " << getName() <<  " " << packet 
                       << endl;

    PipeConfigExitReplyPacket reply;
    reply.result = configExit();

    _state = STATE_STOPPED;
    _flushFrames();

    send( command.getNode(), reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Pipe::_cmdFrameStartClock( eqNet::Command& command )
{
	EQVERB << "start frame clock" << endl;
    _frameTimeMutex.set();
    _frameTimes.push_back( getConfig()->getTime( ));
    _frameTimeMutex.unset();
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Pipe::_cmdFrameStart( eqNet::Command& command )
{
    CHECK_THREAD( _pipeThread );
    const PipeFrameStartPacket* packet = 
        command.getPacket<PipeFrameStartPacket>();
    EQVERB << "handle pipe frame start " << packet << endl;
    EQLOG( LOG_TASKS ) << "---- TASK start frame ---- " << packet << endl;

    const uint32_t frameNumber = packet->frameNumber;
    EQASSERTINFO( _currentFrame + 1 == frameNumber,
                  "current " << _currentFrame << " start " << frameNumber );

    _frameTimeMutex.set();
    EQASSERT( !_frameTimes.empty( ));

    _frameTime = _frameTimes.front();
    _frameTimes.pop_front();
    _frameTimeMutex.unset();

    frameStart( packet->frameID, frameNumber );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Pipe::_cmdFrameFinish( eqNet::Command& command )
{
    CHECK_THREAD( _pipeThread );
    const PipeFrameFinishPacket* packet =
        command.getPacket<PipeFrameFinishPacket>();
    EQVERB << "handle pipe frame finish " << packet << endl;
    EQLOG( LOG_TASKS ) << "---- TASK finish frame --- " << packet << endl;

    const uint32_t frameNumber = packet->frameNumber;
    EQASSERTINFO( _currentFrame >= frameNumber, 
                  "current " << _currentFrame << " finish " << frameNumber );

    frameFinish( packet->frameID, frameNumber );
    
    EQASSERTINFO( _finishedFrame >= frameNumber, 
                  "Pipe::frameFinish() did not release frame " << frameNumber );

    if( _unlockedFrame < frameNumber )
    {
        EQWARN << "Finished frame was not locally unlocked, enforcing unlock" 
               << endl << "    unlocked " << _unlockedFrame.get() << " done "
               << frameNumber << endl;
        releaseFrameLocal( frameNumber );
    }

    if( _finishedFrame < frameNumber )
    {
        EQWARN << "Finished frame was not released, enforcing unlock" << endl;
        releaseFrame( frameNumber );
    }

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Pipe::_cmdFrameDrawFinish( eqNet::Command& command )
{
    CHECK_THREAD( _pipeThread );
    PipeFrameDrawFinishPacket* packet = 
        command.getPacket< PipeFrameDrawFinishPacket >();
    EQLOG( LOG_TASKS ) << "TASK draw finish " << getName() <<  " " << packet
                       << endl;

    frameDrawFinish( packet->frameID, packet->frameNumber );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Pipe::_cmdStopThread( eqNet::Command& command )
{
    EQASSERT( _thread );

    // cleanup
    _pipeThreadQueue->flush();

    EQINFO << "Leaving pipe thread" << endl;
    _thread->exit( EXIT_SUCCESS );
    EQUNREACHABLE;
    return eqNet::COMMAND_HANDLED;
}
}
