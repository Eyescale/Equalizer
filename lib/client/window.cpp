
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "window.h"

#include "channel.h"
#include "commands.h"
#include "configEvent.h"
#include "event.h"
#include "eventHandler.h"
#include "global.h"
#include "log.h"
#include "nodeFactory.h"
#include "packets.h"
#include "windowEvent.h"
#ifdef GLX
#  include "glXEventThread.h"
#endif
#ifdef WGL
#  include "wglEventHandler.h"
#endif

#include <eq/net/barrier.h>
#include <eq/net/command.h>

using namespace eq;
using namespace eqBase;
using namespace std;

#define MAKE_ATTR_STRING( attr ) ( string("EQ_WINDOW_") + #attr )
std::string eq::Window::_iAttributeStrings[IATTR_ALL] = {
    MAKE_ATTR_STRING( IATTR_HINT_STEREO ),
    MAKE_ATTR_STRING( IATTR_HINT_DOUBLEBUFFER ),
    MAKE_ATTR_STRING( IATTR_HINT_FULLSCREEN ),
    MAKE_ATTR_STRING( IATTR_HINT_DECORATION ),
    MAKE_ATTR_STRING( IATTR_PLANES_COLOR ),
    MAKE_ATTR_STRING( IATTR_PLANES_ALPHA ),
    MAKE_ATTR_STRING( IATTR_PLANES_DEPTH ),
    MAKE_ATTR_STRING( IATTR_PLANES_STENCIL )
};


eq::Window::Window()
        : _xDrawable ( 0 ),
          _glXContext( 0 ),
          _cglContext( 0 ),
          _wglWindowHandle( 0 ),
          _wglContext     ( 0 ),
          _wglEventHandler( 0 ),
          _pipe( 0 )
{
    registerCommand( CMD_WINDOW_CREATE_CHANNEL, 
                eqNet::CommandFunc<Window>( this, &Window::_cmdCreateChannel ));
    registerCommand( CMD_WINDOW_DESTROY_CHANNEL,
               eqNet::CommandFunc<Window>( this, &Window::_cmdDestroyChannel ));
    registerCommand( CMD_WINDOW_CONFIG_INIT,
                     eqNet::CommandFunc<Window>( this, &Window::_pushCommand ));
    registerCommand( REQ_WINDOW_CONFIG_INIT, 
                   eqNet::CommandFunc<Window>( this, &Window::_reqConfigInit ));
    registerCommand( CMD_WINDOW_CONFIG_EXIT, 
                     eqNet::CommandFunc<Window>( this, &Window::_pushCommand ));
    registerCommand( REQ_WINDOW_CONFIG_EXIT, 
                   eqNet::CommandFunc<Window>( this, &Window::_reqConfigExit ));
    registerCommand( CMD_WINDOW_FRAME_START,
                     eqNet::CommandFunc<Window>( this, &Window::_pushCommand ));
    registerCommand( REQ_WINDOW_FRAME_START,
                   eqNet::CommandFunc<Window>( this, &Window::_reqFrameStart ));
    registerCommand( CMD_WINDOW_FRAME_FINISH,
                     eqNet::CommandFunc<Window>( this, &Window::_pushCommand ));
    registerCommand( REQ_WINDOW_FRAME_FINISH,
                  eqNet::CommandFunc<Window>( this, &Window::_reqFrameFinish ));
    registerCommand( CMD_WINDOW_FINISH, 
                     eqNet::CommandFunc<Window>( this, &Window::_pushCommand));
    registerCommand( REQ_WINDOW_FINISH, 
                     eqNet::CommandFunc<Window>( this, &Window::_reqFinish));
    registerCommand( CMD_WINDOW_BARRIER, 
                     eqNet::CommandFunc<Window>( this, &Window::_pushCommand ));
    registerCommand( REQ_WINDOW_BARRIER,
                     eqNet::CommandFunc<Window>( this, &Window::_reqBarrier ));
    registerCommand( CMD_WINDOW_SWAP, 
                     eqNet::CommandFunc<Window>( this, &Window::_pushCommand ));
    registerCommand( REQ_WINDOW_SWAP, 
                     eqNet::CommandFunc<Window>( this, &Window::_reqSwap));
}

eq::Window::~Window()
{
    if( _wglEventHandler )
        EQWARN << "WGL event handler present in destructor" << endl;

#ifdef WGL
    delete _wglEventHandler;
    _wglEventHandler = 0;
#endif
}

void eq::Window::_addChannel( Channel* channel )
{
    _channels.push_back( channel );
    channel->_window = this;
}

void eq::Window::_removeChannel( Channel* channel )
{
    vector<Channel*>::iterator iter = find( _channels.begin(), _channels.end(), 
                                            channel );
    EQASSERT( iter != _channels.end( ))
    
    _channels.erase( iter );
    channel->_window = 0;
}

Channel* eq::Window::_findChannel( const uint32_t id )
{
    for( vector<Channel*>::const_iterator i = _channels.begin(); 
         i != _channels.end(); ++i )
    {
        Channel* channel = *i;
        if( channel->getID() == id )
            return channel;
    }
    return 0;
}

//======================================================================
// pipe-thread methods
//======================================================================

//----------------------------------------------------------------------
// viewport
//----------------------------------------------------------------------
void eq::Window::setPixelViewport( const PixelViewport& pvp )
{
    if( !_setPixelViewport( pvp ))
        return; // nothing changed

    WindowSetPVPPacket packet;
    packet.pvp = pvp;
    _send( packet );
}

bool eq::Window::_setPixelViewport( const PixelViewport& pvp )
{
    if( pvp == _pvp || !pvp.hasArea( ))
        return false;

    _pvp = pvp;
    _vp.invalidate();

    EQASSERT( _pipe );
    
    const PixelViewport& pipePVP = _pipe->getPixelViewport();
    if( pipePVP.isValid( ))
        _vp = pvp / pipePVP;

    EQINFO << "Window pvp set: " << _pvp << ":" << _vp << endl;
    return true;
}

void eq::Window::_setViewport( const Viewport& vp )
{
    if( !vp.hasArea( ))
        return;
    
    _vp = vp;
    _pvp.invalidate();

    if( !_pipe )
        return;

    PixelViewport pipePVP = _pipe->getPixelViewport();
    if( pipePVP.isValid( ))
    {
        pipePVP.x = 0;
        pipePVP.y = 0;
        _pvp = pipePVP * vp;
    }
    EQINFO << "Window vp set: " << _pvp << ":" << _vp << endl;
}

//----------------------------------------------------------------------
// configInit
//----------------------------------------------------------------------
bool eq::Window::configInit( const uint32_t initID )
{
    const WindowSystem windowSystem = _pipe->getWindowSystem();
    switch( windowSystem )
    {
        case WINDOW_SYSTEM_GLX:
            if( !configInitGLX( ))
                return false;
            break;

        case WINDOW_SYSTEM_CGL:
            if( !configInitCGL( ))
                return false;
            break;

        case WINDOW_SYSTEM_WGL:
            if( !configInitWGL( ))
                return false;
            break;

        default:
            EQERROR << "Unknown windowing system: " << windowSystem << endl;
            return false;
    }
    return configInitGL( initID );
}

bool eq::Window::configInitGL( const uint32_t initID )
{
    glEnable( GL_SCISSOR_TEST ); // needed to constrain channel viewport
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LESS );

    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );

    glColorMaterial( GL_FRONT_AND_BACK, GL_DIFFUSE );
    glEnable( GL_COLOR_MATERIAL );

    glClearDepth( 1.f );
    //glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );

    glClear( GL_COLOR_BUFFER_BIT );
    swapBuffers();
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    return true;
}

#ifdef GLX
static Bool WaitForNotify( Display*, XEvent *e, char *arg )
{ return (e->type == MapNotify) && (e->xmap.window == (::Window)arg); }
#endif

bool eq::Window::configInitGLX()
{
#ifdef GLX
    Display* display = _pipe->getXDisplay();
    if( !display )
    {
        setErrorMessage( "Pipe has no X11 display connection" );
        return false;
    }

    vector<int> attributes;
    attributes.push_back( GLX_RGBA );

    const int colorSize = getIAttribute( IATTR_PLANES_COLOR );
    if( colorSize > 0 || colorSize == AUTO )
    {
        attributes.push_back( GLX_RED_SIZE );
        attributes.push_back( colorSize>0 ? colorSize : 1 );
        attributes.push_back( GLX_GREEN_SIZE );
        attributes.push_back( colorSize>0 ? colorSize : 1 );
        attributes.push_back( GLX_BLUE_SIZE );
        attributes.push_back( colorSize>0 ? colorSize : 1 );
    }
    const int alphaSize = getIAttribute( IATTR_PLANES_ALPHA );
    if( alphaSize > 0 || alphaSize == AUTO )
    {
        attributes.push_back( GLX_ALPHA_SIZE );
        attributes.push_back( alphaSize>0 ? alphaSize : 1 );
    }
    const int depthSize = getIAttribute( IATTR_PLANES_DEPTH );
    if( depthSize > 0  || depthSize == AUTO )
    {
        attributes.push_back( GLX_DEPTH_SIZE );
        attributes.push_back( depthSize>0 ? depthSize : 1 );
    }
    const int stencilSize = getIAttribute( IATTR_PLANES_STENCIL );
    if( stencilSize >0 || depthSize == AUTO )
    {
        attributes.push_back( GLX_STENCIL_SIZE );
        attributes.push_back( stencilSize>0 ? stencilSize : 1 );
    }

    if( getIAttribute( IATTR_HINT_STEREO ) != OFF )
        attributes.push_back( GLX_STEREO );
    if( getIAttribute( IATTR_HINT_DOUBLEBUFFER ) != OFF )
        attributes.push_back( GLX_DOUBLEBUFFER );
    attributes.push_back( None );

    const int    screen  = DefaultScreen( display );
    XVisualInfo *visInfo = glXChooseVisual( display, screen, &attributes[0] );

    if( !visInfo && getIAttribute( IATTR_HINT_STEREO ) == AUTO )
    {        
        EQINFO << "Stereo not available, requesting mono visual" << endl;
        vector<int>::iterator iter = find( attributes.begin(), attributes.end(),
                                           GLX_STEREO );
        attributes.erase( iter );
        visInfo = glXChooseVisual( display, screen, &attributes[0] );
    }
    if( !visInfo && getIAttribute( IATTR_HINT_DOUBLEBUFFER ) == AUTO )
    {        
        EQINFO << "Doublebuffer not available, requesting singlebuffer visual" 
               << endl;
        vector<int>::iterator iter = find( attributes.begin(), attributes.end(),
                                           GLX_DOUBLEBUFFER );
        attributes.erase( iter );
        visInfo = glXChooseVisual( display, screen, &attributes[0] );
    }

    if ( !visInfo )
    {
        setErrorMessage( "Could not find a matching visual" );
        return false;
    }

    EQINFO << "Using visual 0x" << std::hex << visInfo->visualid << std::dec 
           << endl;

    XID                  parent = RootWindow( display, screen );
    XSetWindowAttributes wa;
    wa.colormap          = XCreateColormap( display, parent, visInfo->visual,
                                            AllocNone );
    wa.background_pixmap = None;
    wa.border_pixel      = 0;
    wa.event_mask        = StructureNotifyMask | VisibilityChangeMask;
    if( getIAttribute( IATTR_HINT_DECORATION ) != OFF )
        wa.override_redirect = False;
    else
        wa.override_redirect = True;
        
    if( getIAttribute( IATTR_HINT_FULLSCREEN ) == ON )
    {
        wa.override_redirect = True;
        _pvp.h = DisplayHeight( display, screen );
        _pvp.w = DisplayWidth( display, screen );
        _pvp.x = 0;
        _pvp.y = 0;
    }

    XID drawable = XCreateWindow( display, parent, 
                                  _pvp.x, _pvp.y, _pvp.w, _pvp.h,
                                  0, visInfo->depth, InputOutput,
                                  visInfo->visual, CWBackPixmap|CWBorderPixel|
                                  CWEventMask|CWColormap|CWOverrideRedirect,
                                  &wa );
    
    if ( !drawable )
    {
        setErrorMessage( "Could not create window" );
        return false;
    }   
   
    XStoreName( display, drawable,_name.empty() ? "Equalizer" : _name.c_str( ));

    // Register for close window request from the window manager
    //  The WM sends a ClientMessage with the delete atom directly to the
    //  window. Therefore we can not request these events with XSelectInput, we
    //  have to handle them from the pipe thread. We'll check during startFrame
    //  for the event and issue a WindowEvent::CLOSE.
    Atom deleteAtom = XInternAtom( display, "WM_DELETE_WINDOW", False );
    XSetWMProtocols( display, drawable, &deleteAtom, 1 );

    // map and wait for MapNotify event
    XMapWindow( display, drawable );

    XEvent event;
    XIfEvent( display, &event, WaitForNotify, (XPointer)(drawable) );

    XMoveResizeWindow( display, drawable, _pvp.x, _pvp.y, _pvp.w, _pvp.h );
    XFlush( display );
    setXDrawable( drawable );

    // create context
    Pipe*      pipe        = getPipe();
    Window*    firstWindow = pipe->getWindow(0);
    GLXContext shareCtx    = firstWindow->getGLXContext();
    GLXContext context = glXCreateContext( display, visInfo, shareCtx, True );

    if ( !context )
    {
        setErrorMessage( "Could not create OpenGL context" );
        return false;
    }

    glXMakeCurrent( display, drawable, context );

    setGLXContext( context );
    EQINFO << "Created X11 drawable " << drawable << ", glX context "
           << context << endl;
    return true;
#else
    return false;
#endif
}

bool eq::Window::configInitCGL()
{
#ifdef CGL
    CGDirectDisplayID displayID = _pipe->getCGLDisplayID();

    vector<int> attributes;
    attributes.push_back( kCGLPFADisplayMask );
    attributes.push_back( CGDisplayIDToOpenGLDisplayMask( displayID ));
    attributes.push_back( kCGLPFAFullScreen );

    const int colorSize = getIAttribute( IATTR_PLANES_COLOR );
    if( colorSize > 0 || colorSize == AUTO )
    {
        attributes.push_back( kCGLPFAColorSize );
        attributes.push_back( colorSize>0 ? colorSize : 1 );
    }
    const int alphaSize = getIAttribute( IATTR_PLANES_ALPHA );
    if( alphaSize > 0 || alphaSize == AUTO )
    {
        attributes.push_back( kCGLPFAAlphaSize );
        attributes.push_back( alphaSize>0 ? alphaSize : 1  );
    }
    const int depthSize = getIAttribute( IATTR_PLANES_DEPTH );
    if( depthSize > 0 || depthSize == AUTO )
    {
        attributes.push_back( kCGLPFADepthSize );
        attributes.push_back( depthSize>0 ? depthSize : 1 );
    }
    const int stencilSize = getIAttribute( IATTR_PLANES_STENCIL );
    if( stencilSize > 0 )
    {
        attributes.push_back( kCGLPFAStencilSize );
        attributes.push_back( stencilSize>0 ? stencilSize : 1 );
    }

    if( getIAttribute( IATTR_HINT_DOUBLEBUFFER ) != OFF )
        attributes.push_back( kCGLPFADoubleBuffer );
    if( getIAttribute( IATTR_HINT_STEREO ) != OFF )
        attributes.push_back( kCGLPFAStereo );

    attributes.push_back( 0 );

    CGLPixelFormatObj pixelFormat = 0;
    long numPixelFormats = 0;
    const CGLPixelFormatAttribute* cglAttribs = 
        (CGLPixelFormatAttribute*)&attributes[0];
    CGLChoosePixelFormat( cglAttribs, &pixelFormat, &numPixelFormats );

    if( !pixelFormat && getIAttribute( IATTR_HINT_STEREO ) == AUTO )
    {
        vector<int>::iterator iter = 
            find( attributes.begin(), attributes.end(), kCGLPFAStereo );
        attributes.erase( iter );
        const CGLPixelFormatAttribute* cglAttribs = 
            (CGLPixelFormatAttribute*)&attributes[0];
        CGLChoosePixelFormat( cglAttribs, &pixelFormat, &numPixelFormats );
    }
    if( !pixelFormat && getIAttribute( IATTR_HINT_DOUBLEBUFFER ) == AUTO )
    {
        vector<int>::iterator iter = 
            find( attributes.begin(), attributes.end(), kCGLPFADoubleBuffer );
        attributes.erase( iter );
        const CGLPixelFormatAttribute* cglAttribs = 
            (CGLPixelFormatAttribute*)&attributes[0];
        CGLChoosePixelFormat( cglAttribs, &pixelFormat, &numPixelFormats );
    }

    if( !pixelFormat )
    {
        setErrorMessage( "Could not find a matching pixel format" );
        return false;
    }

    Pipe*         pipe        = getPipe();
    Window*       firstWindow = pipe->getWindow(0);
    CGLContextObj shareCtx    = firstWindow->getCGLContext();
    CGLContextObj context     = 0;
    CGLCreateContext( pixelFormat, shareCtx, &context );
    CGLDestroyPixelFormat ( pixelFormat );

    if( !context ) 
    {
        setErrorMessage( "Could not create OpenGL context" );
        return false;
    }

    CGLSetCurrentContext( context );
    CGLSetFullScreen( context );

    setCGLContext( context );
    EQINFO << "Created CGL context " << context << endl;
    return true;
#else
    setErrorMessage( "Library compiled without CGL support" );
    return false;
#endif
}

bool eq::Window::configInitWGL()
{
#ifdef WGL
    // window class
    ostringstream className;
    className << (_name.empty() ? string("Equalizer") : _name) << (void*)this;
    const string& classStr = className.str();
                                  
    HINSTANCE instance = GetModuleHandle( 0 );
    WNDCLASS  wc = { 0 };
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; 
    wc.lpfnWndProc   = WGLEventHandler::wndProc;    
    wc.hInstance     = instance; 
    wc.hIcon         = LoadIcon( NULL, IDI_WINLOGO );
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
    wc.lpszClassName = classStr.c_str();       

    if( !RegisterClass( &wc ))
    {
        setErrorMessage( "Can't register window class: " + 
                         getErrorString( GetLastError( )));
	    return false;
    }

    // window
    DWORD windowStyleEx = WS_EX_APPWINDOW;
    DWORD windowStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW;

    if( getIAttribute( IATTR_HINT_DECORATION ) == OFF )
        windowStyle = WS_POPUP;

    if( getIAttribute( IATTR_HINT_FULLSCREEN ) == ON )
    {
        DEVMODE deviceMode = {0};
        deviceMode.dmSize = sizeof( DEVMODE );
        EnumDisplaySettings( 0, ENUM_CURRENT_SETTINGS, &deviceMode );

        if( ChangeDisplaySettings( &deviceMode, CDS_FULLSCREEN ) != 
            DISP_CHANGE_SUCCESSFUL )
        {
            setErrorMessage( "Can't switch to fullscreen mode: " + 
                         getErrorString( GetLastError( )));
	        return false;
        }
        windowStyle = WS_POPUP | WS_MAXIMIZE;
    }

    HWND hWnd = CreateWindowEx( windowStyleEx,
                                wc.lpszClassName, 
                                _name.empty() ? "Equalizer" : _name.c_str(),
                  	            windowStyle, _pvp.x, _pvp.y, _pvp.w, _pvp.h,
                                0, 0, // parent, menu
                                instance, 0 );

    if( !hWnd )
    {
        setErrorMessage( "Can't create window: " + 
                         getErrorString( GetLastError( )));
	    return false;
    }

    setWGLWindowHandle( hWnd );
    ShowWindow( hWnd, SW_SHOW );
    UpdateWindow( hWnd );

    // per-GPU affinity DC
    // We need to create one DC per window, since the window DC pixel format and
    // the affinity RC pixel format have to match, and each window has
    // potentially a different pixel format.
    HDC                  affinityDC;
    PFNWGLDELETEDCNVPROC deleteDC;
    if( !_pipe->createAffinityDC( affinityDC, deleteDC ))
    {
        setErrorMessage( "Can't create affinity dc" );
        return false;
    }

    // pixel format
    HDC windowDC   = GetDC( hWnd );
    HDC dc         = affinityDC ? affinityDC : windowDC;

    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize        = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_DRAW_TO_WINDOW |
                       PFD_SUPPORT_OPENGL;
    pfd.iPixelType   = PFD_TYPE_RGBA;

    const int colorSize = getIAttribute( IATTR_PLANES_COLOR );
    if( colorSize > 0 || colorSize == AUTO )
        pfd.cColorBits = colorSize>0 ? colorSize : 1;

    const int alphaSize = getIAttribute( IATTR_PLANES_ALPHA );
    if( alphaSize > 0 || alphaSize == AUTO )
        pfd.cAlphaBits = alphaSize>0 ? alphaSize : 1;

    const int depthSize = getIAttribute( IATTR_PLANES_DEPTH );
    if( depthSize > 0  || depthSize == AUTO )
        pfd.cDepthBits = depthSize>0 ? depthSize : 1;

    const int stencilSize = getIAttribute( IATTR_PLANES_STENCIL );
    if( stencilSize >0 || stencilSize == AUTO )
        pfd.cStencilBits = stencilSize>0 ? stencilSize : 1;

    if( getIAttribute( IATTR_HINT_STEREO ) != OFF )
        pfd.dwFlags |= PFD_STEREO;
    if( getIAttribute( IATTR_HINT_DOUBLEBUFFER ) != OFF )
        pfd.dwFlags |= PFD_DOUBLEBUFFER;
    int pf = ChoosePixelFormat( dc, &pfd );

    if( pf == 0 && getIAttribute( IATTR_HINT_STEREO ) == AUTO )
    {        
        EQINFO << "Stereo not available, requesting mono visual" << endl;
        pfd.dwFlags |= PFD_STEREO_DONTCARE;
        pf = ChoosePixelFormat( dc, &pfd );
    }

    if( pf == 0 && getIAttribute( IATTR_HINT_DOUBLEBUFFER ) == AUTO )
    {        
        EQINFO << "Doublebuffer not available, requesting singlebuffer visual" 
               << endl;
        pfd.dwFlags |= PFD_DOUBLEBUFFER_DONTCARE;
        pf = ChoosePixelFormat( dc, &pfd );
    }

    if( pf == 0 )
    {
        setErrorMessage( "Can't find matching pixel format: " + 
                         getErrorString( GetLastError( )));
        ReleaseDC( hWnd, windowDC );
	    return false;
    }
 
    if( !SetPixelFormat( dc, pf, &pfd ))
    {
        setErrorMessage( "Can't set pixel format: " + 
                         getErrorString( GetLastError( )));
        ReleaseDC( hWnd, windowDC );
	    return false;
    }

    // set the same pixel format on the window DC
    if( affinityDC )
    {
        DescribePixelFormat( dc, pf, sizeof( pfd ), &pfd );
        if( !SetPixelFormat( windowDC, pf, &pfd ))
        {
            setErrorMessage( "Can't set pixel format: " + 
                             getErrorString( GetLastError( )));
            ReleaseDC( hWnd, windowDC );
	        return false;
        }
    }

    // context
    HGLRC context = wglCreateContext( dc );
    if( !context )
    {
        setErrorMessage( "Can't create OpenGL context: " + 
                         getErrorString( GetLastError( )));
	    return false;
    }

    if( !wglMakeCurrent( dc, context ))
    {
        setErrorMessage( "Can't make OpenGL context current: " + 
                         getErrorString( GetLastError( )));
	    return false;
    }

    Pipe*    pipe        = getPipe();
    Window*  firstWindow = pipe->getWindow(0);
    HGLRC    shareCtx    = firstWindow->getWGLContext();

    if( shareCtx && !wglShareLists( shareCtx, context ))
        EQWARN << "Context sharing failed: " << getErrorString( GetLastError( ))
               << endl;

    setWGLContext( context );
    ReleaseDC( hWnd, windowDC );
    if( affinityDC )
        deleteDC( affinityDC );

    EQINFO << "Created WGL context " << context << endl;
    return true;
#else
    return false;
#endif
}

//----------------------------------------------------------------------
// configExit
//----------------------------------------------------------------------
bool eq::Window::configExit()
{
    const WindowSystem windowSystem = _pipe->getWindowSystem();
    switch( windowSystem )
    {
        case WINDOW_SYSTEM_GLX:
            configExitGLX();
            return true;

        case WINDOW_SYSTEM_CGL:
            configExitCGL();
            return false;

        case WINDOW_SYSTEM_WGL:
            configExitWGL();
            return false;

        default:
            EQWARN << "Unknown windowing system: " << windowSystem << endl;
            return false;
    }
}

void eq::Window::configExitGLX()
{
#ifdef GLX
    Display *display = _pipe->getXDisplay();
    if( !display ) 
        return;

    glXMakeCurrent( display, None, NULL );

    GLXContext context = getGLXContext();
    if( context )
        glXDestroyContext( display, context );
    setGLXContext( 0 );

    XID drawable = getXDrawable();
    if( drawable )
        XDestroyWindow( display, drawable );
    setXDrawable( 0 );
    EQINFO << "Destroyed GLX context and X drawable " << endl;
#endif
}

void eq::Window::configExitCGL()
{
#ifdef CGL
    CGLContextObj context = getCGLContext();
    if( !context )
        return;

    setCGLContext( 0 );

    CGLSetCurrentContext( 0 );
    CGLClearDrawable( context );
    CGLDestroyContext ( context );       
    EQINFO << "Destroyed CGL context" << endl;
#endif
}

void eq::Window::configExitWGL()
{
#ifdef WGL
    wglMakeCurrent( 0, 0 );

    HGLRC context = getWGLContext();
    if( context )
        wglDeleteContext( context );

    setWGLContext( 0 );

    HWND hWnd = getWGLWindowHandle();
    if( hWnd )
        DestroyWindow( hWnd );

    setWGLWindowHandle( 0 );

    if( getIAttribute( IATTR_HINT_FULLSCREEN ) == ON )
        ChangeDisplaySettings( 0, 0 );

    EQINFO << "Destroyed WGL context and window" << endl;
#endif
}

void eq::Window::_initEventHandling()
{
    switch( _pipe->getWindowSystem( ))
    {
        case WINDOW_SYSTEM_GLX:
#ifdef GLX
	{
            GLXEventThread* thread = GLXEventThread::get();
            thread->addWindow( this );
	}
#endif
            break;

        case WINDOW_SYSTEM_WGL:
#ifdef WGL
            _wglEventHandler = new WGLEventHandler( this );
#endif
            break;

        default:
            EQERROR << "event handling not implemented for window system " 
                    << _pipe->getWindowSystem() << endl;
            break;
    }
}

void eq::Window::_exitEventHandling()
{
    switch( _pipe->getWindowSystem( ))
    {
        case WINDOW_SYSTEM_GLX:
#ifdef GLX
	{
            GLXEventThread* thread = GLXEventThread::get();
            thread->removeWindow( this );
	}
#endif
            break;

        case WINDOW_SYSTEM_WGL:
#ifdef WGL
            delete _wglEventHandler;
            _wglEventHandler = 0;
#endif
            break;

        default:
            EQERROR << "event handling not implemented for window system " 
                    << _pipe->getWindowSystem() << endl;
            break;
    }
}

void eq::Window::setXDrawable( XID drawable )
{
#ifdef GLX
    _xDrawable = drawable;

    if( !drawable )
    {
        _pvp.reset();
        return;
    }

    // query pixel viewport of window
    Display          *display = _pipe->getXDisplay();
    EQASSERT( display );

    XWindowAttributes wa;
    XGetWindowAttributes( display, drawable, &wa );
    
    // Window position is relative to parent: translate to absolute coordinates
    ::Window root, parent, *children;
    unsigned nChildren;
    
    XQueryTree( display, drawable, &root, &parent, &children, &nChildren );
    if( children != 0 ) XFree( children );

    int x,y;
    ::Window childReturn;
    XTranslateCoordinates( display, parent, root, wa.x, wa.y, &x, &y,
        &childReturn );

    _pvp.x = x;
    _pvp.y = y;
    _pvp.w = wa.width;
    _pvp.h = wa.height;
#endif // GLX
}

void eq::Window::setCGLContext( CGLContextObj context )
{
#ifdef CGL
    _cglContext = context;
    // TODO: pvp
#endif // CGL
}


void eq::Window::setWGLWindowHandle( HWND handle )
{
#ifdef WGL
    _wglWindowHandle = handle;

    if( !handle )
    {
        _pvp.reset();
        return;
    }

    // query pixel viewport of window
    WINDOWINFO windowInfo;
    windowInfo.cbSize = sizeof( windowInfo );

    GetWindowInfo( handle, &windowInfo );

    _pvp.x = windowInfo.rcWindow.left;
    _pvp.y = windowInfo.rcWindow.top;
    _pvp.w = windowInfo.rcWindow.right  - windowInfo.rcWindow.left;
    _pvp.h = windowInfo.rcWindow.bottom - windowInfo.rcWindow.top;
#endif // WGL
}

void eq::Window::makeCurrent() const
{
    switch( _pipe->getWindowSystem( ))
    {
#ifdef GLX
        case WINDOW_SYSTEM_GLX:
            glXMakeCurrent( _pipe->getXDisplay(), _xDrawable, _glXContext );
            break;
#endif
#ifdef CGL
        case WINDOW_SYSTEM_CGL:
            CGLSetCurrentContext( _cglContext );
            break;
#endif
#ifdef WGL
        case WINDOW_SYSTEM_WGL:
        {
            HDC dc = GetDC( _wglWindowHandle );
            wglMakeCurrent( dc, _wglContext );
            ReleaseDC( _wglWindowHandle, dc );
        } break;
#endif

        default: EQUNIMPLEMENTED;
    }
}

void eq::Window::swapBuffers() const
{
    switch( _pipe->getWindowSystem( ))
    {
#ifdef GLX
        case WINDOW_SYSTEM_GLX:
            glXSwapBuffers( _pipe->getXDisplay(), _xDrawable );
            break;
#endif
#ifdef CGL
        case WINDOW_SYSTEM_CGL:
            CGLFlushDrawable( _cglContext );
            break;
#endif
#ifdef WGL
        case WINDOW_SYSTEM_WGL:
        {
            HDC dc = GetDC( _wglWindowHandle );
            SwapBuffers( dc );
            ReleaseDC( _wglWindowHandle, dc );
        } break;
#endif

        default: EQUNIMPLEMENTED;
    }
    EQVERB << "----- SWAP -----" << endl;
}

//======================================================================
// event-handler methods
//======================================================================
bool eq::Window::processEvent( const WindowEvent& event )
{
    ConfigEvent configEvent;
    switch( event.type )
    {
        case WindowEvent::EXPOSE:
            return true;

        case WindowEvent::RESIZE:
            setPixelViewport( PixelViewport( event.resize.x, event.resize.y, 
                                             event.resize.w, event.resize.h ));
            return true;

        case WindowEvent::CLOSE:
            configEvent.type = ConfigEvent::WINDOW_CLOSE;
            break;

        case WindowEvent::POINTER_MOTION:
            configEvent.type          = ConfigEvent::POINTER_MOTION;
            configEvent.pointerMotion = event.pointerMotion;
            break;
            
        case WindowEvent::POINTER_BUTTON_PRESS:
            configEvent.type = ConfigEvent::POINTER_BUTTON_PRESS;
            configEvent.pointerButtonPress = event.pointerButtonPress;
            break;

        case WindowEvent::POINTER_BUTTON_RELEASE:
            configEvent.type = ConfigEvent::POINTER_BUTTON_RELEASE;
            configEvent.pointerButtonRelease = event.pointerButtonRelease;
            break;

        case WindowEvent::KEY_PRESS:
            if( event.keyPress.key == KC_VOID )
                return true; //ignore
            configEvent.type         = ConfigEvent::KEY_PRESS;
            configEvent.keyPress.key = event.keyPress.key;
            break;
                
        case WindowEvent::KEY_RELEASE:
            if( event.keyPress.key == KC_VOID )
                return true; // ignore
            configEvent.type           = ConfigEvent::KEY_RELEASE;
            configEvent.keyRelease.key = event.keyRelease.key;
            break;

        case WindowEvent::UNHANDLED:
            // Handle other window-system native events here
            return false;

        default:
            EQWARN << "Unhandled window event of type " << event.type << endl;
            EQUNIMPLEMENTED;
    }
    
    Config* config = getConfig();
    config->sendEvent( configEvent );
    return true;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
eqNet::CommandResult eq::Window::_pushCommand( eqNet::Command& command )
{
    return ( _pipe ? _pipe->pushCommand( command ) : _cmdUnknown( command ));
}

eqNet::CommandResult eq::Window::_cmdCreateChannel( eqNet::Command& command )
{
    const WindowCreateChannelPacket* packet = 
        command.getPacket<WindowCreateChannelPacket>();
    EQINFO << "Handle create channel " << packet << endl;

    Channel* channel = Global::getNodeFactory()->createChannel();
    
    getConfig()->attachObject( channel, packet->channelID );
    _addChannel( channel );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult eq::Window::_cmdDestroyChannel(eqNet::Command& command ) 
{
    const WindowDestroyChannelPacket* packet =
        command.getPacket<WindowDestroyChannelPacket>();
    EQINFO << "Handle destroy channel " << packet << endl;

    Channel* channel = _findChannel( packet->channelID );
    EQASSERT( channel )

    _removeChannel( channel );
    Config*  config  = getConfig();
    config->detachObject( channel );
    
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult eq::Window::_reqConfigInit( eqNet::Command& command )
{
    const WindowConfigInitPacket* packet = command.getPacket<WindowConfigInitPacket>();
    EQINFO << "handle window configInit " << packet << endl;

    if( packet->pvp.isValid( ))
        _setPixelViewport( packet->pvp );
    else
        _setViewport( packet->vp );
    _name = packet->name;

    for( uint32_t i=0; i<IATTR_ALL; ++i )
        _iAttributes[i] = packet->iattr[i];

    _error.clear();
    WindowConfigInitReplyPacket reply( packet );
    reply.result = configInit( packet->initID );

    RefPtr<eqNet::Node> node = command.getNode();
    if( !reply.result )
    {
        send( node, reply, _error );
        return eqNet::COMMAND_HANDLED;
    }

    const WindowSystem windowSystem = _pipe->getWindowSystem();
    switch( windowSystem )
    {
        case WINDOW_SYSTEM_GLX:
            if( !_xDrawable || !_glXContext )
            {
                EQERROR
                    << "configInit() did not provide a drawable and context" 
                    << endl;
                reply.result = false;
                send( node, reply, _error );
                return eqNet::COMMAND_HANDLED;
            }
            break;

        case WINDOW_SYSTEM_CGL:
            if( !_cglContext )
            {
                EQERROR << "configInit() did not provide an OpenGL context" 
                        << endl;
                reply.result = false;
                send( node, reply, _error );
                return eqNet::COMMAND_HANDLED;
            }
            // TODO: pvp
            break;

        case WINDOW_SYSTEM_WGL:
            if( !_wglWindowHandle || !_wglContext )
            {
                EQERROR << "configInit() did not provide a window handle and"
                        << " context" << endl;
                reply.result = false;
                send( node, reply, _error );
                return eqNet::COMMAND_HANDLED;
            }
            break;

        default: EQUNIMPLEMENTED;
    }

    GLboolean glStereo;
    GLboolean dBuffer;
    glGetBooleanv( GL_STEREO, &glStereo );
    glGetBooleanv( GL_DOUBLEBUFFER, &dBuffer );
    _drawableConfig.doublebuffered = dBuffer;
    _drawableConfig.stereo         = glStereo;

    _initEventHandling();

    reply.pvp            = _pvp;
    reply.drawableConfig = _drawableConfig;
    send( node, reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult eq::Window::_reqConfigExit( eqNet::Command& command )
{
    const WindowConfigExitPacket* packet =
        command.getPacket<WindowConfigExitPacket>();
    EQINFO << "handle window configExit " << packet << endl;

    _exitEventHandling();
    _pipe->testMakeCurrentWindow( this );
    configExit();

    WindowConfigExitReplyPacket reply( packet );
    send( command.getNode(), reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult eq::Window::_reqFrameStart( eqNet::Command& command )
{
    const WindowFrameStartPacket* packet = 
        command.getPacket<WindowFrameStartPacket>();
    EQVERB << "handle window frame start " << packet << endl;

    //_grabFrame( packet->frameNumber ); single-threaded
    _pipe->testMakeCurrentWindow( this );

#ifdef GLX // handle window close request - see comment in configInitGLX
    if( _pipe->getWindowSystem() == WINDOW_SYSTEM_GLX )
    {
        Display*  display = _pipe->getXDisplay();
        Atom   deleteAtom = XInternAtom( display, "WM_DELETE_WINDOW", False );
        WindowEvent event;
        XEvent&     xEvent = event.xEvent;
        while( XCheckTypedEvent( display, ClientMessage, &xEvent ))
        {
            if( xEvent.xany.window == _xDrawable &&
                static_cast<Atom>( xEvent.xclient.data.l[0] ) == 
                    deleteAtom )
            {
                event.window = this;
                event.type   = WindowEvent::CLOSE;
                processEvent( event );
            }
        }
    }
#endif

    frameStart( packet->frameID, packet->frameNumber );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult eq::Window::_reqFrameFinish( eqNet::Command& command )
{
    const WindowFrameFinishPacket* packet =
        command.getPacket<WindowFrameFinishPacket>();
    EQVERB << "handle window frame sync " << packet << endl;

    _pipe->testMakeCurrentWindow( this );
    frameFinish( packet->frameID, packet->frameNumber );

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult eq::Window::_reqFinish(eqNet::Command& command ) 
{
    _pipe->testMakeCurrentWindow( this );
    finish();
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult eq::Window::_reqBarrier( eqNet::Command& command )
{
    const WindowBarrierPacket* packet = 
        command.getPacket<WindowBarrierPacket>();
    EQVERB << "handle barrier " << packet << endl;
    EQLOG( eqNet::LOG_BARRIER ) << "swap barrier " << packet->barrierID
                                << " v" << packet->barrierVersion <<endl;
    
    Node*           node    = getNode();
    eqNet::Barrier* barrier = node->getBarrier( packet->barrierID, 
                                                packet->barrierVersion );

    barrier->enter();
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult eq::Window::_reqSwap(eqNet::Command& command ) 
{
    _pipe->testMakeCurrentWindow( this );
    swapBuffers();
    return eqNet::COMMAND_HANDLED;
}
