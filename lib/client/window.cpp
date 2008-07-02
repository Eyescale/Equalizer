
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include "windowStatistics.h"

#ifdef GLX
#  include "glXEventHandler.h"
#endif
#ifdef AGL
#  include "aglEventHandler.h"
#endif
#ifdef WGL
#  include "wglEventHandler.h"
#endif

#include <eq/net/barrier.h>
#include <eq/net/command.h>

using namespace eqBase;
using namespace std;
using eqNet::CommandFunc;

namespace eq
{

#define MAKE_ATTR_STRING( attr ) ( string("EQ_WINDOW_") + #attr )
std::string Window::_iAttributeStrings[IATTR_ALL] = {
    MAKE_ATTR_STRING( IATTR_HINT_STEREO ),
    MAKE_ATTR_STRING( IATTR_HINT_DOUBLEBUFFER ),
    MAKE_ATTR_STRING( IATTR_HINT_FULLSCREEN ),
    MAKE_ATTR_STRING( IATTR_HINT_DECORATION ),
    MAKE_ATTR_STRING( IATTR_HINT_SWAPSYNC ),
    MAKE_ATTR_STRING( IATTR_HINT_DRAWABLE ),
    MAKE_ATTR_STRING( IATTR_HINT_STATISTICS ),
    MAKE_ATTR_STRING( IATTR_PLANES_COLOR ),
    MAKE_ATTR_STRING( IATTR_PLANES_ALPHA ),
    MAKE_ATTR_STRING( IATTR_PLANES_DEPTH ),
    MAKE_ATTR_STRING( IATTR_PLANES_STENCIL )
};

Window::Window( Pipe* parent )
        : _eventHandler( 0 )
        , _sharedContextWindow( 0 ) // default set by pipe
        , _glewContext( new GLEWContext )
        , _xDrawable ( 0 )
        , _glXContext( 0 )
        , _aglContext( 0 )
        , _carbonWindow( 0 )
        , _aglPBuffer( 0 )
        , _carbonHandler( 0 )
        , _wglWindow( 0 )
        , _wglPBuffer( 0 )
        , _wglContext( 0 )
        , _wglDC( 0 )
        , _pipe( parent )
        , _renderContextAGLLock( 0 )
{
    eqNet::CommandQueue* queue = parent->getPipeThreadQueue();

    registerCommand( CMD_WINDOW_CREATE_CHANNEL, 
                     CommandFunc<Window>( this, &Window::_cmdCreateChannel ), 
                     queue );
    registerCommand( CMD_WINDOW_DESTROY_CHANNEL,
                     CommandFunc<Window>( this, &Window::_cmdDestroyChannel ), 
                     queue );
    registerCommand( CMD_WINDOW_CONFIG_INIT,
                     CommandFunc<Window>( this, &Window::_cmdConfigInit ), 
                     queue );
    registerCommand( CMD_WINDOW_CONFIG_EXIT, 
                     CommandFunc<Window>( this, &Window::_cmdConfigExit ), 
                     queue );
    registerCommand( CMD_WINDOW_FRAME_START,
                     CommandFunc<Window>( this, &Window::_cmdFrameStart ), 
                     queue );
    registerCommand( CMD_WINDOW_FRAME_FINISH,
                     CommandFunc<Window>( this, &Window::_cmdFrameFinish ), 
                     queue );
    registerCommand( CMD_WINDOW_FINISH, 
                     CommandFunc<Window>( this, &Window::_cmdFinish), queue );
    registerCommand( CMD_WINDOW_BARRIER, 
                     CommandFunc<Window>( this, &Window::_cmdBarrier ), queue );
    registerCommand( CMD_WINDOW_SWAP, 
                     CommandFunc<Window>( this, &Window::_cmdSwap), queue );
    registerCommand( CMD_WINDOW_FRAME_DRAW_FINISH, 
                     CommandFunc<Window>( this, &Window::_cmdFrameDrawFinish ), 
                     queue );

    parent->_addWindow( this );
    EQINFO << " New eq::Window @" << (void*)this << endl;
}


Window::~Window()
{
    _pipe->_removeWindow( this );
    delete _glewContext;
    _glewContext = 0;

    delete _renderContextAGLLock;
    _renderContextAGLLock = 0;

    if( _eventHandler )
        EQWARN << "Event handler present in destructor" << endl;
}

void Window::_addChannel( Channel* channel )
{
    EQASSERT( channel->getWindow() == this );
    _channels.push_back( channel );
}

void Window::_removeChannel( Channel* channel )
{
    ChannelVector::iterator i = find( _channels.begin(), _channels.end(), 
                                      channel );
    EQASSERT( i != _channels.end( ))
    
    _channels.erase( i );
}

Channel* Window::_findChannel( const uint32_t id )
{
    for( ChannelVector::const_iterator i = _channels.begin(); 
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
void Window::setPixelViewport( const PixelViewport& pvp )
{
    if( !_setPixelViewport( pvp ))
        return; // nothing changed

    WindowSetPVPPacket packet;
    packet.pvp = pvp;
    RefPtr< eqNet::Node > node = 
        RefPtr_static_cast< Server, eqNet::Node >( getServer( ));
    send( node, packet );
}

bool Window::_setPixelViewport( const PixelViewport& pvp )
{
    if( pvp == _pvp || !pvp.hasArea( ))
        return false;

    _pvp = pvp;
    _vp.invalidate();

    EQASSERT( _pipe );
    
    const PixelViewport& pipePVP = _pipe->getPixelViewport();
    if( pipePVP.isValid( ))
        _vp = pvp.getSubVP( pipePVP );

    EQINFO << "Window pvp set: " << _pvp << ":" << _vp << endl;
    return true;
}

void Window::_setViewport( const Viewport& vp )
{
    if( !vp.hasArea( ))
        return;
    
    _vp = vp;
    _pvp.invalidate();

    if( !_pipe )
        return;

    PixelViewport pipePVP = _pipe->getPixelViewport();
    if( pipePVP.isValid( ))
        _pvp = pipePVP.getSubPVP( vp );
    EQINFO << "Window vp set: " << _pvp << ":" << _vp << endl;
}

//----------------------------------------------------------------------
// render context
//----------------------------------------------------------------------
void Window::addRenderContext( const RenderContext& context )
{
    // AGL events are dispatched from the main thread, which therefore
    // potentially accesses _renderContexts concurrently with the pipe
    // thread. The _renderContextAGLLock is only active for AGL windows. The
    // proper solution would be to implement our own event queue which
    // dispatches events from the main to the pipe thread, to be handled
    // there. Since the event dispatch is buried in the messagePump, using this
    // lock is easier. Eventually we should implement it, though.
    ScopedMutex< SpinLock > mutex( _renderContextAGLLock );
    _renderContexts[BACK].push_back( context );
}

const RenderContext* Window::getRenderContext( const int32_t x, 
                                               const int32_t y ) const
{
    ScopedMutex< SpinLock > mutex( _renderContextAGLLock );
    const unsigned which = _drawableConfig.doublebuffered ? FRONT : BACK;

    vector< RenderContext >::const_reverse_iterator i   =
        _renderContexts[which].rbegin(); 
    vector< RenderContext >::const_reverse_iterator end =
        _renderContexts[which].rend();

    for( ; i != end; ++i )
    {
        const RenderContext& context = *i;

        if( context.pvp.isPointInside( x, y ))
            return &context;
    }
    return 0;
}


//----------------------------------------------------------------------
// configInit
//----------------------------------------------------------------------
bool Window::configInit( const uint32_t initID )
{
    if( !_pvp.isValid( ))
    {
        setErrorMessage( "Window pixel viewport invalid - pipe init failed?" );
        return false;
    }

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
            EQERROR << "windowing system " << windowSystem << endl;
            return false;
    }

    const bool ret = configInitGL( initID );
    EQ_GL_ERROR( "after Window::configInitGL" );
    return ret;
}

bool Window::configInitGL( const uint32_t initID )
{
    glEnable( GL_SCISSOR_TEST ); // needed to constrain channel viewport
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LESS );

    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );

    glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
    glEnable( GL_COLOR_MATERIAL );

    glClearDepth( 1.f );
    //glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );

    glClear( GL_COLOR_BUFFER_BIT );
    swapBuffers();
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    return true;
}

//---------------------------------------------------------------------------
// GLX init
//---------------------------------------------------------------------------

#ifdef GLX
static Bool WaitForNotify( Display*, XEvent *e, char *arg )
{ return (e->type == MapNotify) && (e->xmap.window == (::Window)arg); }
#endif

bool Window::configInitGLX()
{
#ifdef GLX
    XVisualInfo* visualInfo = chooseXVisualInfo();
    if( !visualInfo )
        return false;

    GLXContext context = createGLXContext( visualInfo );
    setGLXContext( context );

    if( !context )
        return false;

    const bool success = configInitGLXDrawable( visualInfo );
    XFree( visualInfo );

    if( success && !_xDrawable )
    {
        setErrorMessage( "configInitGLXDrawable did set no X11 drawable" );
        return false;
    }

    return success;    
#else
    setErrorMessage( "Client library compiled without GLX support" );
    return false;
#endif
}

XVisualInfo* Window::chooseXVisualInfo()
{
#ifdef GLX
    Display* display = _pipe->getXDisplay();
    if( !display )
    {
        setErrorMessage( "Pipe has no X11 display connection" );
        return 0;
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
    if( stencilSize > 0 || stencilSize == AUTO )
    {
        attributes.push_back( GLX_STENCIL_SIZE );
        attributes.push_back( stencilSize>0 ? stencilSize : 1 );
    }

#ifdef DARWIN
    // WAR: glDrawBuffer( GL_BACK ) renders only to the left back buffer on a
    // stereo visual on Darwin which creates ugly flickering on mono configs
    if( getIAttribute( IATTR_HINT_STEREO ) == ON )
        attributes.push_back( GLX_STEREO );
#else
    if( getIAttribute( IATTR_HINT_STEREO ) == ON ||
        ( getIAttribute( IATTR_HINT_STEREO )   == AUTO && 
          getIAttribute( IATTR_HINT_DRAWABLE ) == WINDOW ))

        attributes.push_back( GLX_STEREO );
#endif
    if( getIAttribute( IATTR_HINT_DOUBLEBUFFER ) == ON ||
        ( getIAttribute( IATTR_HINT_DOUBLEBUFFER ) == AUTO && 
          getIAttribute( IATTR_HINT_DRAWABLE )     == WINDOW ))

        attributes.push_back( GLX_DOUBLEBUFFER );

    attributes.push_back( None );

    // build backoff list, least important attribute last
    vector<int> backoffAttributes;
    if( getIAttribute( IATTR_HINT_DRAWABLE ) == WINDOW )
    {
        if( getIAttribute( IATTR_HINT_DOUBLEBUFFER ) == AUTO )
            backoffAttributes.push_back( GLX_DOUBLEBUFFER );

#ifndef DARWIN
        if( getIAttribute( IATTR_HINT_STEREO ) == AUTO )
            backoffAttributes.push_back( GLX_STEREO );
#endif
    }

    if( stencilSize == AUTO )
        backoffAttributes.push_back( GLX_STENCIL_SIZE );


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
            attributes.erase( iter, iter+2 );
        else                            // one-elem attribute
            attributes.erase( iter );

        visInfo = glXChooseVisual( display, screen, &attributes.front( ));
    }

    if ( !visInfo )
        setErrorMessage( "Could not find a matching visual" );
    else
        EQINFO << "Using visual 0x" << std::hex << visInfo->visualid
               << std::dec << endl;

    return visInfo;
#else
    setErrorMessage( "Client library compiled without GLX support" );
    return 0;
#endif
}

GLXContext Window::createGLXContext( XVisualInfo* visualInfo )
{
#ifdef GLX
    if( !visualInfo )
    {
        setErrorMessage( "No visual info given" );
        return 0;
    }

    Pipe*    pipe    = getPipe();
    Display* display = pipe->getXDisplay();
    if( !display )
    {
        setErrorMessage( "Pipe has no X11 display connection" );
        return 0;
    }

    Window* firstWindow = pipe->getWindows()[0];
    GLXContext shareCtx = firstWindow->getGLXContext();
    GLXContext  context = glXCreateContext(display, visualInfo, shareCtx, True);

    if ( !context )
    {
        setErrorMessage( "Could not create OpenGL context" );
        return 0;
    }

    return context;
#else
    setErrorMessage( "Client library compiled without GLX support" );
    return 0;
#endif
}

bool Window::configInitGLXDrawable( XVisualInfo* visualInfo )
{
    switch( getIAttribute( IATTR_HINT_DRAWABLE ))
    {
        case PBUFFER:
			return configInitGLXPBuffer( visualInfo );

        default:
            EQWARN << "Unknown drawable type " 
                   << getIAttribute(IATTR_HINT_DRAWABLE ) << ", using window" 
                   << endl;
            // no break;
		case UNDEFINED:
        case WINDOW:
            return configInitGLXWindow( visualInfo );
    }
}

bool Window::configInitGLXWindow( XVisualInfo* visualInfo )
{
#ifdef GLX
    EQASSERT( getIAttribute( IATTR_HINT_DRAWABLE ) != PBUFFER )

    if( !visualInfo )
    {
        setErrorMessage( "No visual info given" );
        return false;
    }

    Pipe*    pipe    = getPipe();
    Display* display = pipe->getXDisplay();
    if( !display )
    {
        setErrorMessage( "Pipe has no X11 display connection" );
        return false;
    }

    const int            screen = DefaultScreen( display );
    XID                  parent = RootWindow( display, screen );
    XSetWindowAttributes wa;
    wa.colormap          = XCreateColormap( display, parent, visualInfo->visual,
                                            AllocNone );
    wa.background_pixmap = None;
    wa.border_pixel      = 0;
    wa.event_mask        = StructureNotifyMask | VisibilityChangeMask |
                           ExposureMask | KeyPressMask | KeyReleaseMask |
                           PointerMotionMask | ButtonPressMask |
                           ButtonReleaseMask;

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
                                  0, visualInfo->depth, InputOutput,
                                  visualInfo->visual, 
                                  CWBackPixmap | CWBorderPixel |
                                  CWEventMask | CWColormap | CWOverrideRedirect,
                                  &wa );
    
    if ( !drawable )
    {
        setErrorMessage( "Could not create window" );
        return false;
    }   
   
    stringstream windowTitle;
    if( _name.empty( ))
    {
        windowTitle << "Equalizer";
#ifndef NDEBUG
        windowTitle << " (" << getpid() << ")";
#endif
    }
    else
        windowTitle << _name;

    XStoreName( display, drawable, windowTitle.str().c_str( ));

    // Register for close window request from the window manager
    Atom deleteAtom = XInternAtom( display, "WM_DELETE_WINDOW", False );
    XSetWMProtocols( display, drawable, &deleteAtom, 1 );

    // map and wait for MapNotify event
    XMapWindow( display, drawable );

    XEvent event;
    XIfEvent( display, &event, WaitForNotify, (XPointer)(drawable) );

    XMoveResizeWindow( display, drawable, _pvp.x, _pvp.y, _pvp.w, _pvp.h );
    XFlush( display );

    // Grab keyboard focus in fullscreen mode
    if( getIAttribute( Window::IATTR_HINT_FULLSCREEN ) == ON )
        XGrabKeyboard( display, drawable, True, GrabModeAsync, GrabModeAsync, 
                       CurrentTime );

    setXDrawable( drawable );

    EQINFO << "Created X11 drawable " << drawable << endl;
    return true;
#else
    setErrorMessage( "Client library compiled without GLX support" );
    return false;
#endif
}

bool Window::configInitGLXPBuffer( XVisualInfo* visualInfo )
{
#ifdef GLX
    EQASSERT( getIAttribute( IATTR_HINT_DRAWABLE ) == PBUFFER )

    if( !visualInfo )
    {
        setErrorMessage( "No visual info given" );
        return false;
    }
    
    Pipe*    pipe    = getPipe();
    Display* display = pipe->getXDisplay();
    if( !display )
    {
        setErrorMessage( "Pipe has no X11 display connection" );
        return false;
    }

    // Check for GLX >= 1.3
    int major = 0;
    int minor = 0;
    if( !glXQueryVersion( display, &major, &minor ))
    {
        setErrorMessage( "Can't get GLX version" );
        return false;
    }

    if( major < 1 || (major == 1 && minor < 3 ))
    {
        setErrorMessage( "Need at least GLX 1.3" );
        return false;
    }

    // Find FB config for X visual
    const int    screen   = DefaultScreen( display );
    int          nConfigs = 0;
    GLXFBConfig* configs  = glXGetFBConfigs( display, screen, &nConfigs );
    GLXFBConfig  config   = 0;

    for( int i = 0; i < nConfigs; ++i )
    {
        int visualID;
        if( glXGetFBConfigAttrib( display, configs[i], GLX_VISUAL_ID, 
                                  &visualID ) == 0 )
        {
            if( visualID == static_cast< int >( visualInfo->visualid ))
            {
                config = configs[i];
                break;
	        }
	    }
    }

    if( !config )
    {
        setErrorMessage( "Can't find FBConfig for visual" );
        return false;
    }

    // Create PBuffer
    const int attributes[] = { GLX_PBUFFER_WIDTH, _pvp.w,
                               GLX_PBUFFER_HEIGHT, _pvp.h,
                               GLX_LARGEST_PBUFFER, True,
                               GLX_PRESERVED_CONTENTS, True,
                               0 };

    XID pbuffer = glXCreatePbuffer( display, config, attributes );
    if ( !pbuffer )
    {
        setErrorMessage( "Could not create PBuffer" );
        return false;
    }   
   
    XFlush( display );
    setXDrawable( pbuffer );

    EQINFO << "Created X11 PBuffer " << pbuffer << endl;
    return true;
#else
    setErrorMessage( "Client library compiled without GLX support" );
    return false;
#endif
}

//---------------------------------------------------------------------------
// AGL init
//---------------------------------------------------------------------------

bool Window::configInitAGL()
{
    AGLPixelFormat pixelFormat = chooseAGLPixelFormat();
    if( !pixelFormat )
        return false;

    AGLContext context = createAGLContext( pixelFormat );
    destroyAGLPixelFormat ( pixelFormat );
    setAGLContext( context );

    if( !context )
        return false;

    return configInitAGLDrawable();
}

AGLPixelFormat Window::chooseAGLPixelFormat()
{
#ifdef AGL
    CGDirectDisplayID displayID = _pipe->getCGDisplayID();

    Global::enterCarbon();

#ifdef LEOPARD
    CGOpenGLDisplayMask glDisplayMask =
        CGDisplayIDToOpenGLDisplayMask( displayID );
#else
    GDHandle          displayHandle = 0;

    DMGetGDeviceByDisplayID( (DisplayIDType)displayID, &displayHandle, false );

    if( !displayHandle )
    {
        setErrorMessage( "Can't get display handle" );
        Global::leaveCarbon();
        return 0;
    }
#endif

    // build attribute list
    vector<GLint> attributes;

    attributes.push_back( AGL_RGBA );
    attributes.push_back( GL_TRUE );
    attributes.push_back( AGL_ACCELERATED );
    attributes.push_back( GL_TRUE );

    if( getIAttribute( IATTR_HINT_FULLSCREEN ) == ON && 
        getIAttribute( IATTR_HINT_DRAWABLE )   == WINDOW )

        attributes.push_back( AGL_FULLSCREEN );

#ifdef LEOPARD
    attributes.push_back( AGL_DISPLAY_MASK );
    attributes.push_back( glDisplayMask );
#endif

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

    if( getIAttribute( IATTR_HINT_DOUBLEBUFFER ) == ON ||
        ( getIAttribute( IATTR_HINT_DOUBLEBUFFER ) == AUTO && 
          getIAttribute( IATTR_HINT_DRAWABLE )     == WINDOW ))
    {
        attributes.push_back( AGL_DOUBLEBUFFER );
        attributes.push_back( GL_TRUE );
    }
    if( getIAttribute( IATTR_HINT_STEREO ) == ON )
    {
        attributes.push_back( AGL_STEREO );
        attributes.push_back( GL_TRUE );
    }

    attributes.push_back( AGL_NONE );

    // build backoff list, least important attribute last
    vector<int> backoffAttributes;
    if( getIAttribute( IATTR_HINT_DOUBLEBUFFER ) == AUTO &&
        getIAttribute( IATTR_HINT_DRAWABLE )     == WINDOW  )

        backoffAttributes.push_back( AGL_DOUBLEBUFFER );

    if( stencilSize == AUTO )
        backoffAttributes.push_back( AGL_STENCIL_SIZE );

    // choose pixel format
    AGLPixelFormat pixelFormat = 0;
    while( true )
    {
#ifdef LEOPARD
        pixelFormat = aglCreatePixelFormat( &attributes.front( ));
#else
        pixelFormat = aglChoosePixelFormat( &displayHandle, 1,
                                            &attributes.front( ));
#endif

        if( pixelFormat ||              // found one or
            backoffAttributes.empty( )) // nothing else to try

            break;

        // Gradually remove backoff attributes
        const GLint attribute = backoffAttributes.back();
        backoffAttributes.pop_back();

        vector<GLint>::iterator iter = find( attributes.begin(), 
                                             attributes.end(), attribute );
        EQASSERT( iter != attributes.end( ));

        attributes.erase( iter, iter+2 ); // remove two item (attr, value)
    }

    if( !pixelFormat )
        setErrorMessage( "Could not find a matching pixel format" );

    Global::leaveCarbon();
    return pixelFormat;
#else
    setErrorMessage( "Client library compiled without AGL support" );
    return 0;
#endif
}

void Window::destroyAGLPixelFormat( AGLPixelFormat pixelFormat )
{
#ifdef AGL
    if( !pixelFormat )
        return;

    Global::enterCarbon();
    aglDestroyPixelFormat( pixelFormat );
    Global::leaveCarbon();
#else
    setErrorMessage( "Client library compiled without AGL support" );
#endif
}

AGLContext Window::createAGLContext( AGLPixelFormat pixelFormat )
{
#ifdef AGL
    if( !pixelFormat )
    {
        setErrorMessage( "No pixel format given" );
        return 0;
    }

    Pipe*      pipe        = getPipe();
    Window*    firstWindow = pipe->getWindows()[0];
    AGLContext shareCtx    = firstWindow->getAGLContext();
 
    Global::enterCarbon();
    AGLContext context     = aglCreateContext( pixelFormat, shareCtx );

    if( !context ) 
    {
        setErrorMessage( "Could not create AGL context: " + aglGetError( ));
        Global::leaveCarbon();
        return 0;
    }

    // set vsync on/off
    if( getIAttribute( IATTR_HINT_SWAPSYNC ) != AUTO )
    {
        const GLint vsync = ( getIAttribute( IATTR_HINT_SWAPSYNC )==OFF ) ? 0 : 1;
        aglSetInteger( context, AGL_SWAP_INTERVAL, &vsync );
    }

    aglSetCurrentContext( context );

    Global::leaveCarbon();

    EQINFO << "Created AGL context " << context << endl;
    return context;
#else
    setErrorMessage( "Client library compiled without AGL support" );
    return 0;
#endif
}


bool Window::configInitAGLDrawable()
{
    if( getIAttribute( IATTR_HINT_DRAWABLE ) == PBUFFER )
        return configInitAGLPBuffer();
    if( getIAttribute( IATTR_HINT_FULLSCREEN ) == ON )
        return configInitAGLFullscreen();
    else
        return configInitAGLWindow();
}

bool Window::configInitAGLPBuffer()
{
#ifdef AGL
    AGLContext context = getAGLContext();
    if( !context )
    {
        setErrorMessage( "No AGLContext set" );
        return false;
    }

    // PBuffer
    AGLPbuffer pbuffer;
    if( !aglCreatePBuffer( _pvp.w, _pvp.h, GL_TEXTURE_RECTANGLE_EXT, GL_RGBA,
                           0, &pbuffer ))
    {
        setErrorMessage( "Could not create PBuffer: " + aglGetError( ));
        return false;
    }

    // attach to context
    if( !aglSetPBuffer( context, pbuffer, 0, 0, aglGetVirtualScreen( context )))
    {
        setErrorMessage( "aglSetPBuffer failed: " + aglGetError( ));
        return false;
    }

    setAGLPBuffer( pbuffer );
    return true;
#else
    setErrorMessage( "Client library compiled without AGL support" );
    return false;
#endif
}

bool Window::configInitAGLFullscreen()
{
#ifdef AGL
    AGLContext context = getAGLContext();
    if( !context )
    {
        setErrorMessage( "No AGLContext set" );
        return false;
    }

    Global::enterCarbon();

    aglEnable( context, AGL_FS_CAPTURE_SINGLE );

    const PixelViewport& pipePVP = _pipe->getPixelViewport();
    const PixelViewport& pvp     = pipePVP.isValid() ? pipePVP : _pvp;

#if 1
    if( !aglSetFullScreen( context, pvp.w, pvp.h, 0, 0 ))
        EQWARN << "aglSetFullScreen to " << pvp << " failed: " << aglGetError()
               << endl;
#else
    if( !aglSetFullScreen( context, 0, 0, 0, 0 ))
        EQWARN << "aglSetFullScreen failed: " << aglGetError() << endl;
#endif

    Global::leaveCarbon();
    setPixelViewport( pvp );
    return true;
#else
    setErrorMessage( "Client library compiled without AGL support" );
    return false;
#endif
}

bool Window::configInitAGLWindow()
{
#ifdef AGL
    AGLContext context = getAGLContext();
    if( !context )
    {
        setErrorMessage( "No AGLContext set" );
        return false;
    }

    // window
    WindowAttributes winAttributes = kWindowStandardDocumentAttributes |
                                     kWindowStandardHandlerAttribute   |
                                     kWindowInWindowMenuAttribute;
    // top, left, bottom, right
    const bool     decoration = (getIAttribute( IATTR_HINT_DECORATION ) != OFF);
    const int32_t  menuHeight = decoration ? EQ_AGL_MENUBARHEIGHT : 0 ;
    Rect           windowRect = { _pvp.y + menuHeight, _pvp.x, 
                                 _pvp.y + _pvp.h + menuHeight,
                                 _pvp.x + _pvp.w };
    WindowRef      windowRef;

    Global::enterCarbon();
    const OSStatus status     = CreateNewWindow( kDocumentWindowClass, 
                                                 winAttributes,
                                                 &windowRect, &windowRef );
    if( status != noErr )
    {
        setErrorMessage( "Could not create carbon window: " + status );
        Global::leaveCarbon();
        return false;
    }

    // window title
    stringstream windowTitle;
    if( _name.empty( ))
    {
        windowTitle << "Equalizer";
#ifndef NDEBUG
        windowTitle << " (" << getpid() << ")";
#endif;
    }
    else
        windowTitle << _name;

    CFStringRef title = CFStringCreateWithCString( kCFAllocatorDefault,
                                                   windowTitle.str().c_str(),
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

    return true;
#else
    setErrorMessage( "Client library compiled without AGL support" );
    return false;
#endif
}

//---------------------------------------------------------------------------
// WGL init
//---------------------------------------------------------------------------
bool Window::configInitWGL()
{
#ifdef WGL
    PFNEQDELETEDCPROC deleteDCProc = 0;
    HDC dc = getWGLPipeDC( deleteDCProc );
    EQASSERT( !dc || deleteDCProc );

    int pixelFormat = chooseWGLPixelFormat( dc );
    if( pixelFormat == 0 )
    {
        if( dc )
            deleteDCProc( dc );
        return false;
    }

    if( !configInitWGLDrawable( dc, pixelFormat ))
    {
        if( dc )
            deleteDCProc( dc );
        return false;
    }

    if( !_wglDC )
    {
        if( dc )
            deleteDCProc( dc );
        setErrorMessage( "configInitWGLDrawable did not set a WGL drawable" );
        return false;
    }

    HGLRC context = createWGLContext( dc );
    setWGLContext( context );

    if( getIAttribute( IATTR_HINT_SWAPSYNC ) != AUTO )
    {
        if( WGLEW_EXT_swap_control )
        {
            // set vsync on/off
            const GLint vsync = ( getIAttribute( IATTR_HINT_SWAPSYNC )==OFF ) ?
                                    0 : 1;
            wglSwapIntervalEXT( vsync );
        }
        else
            EQWARN << "WGLEW_EXT_swap_control not supported, ignoring window "
                   << "swapsync hint" << endl;
    }

    if( !context )
    {
        if( dc )
            deleteDCProc( dc );
        return false;
    }

    if( dc )
        deleteDCProc( dc );
    return true;
#else
    setErrorMessage( "Client library compiled without WGL support" );
    return false;
#endif
}

bool Window::configInitWGLDrawable( HDC dc, int pixelFormat )
{
    switch( getIAttribute( IATTR_HINT_DRAWABLE ))
    {
        case PBUFFER:
            return configInitWGLPBuffer( dc, pixelFormat );

        default:
            EQWARN << "Unknown drawable type "
                   << getIAttribute(IATTR_HINT_DRAWABLE )
                   << ", creating a window" << endl;
            // no break;
		case UNDEFINED:
        case WINDOW:
            return configInitWGLWindow( dc, pixelFormat );
    }
}

bool Window::configInitWGLWindow( HDC dc, int pixelFormat )
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
    wc.hIcon         = LoadIcon( 0, IDI_WINLOGO );
    wc.hCursor       = LoadCursor( 0, IDC_ARROW );
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

    // adjust window size (adds border pixels)
    RECT rect;
    rect.left   = _pvp.x;
    rect.top    = _pvp.y;
    rect.right  = _pvp.x + _pvp.w;
    rect.bottom = _pvp.y + _pvp.h;
    AdjustWindowRectEx( &rect, windowStyle, FALSE, windowStyleEx );

    HWND hWnd = CreateWindowEx( windowStyleEx,
                                wc.lpszClassName, 
                                _name.empty() ? "Equalizer" : _name.c_str(),
                                windowStyle, rect.left, rect.top, 
                                rect.right - rect.left, rect.bottom - rect.top,
                                0, 0, // parent, menu
                                instance, 0 );
    if( !hWnd )
    {
        setErrorMessage( "Can't create window: " + 
                         getErrorString( GetLastError( )));
        return false;
    }

    HDC windowDC = GetDC( hWnd );

    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize        = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion     = 1;

    DescribePixelFormat( dc ? dc : windowDC, pixelFormat, sizeof(pfd), &pfd );
    if( !SetPixelFormat( windowDC, pixelFormat, &pfd ))
    {
        ReleaseDC( hWnd, windowDC );
        setErrorMessage( "Can't set window pixel format: " + 
            getErrorString( GetLastError( )));
        return false;
    }
    ReleaseDC( hWnd, windowDC );

    setWGLWindowHandle( hWnd );
    ShowWindow( hWnd, SW_SHOW );
    UpdateWindow( hWnd );

    return true;
#else
    setErrorMessage( "Client library compiled without WGL support" );
    return false;
#endif
}

bool Window::configInitWGLPBuffer( HDC overrideDC, int pixelFormat )
{
#ifdef WGL
    if( !WGLEW_ARB_pbuffer )
    {
        setErrorMessage( "WGL_ARB_pbuffer not supported" );
        return false;
    }

    const eq::PixelViewport& pvp = getPixelViewport();
    EQASSERT( pvp.isValid( ));

    HDC displayDC = CreateDC( "DISPLAY", 0, 0, 0 );
    HDC dc       = overrideDC ? overrideDC : displayDC;
    const int attributes[] = { WGL_PBUFFER_LARGEST_ARB, TRUE, 0 };

    HPBUFFERARB pBuffer = wglCreatePbufferARB( dc, pixelFormat, pvp.w, pvp.h,
                                               attributes );
    if( !pBuffer )
    {
        setErrorMessage( "Can't create PBuffer: " + 
            getErrorString( GetLastError( )));

        DeleteDC( displayDC );
        return false;
    }

    DeleteDC( displayDC );

    setWGLPBufferHandle( pBuffer );
    return true;
#else
    setErrorMessage( "Client library compiled without WGL support" );
    return false;
#endif
}

HDC Window::getWGLPipeDC( PFNEQDELETEDCPROC& deleteProc )
{
#ifdef WGL
    // per-GPU affinity DC
    // We need to create one DC per window, since the window DC pixel format and
    // the affinity RC pixel format have to match, and each window has
    // potentially a different pixel format.
    HDC affinityDC = 0;
    if( !_pipe->createAffinityDC( affinityDC, deleteProc ))
    {
        setErrorMessage( "Can't create affinity dc" );
        return 0;
    }

    return affinityDC;
#else
    setErrorMessage( "Client library compiled without WGL support" );
    return 0;
#endif
}

int Window::chooseWGLPixelFormat( HDC dc )
{
#ifdef WGL
    EQASSERT( WGLEW_ARB_pixel_format );

    vector< int > attributes;
    attributes.push_back( WGL_SUPPORT_OPENGL_ARB );
    attributes.push_back( 1 );
    attributes.push_back( WGL_ACCELERATION_ARB );
    attributes.push_back( WGL_FULL_ACCELERATION_ARB );

    const int colorSize = getIAttribute( IATTR_PLANES_COLOR );
    if( colorSize > 0 || colorSize == AUTO )
    {
        const int colorBits = colorSize>0 ? colorSize : 8;
        attributes.push_back( WGL_COLOR_BITS_ARB );
        attributes.push_back( colorBits * 3 );
    }

    const int alphaSize = getIAttribute( IATTR_PLANES_ALPHA );
    if( alphaSize > 0 || alphaSize == AUTO )
    {
        attributes.push_back( WGL_ALPHA_BITS_ARB );
        attributes.push_back( alphaSize>0 ? alphaSize : 8 );
    }

    const int depthSize = getIAttribute( IATTR_PLANES_DEPTH );
    if( depthSize > 0  || depthSize == AUTO )
    {
        attributes.push_back( WGL_DEPTH_BITS_ARB );
        attributes.push_back( depthSize>0 ? depthSize : 24 );
    }

    const int stencilSize = getIAttribute( IATTR_PLANES_STENCIL );
    if( stencilSize >0 || stencilSize == AUTO )
    {
        attributes.push_back( WGL_STENCIL_BITS_ARB );
        attributes.push_back( stencilSize>0 ? stencilSize : 1 );
    }

    if( getIAttribute( IATTR_HINT_STEREO ) == ON ||
        ( getIAttribute( IATTR_HINT_STEREO )   == AUTO && 
          getIAttribute( IATTR_HINT_DRAWABLE ) == WINDOW ))
    {
        attributes.push_back( WGL_STEREO_ARB );
        attributes.push_back( 1 );
    }

    if( getIAttribute( IATTR_HINT_DOUBLEBUFFER ) == ON ||
        ( getIAttribute( IATTR_HINT_DOUBLEBUFFER ) == AUTO && 
          getIAttribute( IATTR_HINT_DRAWABLE )     == WINDOW ))
    {
        attributes.push_back( WGL_DOUBLE_BUFFER_ARB );
        attributes.push_back( 1 );
    }

    if( getIAttribute( IATTR_HINT_DRAWABLE ) == PBUFFER &&
        WGLEW_ARB_pbuffer )
    {
        attributes.push_back( WGL_DRAW_TO_PBUFFER_ARB );
        attributes.push_back( 1 );
    }
    else
    {
        attributes.push_back( WGL_DRAW_TO_WINDOW_ARB );
        attributes.push_back( 1 );
    }

    attributes.push_back( 0 );

    // build back off list, least important attribute last
    vector<int> backoffAttributes;
    if( getIAttribute( IATTR_HINT_DRAWABLE ) == WINDOW )
    {
        if( getIAttribute( IATTR_HINT_DOUBLEBUFFER ) == AUTO )
            backoffAttributes.push_back( WGL_DOUBLE_BUFFER_ARB );

        if( getIAttribute( IATTR_HINT_STEREO ) == AUTO )
            backoffAttributes.push_back( WGL_STEREO_ARB );
    }

    if( stencilSize == AUTO )
        backoffAttributes.push_back( WGL_STENCIL_BITS_ARB );

    HDC screenDC    = GetDC( 0 );
    HDC pfDC        = dc ? dc : screenDC;
    int pixelFormat = 0;

    while( true )
    {
        UINT nFormats = 0;
        if( !wglChoosePixelFormatARB( pfDC, &attributes[0], 0, 1,
                                      &pixelFormat, &nFormats ))
        {
            EQWARN << "wglChoosePixelFormat failed: " 
                   << getErrorString( GetLastError( )) << endl;
        }

        if( (pixelFormat && nFormats > 0) ||  // found one or
            backoffAttributes.empty( ))       // nothing else to try

            break;

        // Gradually remove back off attributes
        const int attribute = backoffAttributes.back();
        backoffAttributes.pop_back();

        vector<GLint>::iterator iter = find( attributes.begin(), 
            attributes.end(), attribute );
        EQASSERT( iter != attributes.end( ));

        attributes.erase( iter, iter+2 ); // remove two items (attr, value)

    }

    ReleaseDC( 0, screenDC );

    if( pixelFormat == 0 )
    {
        setErrorMessage( "Can't find matching pixel format: " + 
                         getErrorString( GetLastError( )));
        return 0;
    }
 
    if( dc ) // set pixel format on given device context
    {
        PIXELFORMATDESCRIPTOR pfd = {0};
        pfd.nSize        = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion     = 1;

        DescribePixelFormat( dc, pixelFormat, sizeof(pfd), &pfd );
        if( !SetPixelFormat( dc, pixelFormat, &pfd ))
        {
            setErrorMessage( "Can't set device pixel format: " + 
                             getErrorString( GetLastError( )));
            return 0;
        }
    }
    
    return pixelFormat;
#else
    setErrorMessage( "Client library compiled without WGL support" );
    return 0;
#endif
}

HGLRC Window::createWGLContext( HDC overrideDC )
{
#ifdef WGL
    HDC dc = overrideDC ? overrideDC : _wglDC;
    EQASSERT( dc );

    // create context
    HGLRC context = wglCreateContext( dc );
    if( !context )
    {
        setErrorMessage( "Can't create OpenGL context: " + 
                         getErrorString( GetLastError( )));
        return 0;
    }

    // share context
    Pipe*    pipe        = getPipe();
    Window*  firstWindow = pipe->getWindows()[0];
    if( firstWindow == this )
        return context;

    HGLRC    shareCtx    = firstWindow->getWGLContext();

    if( shareCtx && !wglShareLists( shareCtx, context ))
        EQWARN << "Context sharing failed: " << getErrorString( GetLastError( ))
               << endl;

    return context;
#else
    setErrorMessage( "Client library compiled without WGL support" );
    return 0;
#endif
}

//----------------------------------------------------------------------
// configExit
//----------------------------------------------------------------------
bool Window::configExit()
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
            EQWARN << "windowing system " << windowSystem << endl;
            return false;
    }
}

void Window::configExitGLX()
{
#ifdef GLX
    Display *display = _pipe->getXDisplay();
    if( !display ) 
        return;

    glXMakeCurrent( display, None, 0 );

    GLXContext context  = getGLXContext();
    XID        drawable = getXDrawable();

    setGLXContext( 0 );
    setXDrawable( 0 );

    if( context )
        glXDestroyContext( display, context );

    if( drawable )
    {
        if( getIAttribute( IATTR_HINT_DRAWABLE ) == PBUFFER )
            glXDestroyPbuffer( display, drawable );
        else
            XDestroyWindow( display, drawable );
    }

    EQINFO << "Destroyed GLX context and X drawable " << endl;
#endif
}

void Window::configExitAGL()
{
#ifdef AGL
    WindowRef window = getCarbonWindow();
    setCarbonWindow( 0 );

    AGLPbuffer pbuffer = getAGLPBuffer();
    setAGLPBuffer( 0 );

    if( window )
    {
        Global::enterCarbon();
        DisposeWindow( window );
        Global::leaveCarbon();
    }
    if( pbuffer )
        aglDestroyPBuffer( pbuffer );

    AGLContext context = getAGLContext();
    if( context )
    {
        Global::enterCarbon();
        if( getIAttribute( IATTR_HINT_FULLSCREEN ) != ON )
        {
#ifdef LEOPARD
            aglSetWindowRef( context, 0 );
#else
            aglSetDrawable( context, 0 );
#endif
        }

        aglSetCurrentContext( 0 );
        aglDestroyContext( context );
        Global::leaveCarbon();
        setAGLContext( 0 );
    }
    
    EQINFO << "Destroyed AGL window and context" << endl;
#endif
}

void Window::configExitWGL()
{
#ifdef WGL
    wglMakeCurrent( 0, 0 );

    HGLRC context        = getWGLContext();
    HWND  hWnd           = getWGLWindowHandle();
    HPBUFFERARB hPBuffer = getWGLPBufferHandle();

    setWGLContext( 0 );
    setWGLWindowHandle( 0 );
    setWGLPBufferHandle( 0 );

    if( context )
        wglDeleteContext( context );

    if( hWnd )
    {
        char className[256] = {0};
        GetClassName( hWnd, className, 255 );
        DestroyWindow( hWnd );

        if( strlen( className ) > 0 )
            UnregisterClass( className, GetModuleHandle( 0 ));
    }
    if( hPBuffer )
        wglDestroyPbufferARB( hPBuffer );

    if( getIAttribute( IATTR_HINT_FULLSCREEN ) == ON )
        ChangeDisplaySettings( 0, 0 );

    EQINFO << "Destroyed WGL context and window" << endl;
#endif
}

void Window::_queryDrawableConfig()
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

    EQINFO << "Window drawable config: " << _drawableConfig << endl;
}

void Window::_initializeGLData()
{
    makeCurrent();
    _queryDrawableConfig();
    const GLenum result = glewInit();
    if( result != GLEW_OK )
        EQWARN << "GLEW initialization failed with error " << result <<endl;

    _setupObjectManager();
}

void Window::_clearGLData()
{
    _releaseObjectManager();
}

void Window::_setupObjectManager()
{
    _releaseObjectManager();

    if( _sharedContextWindow )
        _objectManager = _sharedContextWindow->getObjectManager();
    
    if( !_objectManager.isValid( ))
        _objectManager = new ObjectManager( _glewContext );
}

void Window::_releaseObjectManager()
{
    if( _objectManager.isValid() && _objectManager->getRefCount() == 1 )
        _objectManager->deleteAll();

    _objectManager = 0;
}

void Window::initEventHandler()
{
    EQASSERT( !_eventHandler );
    _eventHandler = EventHandler::registerWindow( this );
}

void Window::exitEventHandler()
{
    if( _eventHandler )
        _eventHandler->deregisterWindow( this );
    _eventHandler = 0;
}

void Window::setGLXContext( GLXContext context )
{
#ifdef GLX
    _glXContext = context;

    if( _xDrawable && _glXContext )
        _initializeGLData();
    else
        _clearGLData();
#endif
}

void Window::setAGLContext( AGLContext context )
{
#ifdef AGL
    _aglContext = context;

    if( _aglContext )
        _initializeGLData();
    else
        _clearGLData();
#endif // AGL
}

void Window::setWGLContext( HGLRC context )
{
#ifdef WGL
    _wglContext = context; 

    if( _wglContext && _wglDC )
        _initializeGLData();
    else
        _clearGLData();
#endif
}

void Window::setXDrawable( XID drawable )
{
#ifdef GLX
    if( _xDrawable == drawable )
        return;

    if( _xDrawable )
        exitEventHandler();
    _xDrawable = drawable;
    if( _xDrawable )
        initEventHandler();

    if( _xDrawable && _glXContext )
        _initializeGLData();
    else
        _clearGLData();

    if( !drawable )
    {
        _pvp.invalidate();
        return;
    }

    // query pixel viewport of window
    Display          *display = _pipe->getXDisplay();
    EQASSERT( display );

    PixelViewport pvp;
    if( getIAttribute( IATTR_HINT_DRAWABLE ) == PBUFFER )
    {
        pvp.x = 0;
        pvp.y = 0;
        
        unsigned value = 0;
        glXQueryDrawable( display, drawable, GLX_WIDTH,  &value );
        pvp.w = static_cast< int32_t >( value );

        value = 0;
        glXQueryDrawable( display, drawable, GLX_HEIGHT, &value );
        pvp.h = static_cast< int32_t >( value );
    }
    else
    {
        XWindowAttributes wa;
        XGetWindowAttributes( display, drawable, &wa );
    
        // Window position is relative to parent: translate to absolute coords
        ::Window root, parent, *children;
        unsigned nChildren;
    
        XQueryTree( display, drawable, &root, &parent, &children, &nChildren );
        if( children != 0 ) XFree( children );

        int x,y;
        ::Window childReturn;
        XTranslateCoordinates( display, parent, root, wa.x, wa.y, &x, &y,
                               &childReturn );

        pvp.x = x;
        pvp.y = y;
        pvp.w = wa.width;
        pvp.h = wa.height;
    }

    setPixelViewport( pvp );
#endif // GLX
}

void Window::setCarbonWindow( WindowRef window )
{
#ifdef AGL
    EQINFO << "set Carbon window " << window << endl;

    if( _carbonWindow == window )
        return;

    if( _carbonWindow )
        exitEventHandler();
    _carbonWindow = window;
    _pvp.invalidate();

    if( window )
    {
        initEventHandler();

        Rect rect;
        Global::enterCarbon();
        if( GetWindowBounds( window, kWindowContentRgn, &rect ) == noErr )
        {
            PixelViewport pvp;
            pvp.x = rect.left;
            pvp.y = rect.top;
            pvp.w = rect.right - rect.left;
            pvp.h = rect.bottom - rect.top;
            setPixelViewport( pvp );
        }
        Global::leaveCarbon();
    }
#endif // AGL
}

void Window::setAGLPBuffer( AGLPbuffer pbuffer )
{
#ifdef AGL
    EQINFO << "set AGL PBuffer " << pbuffer << endl;

    if( _aglPBuffer == pbuffer )
        return;

    _aglPBuffer = pbuffer;
    _pvp.invalidate();

    if( pbuffer )
    {
        GLint         w;
        GLint         h;
        GLenum        target;
        GLenum        format;
        GLint         maxLevel;

        if( aglDescribePBuffer( pbuffer, &w, &h, &target, &format, &maxLevel ))
        {
            EQASSERT( target == GL_TEXTURE_RECTANGLE_EXT );

            const PixelViewport pvp( 0, 0, w, h );
            setPixelViewport( pvp );
        }
    }
#endif // AGL
}

void Window::setWGLWindowHandle( HWND handle )
{
#ifdef WGL
    if( _wglWindow == handle )
        return;

    if( _wglWindow )
    {
        exitEventHandler();
        EQASSERT( _wglDC );
        ReleaseDC( _wglWindow, _wglDC );
        _wglDC = 0;
    }
    EQASSERTINFO( !_wglDC, "Window and PBuffer set simultaneously?" );

    _wglWindow = handle;

    if( _wglWindow )
    {
        initEventHandler();
        _wglDC = GetDC( _wglWindow );
    }

    if( _wglContext && _wglWindow )
        _initializeGLData();
    else
        _clearGLData();

    if( !handle )
    {
        _pvp.invalidate();
        return;
    }

    // query pixel viewport of window
    WINDOWINFO windowInfo;
    windowInfo.cbSize = sizeof( windowInfo );

    GetWindowInfo( handle, &windowInfo );

    PixelViewport pvp;
    pvp.x = windowInfo.rcClient.left;
    pvp.y = windowInfo.rcClient.top;
    pvp.w = windowInfo.rcClient.right  - windowInfo.rcClient.left;
    pvp.h = windowInfo.rcClient.bottom - windowInfo.rcClient.top;
    setPixelViewport( pvp );
#endif // WGL
}

void Window::setWGLPBufferHandle( HPBUFFERARB handle )
{
#ifdef WGL
    if( _wglPBuffer == handle )
        return;

    if( _wglPBuffer )
    {
        EQASSERT( _wglDC );
        wglReleasePbufferDCARB( _wglPBuffer, _wglDC );
        _wglDC = 0;
    }
    EQASSERTINFO( !_wglDC, "Window and PBuffer set simultaneously?" );

    _wglPBuffer = handle;

    if( _wglPBuffer )
        _wglDC = wglGetPbufferDCARB( _wglPBuffer );

    if( _wglContext && _wglPBuffer)
        _initializeGLData();
    else
        _clearGLData();

    if( !handle )
    {
        _pvp.invalidate();
        return;
    }

    // query pixel viewport of PBuffer
    int w,h;
    wglQueryPbufferARB( handle, WGL_PBUFFER_WIDTH_ARB, &w );
    wglQueryPbufferARB( handle, WGL_PBUFFER_HEIGHT_ARB, &h );

    PixelViewport pvp;
    pvp.w = w;
    pvp.h = h;
    setPixelViewport( pvp );
#endif // WGL
}

void Window::makeCurrent() const
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
            wglMakeCurrent( _wglDC, _wglContext );
            break;
#endif

        default: EQUNIMPLEMENTED;
    }
}

void Window::swapBuffers()
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
            SwapBuffers( _wglDC );
            break;
#endif

        default: EQUNIMPLEMENTED;
    }
    EQVERB << "----- SWAP -----" << endl;
}

//======================================================================
// event-handler methods
//======================================================================
bool Window::processEvent( const WindowEvent& event )
{
    ConfigEvent configEvent;
    switch( event.data.type )
    {
        case Event::EXPOSE:
            break;

        case Event::RESIZE:
            setPixelViewport( PixelViewport( event.data.resize.x, 
                                             event.data.resize.y, 
                                             event.data.resize.w,
                                             event.data.resize.h ));
#ifdef AGL
            // 'refresh' agl context viewport
            EQASSERT( _pipe );
            if( _aglContext && _pipe->getWindowSystem() == WINDOW_SYSTEM_AGL )
                aglUpdateContext( _aglContext );
#endif
            return true;

        case Event::KEY_PRESS:
        case Event::KEY_RELEASE:
            if( event.data.keyEvent.key == KC_VOID )
                return true; //ignore
            // else fall through
        case Event::WINDOW_CLOSE:
        case Event::POINTER_MOTION:
        case Event::POINTER_BUTTON_PRESS:
        case Event::POINTER_BUTTON_RELEASE:
        case Event::STATISTIC:
            break;


        case Event::UNKNOWN:
            // Handle other window-system native events here
            return false;

        default:
            EQWARN << "Unhandled window event of type " << event.data.type
                   << endl;
            EQUNIMPLEMENTED;
    }

    configEvent.data = event.data;

    Config* config = getConfig();
    config->sendEvent( configEvent );
    return true;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
eqNet::CommandResult Window::_cmdCreateChannel( eqNet::Command& command )
{
    const WindowCreateChannelPacket* packet = 
        command.getPacket<WindowCreateChannelPacket>();
    EQINFO << "Handle create channel " << packet << endl;

    Channel* channel = Global::getNodeFactory()->createChannel( this );
    getConfig()->attachObject( channel, packet->channelID );

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Window::_cmdDestroyChannel(eqNet::Command& command ) 
{
    const WindowDestroyChannelPacket* packet =
        command.getPacket<WindowDestroyChannelPacket>();
    EQINFO << "Handle destroy channel " << packet << endl;

    Channel* channel = _findChannel( packet->channelID );
    EQASSERT( channel )

    Config*  config  = getConfig();
    config->detachObject( channel );
    delete channel;

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Window::_cmdConfigInit( eqNet::Command& command )
{
    const WindowConfigInitPacket* packet = 
        command.getPacket<WindowConfigInitPacket>();
    EQLOG( LOG_TASKS ) << "TASK window config init " << packet << endl;

    if( packet->pvp.isValid( ))
        _setPixelViewport( packet->pvp );
    else
        _setViewport( packet->vp );
    _name = packet->name;

    for( uint32_t i=0; i<IATTR_ALL; ++i )
        _iAttributes[i] = packet->iattr[i];

    _error.clear();
    if( _pipe->getWindowSystem() == WINDOW_SYSTEM_AGL )
        _renderContextAGLLock = new SpinLock;

    WindowConfigInitReplyPacket reply;
    reply.result = configInit( packet->initID );
    EQLOG( LOG_TASKS ) << "TASK window config init reply " << &reply << endl;

    eqNet::NodePtr node = command.getNode();
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
            if( !_wglDC || !_wglContext )
            {
                EQERROR << "configInit() did not provide a drawable handle and"
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

eqNet::CommandResult Window::_cmdConfigExit( eqNet::Command& command )
{
    const WindowConfigExitPacket* packet =
        command.getPacket<WindowConfigExitPacket>();
    EQLOG( LOG_TASKS ) << "TASK configExit " << getName() <<  " " << packet 
                       << endl;

    if( _pipe->isInitialized( ))
        EQ_GL_CALL( makeCurrent( ));
    // else emergency exit, no context available.

    WindowConfigExitReplyPacket reply;
    reply.result = configExit();

    send( command.getNode(), reply );

    delete _renderContextAGLLock;
    _renderContextAGLLock = 0;

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Window::_cmdFrameStart( eqNet::Command& command )
{
    const WindowFrameStartPacket* packet = 
        command.getPacket<WindowFrameStartPacket>();
    EQVERB << "handle window frame start " << packet << endl;

    //_grabFrame( packet->frameNumber ); single-threaded

    {
        ScopedMutex< SpinLock > mutex( _renderContextAGLLock );
        if( _drawableConfig.doublebuffered )
            _renderContexts[FRONT].swap( _renderContexts[BACK] );
        _renderContexts[BACK].clear();
    }
    EQ_GL_CALL( makeCurrent( ));

    frameStart( packet->frameID, packet->frameNumber );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Window::_cmdFrameFinish( eqNet::Command& command )
{
    const WindowFrameFinishPacket* packet =
        command.getPacket<WindowFrameFinishPacket>();
    EQVERB << "handle window frame sync " << packet << endl;

    EQ_GL_CALL( makeCurrent( ));
    frameFinish( packet->frameID, packet->frameNumber );

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Window::_cmdFinish(eqNet::Command& command ) 
{
    EQ_GL_CALL( makeCurrent( ));
    finish();
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Window::_cmdBarrier( eqNet::Command& command )
{
    const WindowBarrierPacket* packet = 
        command.getPacket<WindowBarrierPacket>();
    EQVERB << "handle barrier " << packet << endl;
    EQLOG( eqNet::LOG_BARRIER ) << "swap barrier " << packet->barrierID
                                << " v" << packet->barrierVersion <<endl;
    
    Node*           node    = getNode();
    eqNet::Barrier* barrier = node->getBarrier( packet->barrierID, 
                                                packet->barrierVersion );

    WindowStatistics stat( Statistic::WINDOW_SWAP_BARRIER, this );
    barrier->enter();
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Window::_cmdSwap(eqNet::Command& command ) 
{
    EQLOG( LOG_TASKS ) << "TASK swap  " << getName() << endl;
    EQ_GL_CALL( makeCurrent( ));

    WindowStatistics stat( Statistic::WINDOW_SWAP, this );
    swapBuffers();

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Window::_cmdFrameDrawFinish( eqNet::Command& command )
{
    WindowFrameDrawFinishPacket* packet = 
        command.getPacket< WindowFrameDrawFinishPacket >();
    EQLOG( LOG_TASKS ) << "TASK draw finish " << getName() <<  " " << packet
                       << endl;

    frameDrawFinish( packet->frameID, packet->frameNumber );
    return eqNet::COMMAND_HANDLED;
}

std::ostream& operator << ( std::ostream& os,
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
}
