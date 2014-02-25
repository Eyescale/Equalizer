
/* Copyright (c) 2009-2014, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2014, Daniel Nachbaur <danielnachbaur@gmail.com>
 *                    2009, Maxim Makhinya
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "window.h"

#include "eventHandler.h"
#include "pipe.h"
#include "windowEvent.h"

#include "../global.h"
#include "../pipe.h"


namespace eq
{
namespace glx
{
namespace detail
{
class Window
{
public:
    Window( Display* xDisplay_, const GLXEWContext* glxewContext_,
            MessagePump* messagePump_, const SystemWindow* shareWindow_ )
        : xDisplay( xDisplay_ )
        , xDrawable ( 0 )
        , glXContext( 0 )
        , glXNVSwapGroup( 0 )
        , glXEventHandler( 0 )
        , glxewContext( glxewContext_ )
        , messagePump( messagePump_ )
        , shareWindow( shareWindow_ )
    {}

    /** The display connection (maintained by GLXPipe) */
    Display*   xDisplay;
    /** The X11 drawable ID of the window. */
    XID        xDrawable;
    /** The glX rendering context. */
    GLXContext glXContext;
    /** The currently joined swap group. */
    uint32_t glXNVSwapGroup;

    /** The event handler. */
    EventHandler* glXEventHandler;

    /** The glX extension pointer table. */
    const GLXEWContext* glxewContext;

    MessagePump* const messagePump;
    const SystemWindow* shareWindow;
};
}

Window::Window( NotifierInterface* parent, const WindowSettings& settings,
                Display* xDisplay, const GLXEWContext* glxewContext,
                MessagePump* messagePump, const SystemWindow* shareWindow )
    : WindowIF( parent, settings )
    , _impl( new detail::Window( xDisplay, glxewContext, messagePump,
                                 shareWindow ))
{
}

Window::~Window()
{
    delete _impl;
}

//---------------------------------------------------------------------------
// GLX init
//---------------------------------------------------------------------------
namespace
{
static Bool WaitForNotify( Display*, XEvent *e, char *arg )
{ return (e->type == MapNotify) && (e->xmap.window == (::Window)arg); }
}

bool Window::configInit()
{
    GLXFBConfig* fbConfig = chooseGLXFBConfig();
    if( !fbConfig )
    {
        sendError( ERROR_SYSTEMWINDOW_PIXELFORMAT_NOTFOUND );
        return false;
    }

    GLXContext context = createGLXContext( fbConfig );
    setGLXContext( context );
    if( !context )
    {
        XFree( fbConfig );
        return false;
    }

    const bool success = configInitGLXDrawable( fbConfig );
    XFree( fbConfig );

    if( !success || !_impl->xDrawable )
    {
        sendError( ERROR_GLXWINDOW_NO_DRAWABLE );
        return false;
    }

    makeCurrent();
    initGLEW();
    _initSwapSync();
    if( getIAttribute( WindowSettings::IATTR_HINT_DRAWABLE ) == FBO )
        configInitFBO();

    return success;
}

GLXFBConfig* Window::chooseGLXFBConfig()
{
    if( !_impl->xDisplay )
    {
        sendError( ERROR_GLXWINDOW_NO_DISPLAY );
        return 0;
    }
    if( !GLXEW_VERSION_1_3 && !GLXEW_SGIX_fbconfig )
    {
        sendError( ERROR_GLXWINDOW_FBCONFIG_REQUIRED );
        return 0;
    }

    // build attribute list
    std::vector< int > attributes;
    const int32_t drawableHint = getIAttribute( WindowSettings::IATTR_HINT_DRAWABLE );
    switch( drawableHint )
    {
      case PBUFFER:
        attributes.push_back ( GLX_DRAWABLE_TYPE );
        attributes.push_back ( GLX_PBUFFER_BIT );
        break;

      default:
        LBWARN << "Unknown drawable type " << drawableHint << ", using window"
               << std::endl;
        // no break;
      case UNDEFINED:
      case OFF: // needs fbConfig with visual for dummy window
      case FBO: // No typo - FBO needs fbConfig with visual for dummy window
      case WINDOW:
        attributes.push_back( GLX_X_RENDERABLE );
        attributes.push_back( True );
    }

    int colorSize = getIAttribute( WindowSettings::IATTR_PLANES_COLOR );
    if( colorSize != OFF )
    {
        if( drawableHint == FBO || drawableHint == OFF )
            colorSize = 8; // Create FBO dummy window with 8bpp
        else switch( colorSize )
        {
          case RGBA16F:
          case RGBA32F:
            if( !GLXEW_ARB_fbconfig_float )
            {
                sendError( ERROR_SYSTEMWINDOW_ARB_FLOAT_FB_REQUIRED );
                return 0;
            }
            attributes.push_back( GLX_RENDER_TYPE );
            attributes.push_back( GLX_RGBA_FLOAT_BIT );
            colorSize = colorSize == RGBA16F ? 16 : 32;
            break;

          case AUTO:
          case ON:
            colorSize = 8;
            break;
          default:
            break;
        }

        LBASSERT( colorSize > 0 );
        attributes.push_back( GLX_RED_SIZE );
        attributes.push_back( colorSize );
        attributes.push_back( GLX_GREEN_SIZE );
        attributes.push_back( colorSize );
        attributes.push_back( GLX_BLUE_SIZE );
        attributes.push_back( colorSize );

        const int alphaSize = getIAttribute( WindowSettings::IATTR_PLANES_ALPHA );
        switch( alphaSize )
        {
          case AUTO:
          case UNDEFINED:
          case ON:
            attributes.push_back( GLX_ALPHA_SIZE );
            attributes.push_back( colorSize );
            break;

          case OFF:
              break;

          default:
            LBASSERT( alphaSize > 0 );
            attributes.push_back( GLX_ALPHA_SIZE );
            attributes.push_back( alphaSize > 0 ? alphaSize : colorSize );
        }
    }
    const int depthSize = getIAttribute( WindowSettings::IATTR_PLANES_DEPTH );
    if( depthSize > 0  || depthSize == AUTO )
    {
        attributes.push_back( GLX_DEPTH_SIZE );
        attributes.push_back( depthSize > 0 ? depthSize : 1 );
    }
    const int stencilSize = getIAttribute( WindowSettings::IATTR_PLANES_STENCIL );
    if( stencilSize > 0 || stencilSize == AUTO )
    {
        attributes.push_back( GLX_STENCIL_SIZE );
        attributes.push_back( stencilSize>0 ? stencilSize : 1 );
    }
    const int accumSize = getIAttribute( WindowSettings::IATTR_PLANES_ACCUM );
    const int accumAlpha = getIAttribute( WindowSettings::IATTR_PLANES_ACCUM_ALPHA );
    if( accumSize >= 0 )
    {
        attributes.push_back( GLX_ACCUM_RED_SIZE );
        attributes.push_back( accumSize );
        attributes.push_back( GLX_ACCUM_GREEN_SIZE );
        attributes.push_back( accumSize );
        attributes.push_back( GLX_ACCUM_BLUE_SIZE );
        attributes.push_back( accumSize );
        attributes.push_back( GLX_ACCUM_ALPHA_SIZE );
        attributes.push_back( accumAlpha >= 0 ? accumAlpha : accumSize );
    }
    else if( accumAlpha >= 0 )
    {
        attributes.push_back( GLX_ACCUM_ALPHA_SIZE );
        attributes.push_back( accumAlpha );
    }

    const int samplesSize  = getIAttribute( WindowSettings::IATTR_PLANES_SAMPLES );
    if( samplesSize >= 0 )
    {
        attributes.push_back( GLX_SAMPLE_BUFFERS );
        attributes.push_back( 1 );
        attributes.push_back( GLX_SAMPLES );
        attributes.push_back( samplesSize );
    }

#ifdef Darwin
    // WAR: glDrawBuffer( GL_BACK ) renders only to the left back buffer on a
    // stereo visual on Darwin which creates ugly flickering on mono configs
    if( getIAttribute( WindowSettings::IATTR_HINT_STEREO ) == ON )
    {
        attributes.push_back( GLX_STEREO );
        attributes.push_back( true );
    }
#else
    if( getIAttribute( WindowSettings::IATTR_HINT_STEREO ) == ON ||
        ( getIAttribute( WindowSettings::IATTR_HINT_STEREO )   == AUTO &&
          getIAttribute( WindowSettings::IATTR_HINT_DRAWABLE ) == WINDOW ))
    {
        attributes.push_back( GLX_STEREO );
        attributes.push_back( true );
    }
#endif
    if( getIAttribute( WindowSettings::IATTR_HINT_DOUBLEBUFFER ) == ON ||
        ( getIAttribute( WindowSettings::IATTR_HINT_DOUBLEBUFFER ) == AUTO &&
          getIAttribute( WindowSettings::IATTR_HINT_DRAWABLE )     == WINDOW ))
    {
        attributes.push_back( GLX_DOUBLEBUFFER );
        attributes.push_back( true );
    }
    attributes.push_back( None );

    // build backoff list, least important attribute last
    std::vector<int> backoffAttributes;
    if( getIAttribute( WindowSettings::IATTR_HINT_DRAWABLE ) == WINDOW )
    {
        if( getIAttribute( WindowSettings::IATTR_HINT_DOUBLEBUFFER ) == AUTO )
            backoffAttributes.push_back( GLX_DOUBLEBUFFER );

#ifndef Darwin
        if( getIAttribute( WindowSettings::IATTR_HINT_STEREO ) == AUTO )
            backoffAttributes.push_back( GLX_STEREO );
#endif
    }

    if( stencilSize == AUTO )
        backoffAttributes.push_back( GLX_STENCIL_SIZE );

    PFNGLXCHOOSEFBCONFIGSGIXPROC chooseFBConfig = GLXEW_VERSION_1_3 ?
        glXChooseFBConfig : glXChooseFBConfigSGIX;

    const int screen = DefaultScreen( _impl->xDisplay );
    int nConfigs = 0;
    GLXFBConfig* configs = chooseFBConfig( _impl->xDisplay, screen,
                                           &attributes[0], &nConfigs );

    while(( nConfigs == 0 || !configs ) && !backoffAttributes.empty( ))
    {
        // Gradually remove backoff attributes
        const int attribute = backoffAttributes.back();
        backoffAttributes.pop_back();

        std::vector<int>::iterator iter = find( attributes.begin(),
                                                attributes.end(), attribute );
        LBASSERT( iter != attributes.end( ));
        attributes.erase( iter, iter+2 );
        configs = chooseFBConfig( _impl->xDisplay, screen, &attributes[0],
                                  &nConfigs );
    }

    return configs;
}

GLXContext Window::createGLXContext( GLXFBConfig* fbConfig )
{
    if( !_impl->xDisplay )
    {
        sendError( ERROR_GLXWINDOW_NO_DISPLAY );
        return 0;
    }
    if( !fbConfig )
    {
        sendError( ERROR_SYSTEMWINDOW_NO_PIXELFORMAT );
        return 0;
    }

    GLXContext shCtx = 0;
    if( _impl->shareWindow )
    {
        const WindowIF* shareGLXWindow = dynamic_cast< const WindowIF* >(
                                                        _impl->shareWindow );
        if( shareGLXWindow )
            shCtx = shareGLXWindow->getGLXContext();
    }
    int type = GLX_RGBA_TYPE;
    if( getIAttribute( WindowSettings::IATTR_HINT_DRAWABLE ) == PBUFFER &&
        ( getIAttribute( WindowSettings::IATTR_PLANES_COLOR ) == RGBA16F ||
          getIAttribute( WindowSettings::IATTR_PLANES_COLOR ) == RGBA32F ))
    {
        type = GLX_RGBA_FLOAT_TYPE;
    }

    GLXContext context = GLXEW_VERSION_1_3 ?
        glXCreateNewContext( _impl->xDisplay, fbConfig[0], type, shCtx, True ):
        glXCreateContextWithConfigSGIX( _impl->xDisplay, fbConfig[0], type,
                                        shCtx, True );

#ifdef Darwin
    // WAR http://xquartz.macosforge.org/trac/ticket/466
    if( !context )
    {
        XVisualInfo* visInfo = GLXEW_VERSION_1_3 ?
            glXGetVisualFromFBConfig( _impl->xDisplay, fbConfig[0] ) :
            glXGetVisualFromFBConfigSGIX( _impl->xDisplay, fbConfig[0] );
        if( !visInfo )
        {
            std::vector<int> attributes;
            attributes.push_back( GLX_RGBA );
            attributes.push_back( GLX_RED_SIZE );
            attributes.push_back( 1 );
            attributes.push_back( GLX_ALPHA_SIZE );
            attributes.push_back( 1 );
            attributes.push_back( GLX_DEPTH_SIZE );
            attributes.push_back( 1 );
            attributes.push_back( GLX_DOUBLEBUFFER );
            attributes.push_back( None );

            const int screen = DefaultScreen( _impl->xDisplay );
            visInfo = glXChooseVisual( _impl->xDisplay, screen,
                                       &attributes.front( ));
            if( !visInfo )
            {
                sendError( ERROR_GLXWINDOW_NO_VISUAL );
                return 0;
            }
        }

        context = glXCreateContext( _impl->xDisplay, visInfo, shCtx, True );
        XFree( visInfo );
    }
#endif

    if( !context )
    {
        sendError( ERROR_GLXWINDOW_CREATECONTEXT_FAILED );
        return 0;
    }
    return context;
}

bool Window::configInitGLXDrawable( GLXFBConfig* fbConfig )
{
    switch( getIAttribute( WindowSettings::IATTR_HINT_DRAWABLE ))
    {
        case PBUFFER:
            return configInitGLXPBuffer( fbConfig );

        case FBO:
        case OFF:
        {
            const PixelViewport pvp( 0, 0, 1, 1 );
            setXDrawable( _createGLXWindow( fbConfig, pvp ));
            return _impl->xDrawable != 0;
        }

        default:
            LBWARN << "Unknown drawable type "
                   << getIAttribute( WindowSettings::IATTR_HINT_DRAWABLE )
                   << ", using window" << std::endl;
            // no break;
        case UNDEFINED:
        case WINDOW:
            return configInitGLXWindow( fbConfig );
    }
}

bool Window::configInitGLXWindow( GLXFBConfig* fbConfig )
{
    if( !_impl->xDisplay )
    {
        sendError( ERROR_GLXWINDOW_NO_DISPLAY );
        return false;
    }

    PixelViewport pvp = getPixelViewport();
    if( getIAttribute( WindowSettings::IATTR_HINT_FULLSCREEN ) == ON )
    {
        const int screen = DefaultScreen( _impl->xDisplay );
        pvp.h = DisplayHeight( _impl->xDisplay, screen );
        pvp.w = DisplayWidth( _impl->xDisplay, screen );
        pvp.x = 0;
        pvp.y = 0;

        setPixelViewport( pvp );
    }

    XID drawable = _createGLXWindow( fbConfig, pvp );
    if( !drawable )
        return false;

    // map and wait for MapNotify event
    XMapWindow( _impl->xDisplay, drawable );

    XEvent event;
    XIfEvent( _impl->xDisplay, &event, WaitForNotify, (XPointer)(drawable) );

    XMoveResizeWindow( _impl->xDisplay, drawable, pvp.x, pvp.y, pvp.w, pvp.h );
    XFlush( _impl->xDisplay );

    // Grab keyboard focus in fullscreen mode
    if( getIAttribute( WindowSettings::IATTR_HINT_FULLSCREEN ) == ON ||
        getIAttribute( WindowSettings::IATTR_HINT_DECORATION ) == OFF )
    {
        XGrabKeyboard( _impl->xDisplay, drawable, True, GrabModeAsync,
                       GrabModeAsync, CurrentTime );
    }
    setXDrawable( drawable );

    LBVERB << "Created X11 drawable " << drawable << std::endl;
    return true;
}

XID Window::_createGLXWindow( GLXFBConfig* fbConfig, const PixelViewport& pvp )
{
    LBASSERT( getIAttribute( WindowSettings::IATTR_HINT_DRAWABLE ) != PBUFFER );

    if( !_impl->xDisplay )
    {
        sendError( ERROR_GLXWINDOW_NO_DISPLAY );
        return 0;
    }
    if( !fbConfig )
    {
        sendError( ERROR_SYSTEMWINDOW_NO_PIXELFORMAT );
        return 0;
    }

    XVisualInfo* visInfo = GLXEW_VERSION_1_3 ?
        glXGetVisualFromFBConfig( _impl->xDisplay, fbConfig[0] ) :
        glXGetVisualFromFBConfigSGIX( _impl->xDisplay, fbConfig[0]);
    if( !visInfo )
    {
        sendError( ERROR_GLXWINDOW_NO_VISUAL );
        return 0;
    }

    const int screen = DefaultScreen( _impl->xDisplay );
    XID parent = RootWindow( _impl->xDisplay, screen );
    XSetWindowAttributes wa;
    wa.colormap = XCreateColormap( _impl->xDisplay, parent, visInfo->visual,
                                   AllocNone );
    wa.background_pixmap = None;
    wa.border_pixel = 0;
    wa.event_mask = StructureNotifyMask | VisibilityChangeMask | ExposureMask |
                    KeyPressMask | KeyReleaseMask | PointerMotionMask |
                    ButtonPressMask | ButtonReleaseMask;

    switch( getIAttribute( WindowSettings::IATTR_HINT_DECORATION ))
    {
      case ON:
          wa.override_redirect = False;
          break;

      case OFF:
          wa.override_redirect = True;
          break;

      case AUTO:
      default:
          wa.override_redirect =
              getIAttribute( WindowSettings::IATTR_HINT_FULLSCREEN ) == ON ?
              True : False;
          break;
    }

    XID drawable = XCreateWindow( _impl->xDisplay, parent,
                                  pvp.x, pvp.y, pvp.w, pvp.h,
                                  0, visInfo->depth, InputOutput,
                                  visInfo->visual,
                                  CWBackPixmap | CWBorderPixel | CWEventMask |
                                  CWColormap | CWOverrideRedirect, &wa );
    XFree( visInfo );
    if( !drawable )
    {
        sendError( ERROR_GLXWINDOW_CREATEWINDOW_FAILED );
        return 0;
    }

    std::stringstream windowTitle;
    const std::string& name = getName();

    if( name.empty( ))
    {
        windowTitle << "Equalizer";
#ifndef NDEBUG
        windowTitle << " (" << getpid() << ")";
#endif
    }
    else
        windowTitle << name;

    XStoreName( _impl->xDisplay, drawable, windowTitle.str().c_str( ));

    // Register for close window request from the window manager
    Atom deleteAtom = XInternAtom( _impl->xDisplay, "WM_DELETE_WINDOW", False );
    XSetWMProtocols( _impl->xDisplay, drawable, &deleteAtom, 1 );

    return drawable;
}

bool Window::configInitGLXPBuffer( GLXFBConfig* fbConfig )
{
    if( !_impl->xDisplay )
    {
        sendError( ERROR_GLXWINDOW_NO_DISPLAY );
        return false;
    }
    if( !fbConfig )
    {
        sendError( ERROR_SYSTEMWINDOW_NO_PIXELFORMAT );
        return false;
    }
    if( !GLXEW_VERSION_1_3 )
    {
        sendError( ERROR_GLXWINDOW_GLX_1_3_REQUIRED );
        return false;
    }

    // Create PBuffer
    const PixelViewport& pvp = getPixelViewport();
    const int attributes[] = { GLX_PBUFFER_WIDTH, pvp.w,
                               GLX_PBUFFER_HEIGHT, pvp.h,
                               GLX_LARGEST_PBUFFER, True,
                               GLX_PRESERVED_CONTENTS, True,
                               0 };

    // TODO: Could check for GLX_SGIX_pbuffer, but the only GLX 1.2 platform at
    // hand is OS X, which does not support this extension.

    XID pbuffer = glXCreatePbuffer( _impl->xDisplay, fbConfig[ 0 ], attributes );
    if ( !pbuffer )
    {
        sendError( ERROR_GLXWINDOW_CREATEPBUFFER_FAILED );
        return false;
    }

    XFlush( _impl->xDisplay );
    setXDrawable( pbuffer );

    LBVERB << "Created X11 PBuffer " << pbuffer << std::endl;
    return true;
}

void Window::setXDrawable( XID drawable )
{
    LBASSERT( _impl->xDisplay );

    if( _impl->xDrawable == drawable )
        return;

    if( _impl->xDrawable )
        exitEventHandler();

    _impl->xDrawable = drawable;

    if( !drawable )
        return;

    const int32_t drawableType = getIAttribute(WindowSettings::IATTR_HINT_DRAWABLE);
    if( drawableType != OFF )
        initEventHandler( _impl->messagePump );

    // query pixel viewport of window
    switch( drawableType )
    {
        case PBUFFER:
        {
            unsigned width = 0;
            unsigned height = 0;
            glXQueryDrawable( _impl->xDisplay, drawable, GLX_WIDTH,  &width );
            glXQueryDrawable( _impl->xDisplay, drawable, GLX_HEIGHT, &height );

            setPixelViewport(
                PixelViewport( 0, 0, int32_t( width ), int32_t( height )));
            break;
        }
        case WINDOW:
        {
            XWindowAttributes wa;
            XGetWindowAttributes( _impl->xDisplay, drawable, &wa );

            // position is relative to parent: translate to absolute coords
            ::Window root, parent, *children;
            unsigned nChildren;

            XQueryTree( _impl->xDisplay, drawable, &root, &parent, &children,
                        &nChildren );
            if( children != 0 )
                XFree( children );

            int x,y;
            ::Window childReturn;
            XTranslateCoordinates( _impl->xDisplay, parent, root, wa.x, wa.y,
                                   &x, &y, &childReturn );

            setPixelViewport( PixelViewport( x, y, wa.width, wa.height ));
            break;
        }
        default:
            LBUNIMPLEMENTED;
        case OFF:
        case FBO:
            LBASSERT( getPixelViewport().hasArea( ));
    }
}

void Window::setGLXContext( GLXContext context )
{
    _impl->glXContext = context;
}

GLXContext Window::getGLXContext() const
{
    return _impl->glXContext;
}

XID Window::getXDrawable() const
{
    return _impl->xDrawable;
}

Display* Window::getXDisplay()
{
    return _impl->xDisplay;
}

const GLXEWContext* Window::glxewGetContext() const
{
    return _impl->glxewContext;
}

void Window::_initSwapSync()
{
    if( getIAttribute( WindowSettings::IATTR_HINT_DRAWABLE ) == OFF )
        return;

    const int32_t swapSync = getIAttribute( WindowSettings::IATTR_HINT_SWAPSYNC );
    if( swapSync == AUTO ) // leave it alone
        return;

    if( GLXEW_SGI_swap_control )
    {
        glXSwapIntervalSGI( (swapSync < 0) ? 1 : swapSync );
    }
    else
        LBWARN << "GLX_SGI_swap_control not supported, ignoring window "
               << "swapsync hint " << IAttribute( swapSync ) << std::endl;
}


void Window::configExit()
{
    if( !_impl->xDisplay )
        return;

    leaveNVSwapBarrier();
    configExitFBO();
    exitGLEW();

    glXMakeCurrent( _impl->xDisplay, None, 0 );

    GLXContext context = getGLXContext();
    XID drawable = getXDrawable();

    setGLXContext( 0 );
    setXDrawable( 0 );

    // WAR assert in glXDestroyContext/xcb_io.c:183
    XSync( _impl->xDisplay, False );

    if( context )
        glXDestroyContext( _impl->xDisplay, context );

    if( drawable )
    {
        if( getIAttribute( WindowSettings::IATTR_HINT_DRAWABLE ) == PBUFFER )
            glXDestroyPbuffer( _impl->xDisplay, drawable );
        else
            XDestroyWindow( _impl->xDisplay, drawable );
    }

    LBVERB << "Destroyed GLX context and X drawable " << std::endl;
}

void Window::makeCurrent( const bool cache ) const
{
    LBASSERT( _impl->xDisplay );
    if( cache && isCurrent( ))
        return;

    glXMakeCurrent( _impl->xDisplay, _impl->xDrawable, _impl->glXContext );
    WindowIF::makeCurrent();
    if( _impl->glXContext )
    {
        EQ_GL_ERROR( "After glXMakeCurrent" );
    }
}

void Window::swapBuffers()
{
    LBASSERT( _impl->xDisplay );
    glXSwapBuffers( _impl->xDisplay, _impl->xDrawable );
}

void Window::joinNVSwapBarrier( const uint32_t group, const uint32_t barrier)
{
    if( group == 0 )
        return;

#if 1
    LBWARN << "Entering untested function GLXWindow::joinNVSwapBarrier"
           << std::endl;

    if ( !GLXEW_NV_swap_group )
    {
        LBWARN << "NV Swap group extension not supported" << std::endl;
        return;
    }

    const int screen = DefaultScreen( _impl->xDisplay );
    uint32_t maxBarrier = 0;
    uint32_t maxGroup = 0;

    glXQueryMaxSwapGroupsNV( _impl->xDisplay, screen, &maxGroup, &maxBarrier );

    if( group > maxGroup )
    {
        LBWARN << "Failed to initialize GLX_NV_swap_group: requested group "
               << group << " greater than maxGroups (" << maxGroup << ")"
               << std::endl;
        return;
    }

    if( barrier > maxBarrier )
    {
        LBWARN << "Failed to initialize GLX_NV_swap_group: requested barrier "
               << barrier << "greater than maxBarriers (" << maxBarrier << ")"
               << std::endl;
        return;
    }

    if( !glXJoinSwapGroupNV( _impl->xDisplay, _impl->xDrawable, group ))
    {
        LBWARN << "Failed to join swap group " << group << std::endl;
        return;
    }

    _impl->glXNVSwapGroup = group;

    if( !glXBindSwapBarrierNV( _impl->xDisplay, group, barrier ))
    {
        LBWARN << "Failed to bind swap barrier " << barrier << std::endl;
        return;
    }

    LBVERB << "Joined swap group " << group << " and barrier " << barrier
           << std::endl;
#else
    LBUNIMPLEMENTED;
#endif
}

void Window::leaveNVSwapBarrier()
{
    if( _impl->glXNVSwapGroup == 0 )
        return;

    glXBindSwapBarrierNV( _impl->xDisplay, _impl->glXNVSwapGroup, 0 );
    glXJoinSwapGroupNV( _impl->xDisplay, _impl->xDrawable, 0 );

    _impl->glXNVSwapGroup = 0;
}

bool Window::processEvent( const WindowEvent& event )
{
    switch( event.type )
    {
      case Event::WINDOW_POINTER_BUTTON_PRESS:
        if( getIAttribute( WindowSettings::IATTR_HINT_GRAB_POINTER ) == ON &&
            getIAttribute( WindowSettings::IATTR_HINT_DRAWABLE ) == WINDOW &&
            // If no other button was pressed already, capture the mouse
            event.pointerButtonPress.buttons == event.pointerButtonPress.button)
        {
            const unsigned int eventMask = ButtonPressMask | ButtonReleaseMask |
                                           ButtonMotionMask;
            const int result = XGrabPointer( getXDisplay(), getXDrawable(),
                                             False, eventMask, GrabModeAsync,
                                             GrabModeAsync, None, None,
                                             CurrentTime );
            if( result == GrabSuccess )
            {
                WindowEvent grabEvent = event;
                grabEvent.type = Event::WINDOW_POINTER_GRAB;
                processEvent( grabEvent );
            }
            else
            {
                LBWARN << "Failed to grab mouse: XGrabPointer returned "
                       << result << std::endl;
            }
        }
        break;

      case Event::WINDOW_POINTER_BUTTON_RELEASE:
        if( getIAttribute( WindowSettings::IATTR_HINT_GRAB_POINTER ) == ON &&
            getIAttribute( WindowSettings::IATTR_HINT_DRAWABLE ) == WINDOW &&
            // If no button is pressed anymore, release the mouse
            event.pointerButtonRelease.buttons == PTR_BUTTON_NONE )
        {
            // Call early for consistent ordering
            const bool result = SystemWindow::processEvent( event );

            WindowEvent ungrabEvent = event;
            ungrabEvent.type = Event::WINDOW_POINTER_UNGRAB;
            processEvent( ungrabEvent );
            XUngrabPointer( getXDisplay(), CurrentTime );
            return result;
        }
        break;
    }
    return SystemWindow::processEvent( event );
}

void Window::initEventHandler( MessagePump* messagePump )
{
    LBASSERT( !_impl->glXEventHandler );
    _impl->glXEventHandler = new EventHandler( this, messagePump );
}

void Window::exitEventHandler()
{
    delete _impl->glXEventHandler;
    _impl->glXEventHandler = 0;
}

}
}
