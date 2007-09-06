
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
#ifdef AGL
#  include "aglEventHandler.h"
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
        : _eventHandler( 0 ),
          _xDrawable ( 0 ),
          _glXContext( 0 ),
          _aglContext( 0 ),
          _carbonWindow( 0 ),
          _carbonHandler( 0 ),
          _wglWindowHandle( 0 ),
          _wglContext     ( 0 ),
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
    if( _eventHandler )
        EQWARN << "Event handler present in destructor" << endl;
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
        _pvp = pipePVP * vp;
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

        case WINDOW_SYSTEM_AGL:
            if( !configInitAGL( ))
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

    // build attribute list
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
    if( stencilSize >0 || stencilSize == AUTO )
    {
        attributes.push_back( GLX_STENCIL_SIZE );
        attributes.push_back( stencilSize>0 ? stencilSize : 1 );
    }

    if( getIAttribute( IATTR_HINT_STEREO ) != OFF )
        attributes.push_back( GLX_STEREO );
    if( getIAttribute( IATTR_HINT_DOUBLEBUFFER ) != OFF )
        attributes.push_back( GLX_DOUBLEBUFFER );

    attributes.push_back( None );

    // build backoff list, least important attribute last
    vector<int> backoffAttributes;
    if( getIAttribute( IATTR_HINT_DOUBLEBUFFER ) == AUTO )
        backoffAttributes.push_back( GLX_DOUBLEBUFFER );
    if( stencilSize == AUTO )
        backoffAttributes.push_back( GLX_STENCIL_SIZE );
    if( getIAttribute( IATTR_HINT_STEREO ) == AUTO )
        backoffAttributes.push_back( GLX_STEREO );

    // Choose visual
    const int    screen  = DefaultScreen( display );
    XVisualInfo *visInfo = glXChooseVisual( display, screen, 
                                            &attributes.front( ));

    while( !visInfo && !backoffAttributes.empty( ))
    {   // Gradually remove backoff attributes
        const int attribute = backoffAttributes.back();
        backoffAttributes.pop_back();

        vector<int>::iterator iter = find( attributes.begin(), attributes.end(),
                                           attribute );
        EQASSERT( iter != attributes.end( ));
        if( *iter == GLX_STENCIL_SIZE ) // two-elem attribute
            attributes.erase( iter, iter+1 );
        else                            // one-elem attribute
            attributes.erase( iter );

        visInfo = glXChooseVisual( display, screen, &attributes.front( ));
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
    Pipe*          pipe = getPipe();
    Window* firstWindow = pipe->getWindow( 0 );
    GLXContext shareCtx = firstWindow->getGLXContext();
    GLXContext  context = glXCreateContext( display, visInfo, shareCtx, True );

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
    setErrorMessage( "Client library compiled without GLX support" );
    return false;
#endif
}

bool eq::Window::configInitAGL()
{
#ifdef AGL
    CGDirectDisplayID displayID = _pipe->getCGDisplayID();
#ifndef LEOPARD
    GDHandle          displayHandle = 0;
    
    DMGetGDeviceByDisplayID( (DisplayIDType)displayID, &displayHandle, false );
    if( !displayHandle )
        return false;
#endif

    // build attribute list
    vector<GLint> attributes;

    attributes.push_back( AGL_RGBA );
    attributes.push_back( GL_TRUE );
    attributes.push_back( AGL_ACCELERATED );
    attributes.push_back( GL_TRUE );

    if( getIAttribute( IATTR_HINT_FULLSCREEN ) == ON )
        attributes.push_back( AGL_FULLSCREEN );

    const int colorSize = getIAttribute( IATTR_PLANES_COLOR );
    if( colorSize > 0 || colorSize == AUTO )
    {
        const GLint size = colorSize > 0 ? colorSize : 8;

        attributes.push_back( AGL_RED_SIZE );
        attributes.push_back( size );
        attributes.push_back( AGL_GREEN_SIZE );
        attributes.push_back( size );
        attributes.push_back( AGL_BLUE_SIZE );
        attributes.push_back( size );
    }
    const int alphaSize = getIAttribute( IATTR_PLANES_ALPHA );
    if( alphaSize > 0 || alphaSize == AUTO )
    {
        attributes.push_back( AGL_ALPHA_SIZE );
        attributes.push_back( alphaSize>0 ? alphaSize : 8  );
    }
    const int depthSize = getIAttribute( IATTR_PLANES_DEPTH );
    if( depthSize > 0 || depthSize == AUTO )
    {
        attributes.push_back( AGL_DEPTH_SIZE );
        attributes.push_back( depthSize>0 ? depthSize : 24 );
    }
    const int stencilSize = getIAttribute( IATTR_PLANES_STENCIL );
    if( stencilSize > 0 || stencilSize == AUTO )
    {
        attributes.push_back( AGL_STENCIL_SIZE );
        attributes.push_back( stencilSize>0 ? stencilSize : 1 );
    }

    if( getIAttribute( IATTR_HINT_DOUBLEBUFFER ) != OFF )
    {
        attributes.push_back( AGL_DOUBLEBUFFER );
        attributes.push_back( GL_TRUE );
    }
    if( getIAttribute( IATTR_HINT_STEREO ) != OFF )
    {
        attributes.push_back( AGL_STEREO );
        attributes.push_back( GL_TRUE );
    }

    attributes.push_back( AGL_NONE );

    // build backoff list, least important attribute last
    vector<int> backoffAttributes;
    if( getIAttribute( IATTR_HINT_DOUBLEBUFFER ) == AUTO )
        backoffAttributes.push_back( AGL_DOUBLEBUFFER );
    if( stencilSize == AUTO )
        backoffAttributes.push_back( AGL_STENCIL_SIZE );
    if( getIAttribute( IATTR_HINT_STEREO ) == AUTO )
        backoffAttributes.push_back( AGL_STEREO );

    // choose pixel format
    AGLPixelFormat pixelFormat = 0;
    string         error;
    while( true )
    {
#ifndef LEOPARD
        pixelFormat = aglChoosePixelFormat( &displayHandle, 1,
                                            &attributes.front( ));
#else
        pixelFormat = aglChoosePixelFormat( 0, 0, &attributes.front( ));

        if( !pixelFormat )
            error = reinterpret_cast<const char*>(
                aglErrorString( aglGetError( )));

        EQINFO << "PF: " << pixelFormat << endl;
        while( pixelFormat )
        {
            // Find a pixelFormat for our display
            bool               found      = false;
            GLint              nDisplayIDs = 0;
            CGDirectDisplayID *displayIDs = 
                aglDisplaysOfPixelFormat( pixelFormat, &nDisplayIDs );

            if( !displayIDs )
                EQWARN << "aglDisplaysOfPixelFormat failed: "
                       << reinterpret_cast<const char*>(
                           aglErrorString( aglGetError( ))) << endl;
            else
                EQINFO << nDisplayIDs << " displays " << endl;

            for( GLint i = 0; i < nDisplayIDs && !found; ++i )
            {
                EQINFO << displayIDs[i] << "=? " << displayID << endl;
                if( displayIDs[i] == displayID )
                    found = true;
            }

            if( found )
                break;
            
            error = "No matching pixel format on display";

            // None found, try next pixelFormat in list
            pixelFormat = aglNextPixelFormat( pixelFormat );
            EQINFO << "next PF: " << pixelFormat << endl;
        }
#endif // LEOPARD

        if( pixelFormat ||              // found one or
            backoffAttributes.empty( )) // nothing else to try

            break;

        // Gradually remove backoff attributes
        const GLint attribute = backoffAttributes.back();
        backoffAttributes.pop_back();

        vector<GLint>::iterator iter = find( attributes.begin(), 
                                             attributes.end(), attribute );
        EQASSERT( iter != attributes.end( ));

        attributes.erase( iter, iter+1 ); // remove two item (attr, value)
    }

    if( !pixelFormat )
    {
        setErrorMessage( "Could not find a pixel format: " + error );
        return false;
    }

    Pipe*      pipe        = getPipe();
    Window*    firstWindow = pipe->getWindow(0);
    AGLContext shareCtx    = firstWindow->getAGLContext();
    AGLContext context     = aglCreateContext( pixelFormat, shareCtx );
    aglDestroyPixelFormat ( pixelFormat );

    if( !context ) 
    {
        setErrorMessage( "Could not create AGL context: " + aglGetError( ));
        return false;
    }

    // Always enable sync on vertical retrace - TODO should be a window hint
    const GLint interval = 1;
    aglSetInteger( context, AGL_SWAP_INTERVAL, &interval );

    aglSetCurrentContext( context );
    setAGLContext( context );
    EQINFO << "Created AGL context " << context << endl;

    if( getIAttribute( IATTR_HINT_FULLSCREEN ) == ON )
    {
#if 0
        const PixelViewport& pipePVP = _pipe->getPixelViewport();
        const PixelViewport& pvp     = pipePVP.isValid() ? pipePVP : _pvp;
        if( !aglSetFullScreen( context, pvp.w, pvp.h, 0, 0 ))
            EQWARN << "aglSetFullScreen to " << pvp << " failed: " 
                   << aglGetError() << endl;

#else

        if( !aglSetFullScreen( context, 0, 0, 0, 0 ))
            EQWARN << "aglSetFullScreen failed: " << aglGetError()
                   << endl;
#endif
    }
    else // create carbon window and bind drawable to context
    {
        // window
        WindowAttributes attributes = kWindowStandardDocumentAttributes |
                                      kWindowStandardHandlerAttribute   |
                                      kWindowInWindowMenuAttribute;
        Rect             windowRect = { _pvp.y, _pvp.x, // top, left, b, r
                                        _pvp.y + _pvp.h, _pvp.x + _pvp.w };
        WindowRef        windowRef;

        Global::enterCarbon();
        const OSStatus   status     = CreateNewWindow( kDocumentWindowClass, 
                                                       attributes, &windowRect, 
                                                       &windowRef );
        if( status != noErr )
        {
            setErrorMessage( "Could not create carbon window: " + status );
            Global::leaveCarbon();
            return false;
        }

        // window title
        CFStringRef title = 
            CFStringCreateWithCString( kCFAllocatorDefault,
                                    _name.empty() ? "Equalizer" : _name.c_str(),
                                       kCFStringEncodingMacRoman );
        SetWindowTitleWithCFString( windowRef, title );
        CFRelease( title );
        
#ifdef LEOPARD
        if( !aglSetWindowRef( context, windowRef ))
        {
            setErrorMessage( "aglSetWindowRef failed: " + aglGetError( ));
            Global::leaveCarbon();
            return false;
        }
#else
        if( !aglSetDrawable( context, GetWindowPort( windowRef )))
        {
            setErrorMessage( "aglSetDrawable failed: " + aglGetError( ));
            Global::leaveCarbon();
            return false;
        }
#endif

        // show
        ShowWindow( windowRef );
        Global::leaveCarbon();
        setCarbonWindow( windowRef );
    }

    aglSetCurrentContext( context );
    return true;
#else
    setErrorMessage( "Client library compiled without AGL support" );
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
    wc.lpfnWndProc   = DefWindowProc;    
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

    // describe pixel format
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
        EQINFO << "Visual not available, trying mono visual" << endl;
        pfd.dwFlags |= PFD_STEREO_DONTCARE;
        pf = ChoosePixelFormat( dc, &pfd );
    }

    if( pf == 0 && stencilSize == AUTO )
    {        
        EQINFO << "Visual not available, trying non-stencil visual" << endl;
        pfd.cStencilBits = 0;
        pf = ChoosePixelFormat( dc, &pfd );
    }

    if( pf == 0 && getIAttribute( IATTR_HINT_DOUBLEBUFFER ) == AUTO )
    {        
        EQINFO << "Visual not available, trying singlebuffered visual" 
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
    setErrorMessage( "Client library compiled without WGL support" );
    return false;
#endif
}

//----------------------------------------------------------------------
// configExit
//----------------------------------------------------------------------
bool eq::Window::configExit()
{
    const bool         ret          = configExitGL();
    const WindowSystem windowSystem = _pipe->getWindowSystem();

    switch( windowSystem )
    {
        case WINDOW_SYSTEM_GLX:
            configExitGLX();
            return ret;

        case WINDOW_SYSTEM_AGL:
            configExitAGL();
            return ret;

        case WINDOW_SYSTEM_WGL:
            configExitWGL();
            return ret;

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

void eq::Window::configExitAGL()
{
#ifdef AGL
    WindowRef window = getCarbonWindow();
    if( window )
    {
        Global::enterCarbon();
        DisposeWindow( window );
        Global::leaveCarbon();
        setCarbonWindow( 0 );
    }

    AGLContext context = getAGLContext();
    if( context )
    {
#ifdef LEOPARD
        aglSetWindowRef( context, 0 );
#else
        aglSetDrawable( context, 0 );
#endif
        aglSetCurrentContext( 0 );
        aglDestroyContext( context );
        setAGLContext( 0 );
    }
    
    EQINFO << "Destroyed AGL window and context" << endl;
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
    {
        char className[256] = {0};
        GetClassName( hWnd, className, 255 );
        DestroyWindow( hWnd );

        if( strlen( className ) > 0 )
            UnregisterClass( className, GetModuleHandle( 0 ));
    }

    setWGLWindowHandle( 0 );

    if( getIAttribute( IATTR_HINT_FULLSCREEN ) == ON )
        ChangeDisplaySettings( 0, 0 );

    EQINFO << "Destroyed WGL context and window" << endl;
#endif
}

void eq::Window::_queryDrawableConfig()
{
    // GL version
    const char* glVersion = (const char*)glGetString( GL_VERSION );
    if( !glVersion ) // most likely no context - fail
    {
        EQWARN << "glGetString(GL_VERSION) returned 0, assuming GL version 1.1" 
               << endl;
        _drawableConfig.glVersion = 1.1f;
    }
    else
        _drawableConfig.glVersion = static_cast<float>( atof( glVersion ));

    // Framebuffer capabilities
    GLboolean result;
    glGetBooleanv( GL_STEREO,       &result );
    _drawableConfig.stereo = result;

    glGetBooleanv( GL_DOUBLEBUFFER, &result );
    _drawableConfig.doublebuffered = result;

    GLint stencilBits;
    glGetIntegerv( GL_STENCIL_BITS, &stencilBits );
    _drawableConfig.stencilBits = stencilBits;

    GLint alphaBits;
    glGetIntegerv( GL_ALPHA_BITS, &alphaBits );
    _drawableConfig.alphaBits = alphaBits;

#if 0
    // OpenGL Extensions
    const string extList = (const char*)glGetString( GL_EXTENSIONS );
    
    if( extList.find( "GL_EXT_packed_depth_stencil" ) != string::npos ||
        extList.find( "GL_NV_packed_depth_stencil" ) != string::npos )

        _drawableConfig.extPackedDepthStencil = true;
#endif
    EQINFO << "Window drawable config: " << _drawableConfig << endl;
}

void eq::Window::initEventHandler()
{
    EQASSERT( !_eventHandler );
    _eventHandler = EventHandler::registerWindow( this );
}

void eq::Window::exitEventHandler()
{
    if( _eventHandler )
        _eventHandler->deregisterWindow( this );
    _eventHandler = 0;
}

void eq::Window::setXDrawable( XID drawable )
{
#ifdef GLX
    if( _xDrawable == drawable )
        return;

    if( _xDrawable )
        exitEventHandler();
    _xDrawable = drawable;
    if( _xDrawable )
        initEventHandler();

    if( !drawable )
    {
        _pvp.invalidate();
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

void eq::Window::setAGLContext( AGLContext context )
{
#ifdef AGL
    _aglContext = context;
    if( _aglContext )
        _queryDrawableConfig();
#endif // AGL
}

void eq::Window::setGLXContext( GLXContext context )
{
#ifdef GLX
    _glXContext = context;
    if( _glXContext )
        _queryDrawableConfig();
#endif
}

void eq::Window::setWGLContext( HGLRC context )
{
#ifdef WGL
    _wglContext = context; 
    if( _wglContext )
        _queryDrawableConfig();
#endif
}

void eq::Window::setCarbonWindow( WindowRef window )
{
#ifdef AGL
    if( _carbonWindow == window )
        return;

    if( _carbonWindow )
        exitEventHandler();
    _carbonWindow = window;
    if( _carbonWindow )
        initEventHandler();

    _pvp.invalidate();

    if( window )
    {
        Rect rect;
        Global::enterCarbon();
        if( GetWindowBounds( window, kWindowContentRgn, &rect ) == noErr )
        {
            Global::leaveCarbon();
            _pvp.x = rect.left;
            _pvp.y = rect.top;
            _pvp.w = rect.right - rect.left;
            _pvp.h = rect.bottom - rect.top;
        }
        Global::leaveCarbon();
    }
#endif // AGL
}

void eq::Window::setWGLWindowHandle( HWND handle )
{
#ifdef WGL
    if( _wglWindowHandle == handle )
        return;

    if( _wglWindowHandle )
        exitEventHandler();
    _wglWindowHandle = handle;
    if( _wglWindowHandle )
        initEventHandler();

    if( !handle )
    {
        _pvp.invalidate();
        return;
    }

    // query pixel viewport of window
    WINDOWINFO windowInfo;
    windowInfo.cbSize = sizeof( windowInfo );

    GetWindowInfo( handle, &windowInfo );

    _pvp.x = windowInfo.rcClient.left;
    _pvp.y = windowInfo.rcClient.top;
    _pvp.w = windowInfo.rcClient.right  - windowInfo.rcClient.left;
    _pvp.h = windowInfo.rcClient.bottom - windowInfo.rcClient.top;
#endif // WGL
}

void eq::Window::makeCurrent() const
{
    switch( _pipe->getWindowSystem( ))
    {
#ifdef GLX
        case WINDOW_SYSTEM_GLX:
            EQASSERT( _pipe );
            EQASSERT( _pipe->getXDisplay( ));

            glXMakeCurrent( _pipe->getXDisplay(), _xDrawable, _glXContext );
            break;
#endif
#ifdef AGL
        case WINDOW_SYSTEM_AGL:
            aglSetCurrentContext( _aglContext );
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
#ifdef AGL
        case WINDOW_SYSTEM_AGL:
            aglSwapBuffers( _aglContext );
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
#ifdef AGL
            // 'refresh' agl context viewport
            EQASSERT( _pipe );
            if( _aglContext && _pipe->getWindowSystem() == WINDOW_SYSTEM_AGL )
                aglUpdateContext( _aglContext );
#endif
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
    delete channel;

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult eq::Window::_reqConfigInit( eqNet::Command& command )
{
    const WindowConfigInitPacket* packet = 
        command.getPacket<WindowConfigInitPacket>();
    EQLOG( LOG_TASKS ) << "TASK configInit " << getName() <<  " " << packet 
                       << endl;

    if( packet->pvp.isValid( ))
        _setPixelViewport( packet->pvp );
    else
        _setViewport( packet->vp );
    _name = packet->name;

    for( uint32_t i=0; i<IATTR_ALL; ++i )
        _iAttributes[i] = packet->iattr[i];

    _error.clear();
    WindowConfigInitReplyPacket reply;
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
                    << "configInit() did not provide a drawable and/or context" 
                    << endl;
                reply.result = false;
                send( node, reply, _error );
                return eqNet::COMMAND_HANDLED;
            }
            break;

        case WINDOW_SYSTEM_AGL:
            if( !_aglContext )
            {
                EQERROR << "configInit() did not provide an AGL context" 
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

    reply.pvp            = _pvp;
    reply.drawableConfig = _drawableConfig;
    send( node, reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult eq::Window::_reqConfigExit( eqNet::Command& command )
{
    const WindowConfigExitPacket* packet =
        command.getPacket<WindowConfigExitPacket>();
    EQLOG( LOG_TASKS ) << "TASK configExit " << getName() <<  " " << packet 
                       << endl;

    if( _pipe->isInitialized( ))
        _pipe->testMakeCurrentWindow( this );
    // else emergency exit, no context available.

    WindowConfigExitReplyPacket reply;
    reply.result = configExit();

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

std::ostream& eq::operator << ( std::ostream& os, 
                                const Window::DrawableConfig& config )
{
    os << "GL" << config.glVersion;
    if( config.stereo )
        os << "|ST";
    if( config.doublebuffered )
        os << "|DB";
    if( config.stencilBits )
        os << "|st" << config.stencilBits;
    if( config.alphaBits )
        os << "|a" << config.alphaBits;
    return os;
}
