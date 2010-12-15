
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "glXWindow.h"

#include "global.h"
#include "glXEventHandler.h"
#include "glXPipe.h"
#include "pipe.h"

namespace eq
{
GLXWindow::GLXWindow( Window* parent, Display* xDisplay,
                      GLXEWContext* glxewContext )
    : GLXWindowIF( parent )
    , _xDisplay( xDisplay )
    , _xDrawable ( 0 )
    , _glXContext( 0 )
    , _glXNVSwapGroup( 0 )
    , _glXEventHandler( 0 )
    , _glxewContext( glxewContext )
{
    if( !_xDisplay )
    {
        Pipe* pipe = getPipe();
        GLXPipe* glxPipe = dynamic_cast< GLXPipe* >( pipe->getSystemPipe( ));
        if( glxPipe )
            _xDisplay = glxPipe->getXDisplay();
    }
    if( !_glxewContext )
    {
        Pipe* pipe = getPipe();
        GLXPipe* glxPipe = dynamic_cast< GLXPipe* >( pipe->getSystemPipe( ));
        if( glxPipe )
            _glxewContext = glxPipe->glxewGetContext();
    }
}

GLXWindow::~GLXWindow( )
{
}

//---------------------------------------------------------------------------
// GLX init
//---------------------------------------------------------------------------
namespace
{
static Bool WaitForNotify( Display*, XEvent *e, char *arg )
{ return (e->type == MapNotify) && (e->xmap.window == (::Window)arg); }
}

bool GLXWindow::configInit( )
{
    XVisualInfo* visualInfo = chooseXVisualInfo();
    if( !visualInfo )
        return false;

    GLXContext context = createGLXContext( visualInfo );
    setGLXContext( context );
    if( !context )
        return false;

    const bool success = configInitGLXDrawable( visualInfo );
    XFree( visualInfo );

    if( !success || !_xDrawable )
    {
        if( getError() == ERROR_NONE )
            setError( ERROR_GLXWINDOW_NO_DRAWABLE );
        return false;
    }

    makeCurrent();
    initGLEW();

    if( getIAttribute( Window::IATTR_HINT_DRAWABLE ) == FBO )
        configInitFBO();

    return success;
}

XVisualInfo* GLXWindow::chooseXVisualInfo()
{
    if( !_xDisplay )
    {
        setError( ERROR_GLXWINDOW_NO_DISPLAY );
        return 0;
    }

    // build attribute list
    std::vector<int> attributes;
    attributes.push_back( GLX_RGBA );

    const int colorSize = getIAttribute( Window::IATTR_PLANES_COLOR );
    if( colorSize > 0 || colorSize == eq::AUTO )
    {
        attributes.push_back( GLX_RED_SIZE );
        attributes.push_back( colorSize>0 ? colorSize : 1 );
        attributes.push_back( GLX_GREEN_SIZE );
        attributes.push_back( colorSize>0 ? colorSize : 1 );
        attributes.push_back( GLX_BLUE_SIZE );
        attributes.push_back( colorSize>0 ? colorSize : 1 );
    }
    const int alphaSize = getIAttribute( Window::IATTR_PLANES_ALPHA );
    if( alphaSize > 0 || alphaSize == AUTO )
    {
        attributes.push_back( GLX_ALPHA_SIZE );
        attributes.push_back( alphaSize>0 ? alphaSize : 1 );
    }
    const int depthSize = getIAttribute( Window::IATTR_PLANES_DEPTH );
    if( depthSize > 0  || depthSize == AUTO )
    {
        attributes.push_back( GLX_DEPTH_SIZE );
        attributes.push_back( depthSize>0 ? depthSize : 1 );
    }
    const int stencilSize = getIAttribute( Window::IATTR_PLANES_STENCIL );
    if( stencilSize > 0 || stencilSize == AUTO )
    {
        attributes.push_back( GLX_STENCIL_SIZE );
        attributes.push_back( stencilSize>0 ? stencilSize : 1 );
    }
    const int accumSize = getIAttribute( Window::IATTR_PLANES_ACCUM );
    const int accumAlpha = getIAttribute( Window::IATTR_PLANES_ACCUM_ALPHA );
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

    const int samplesSize  = getIAttribute( Window::IATTR_PLANES_SAMPLES );
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
    if( getIAttribute( Window::IATTR_HINT_STEREO ) == ON )
        attributes.push_back( GLX_STEREO );
#else
    if( getIAttribute( Window::IATTR_HINT_STEREO ) == ON ||
        ( getIAttribute( Window::IATTR_HINT_STEREO )   == AUTO && 
          getIAttribute( Window::IATTR_HINT_DRAWABLE ) == WINDOW ))

        attributes.push_back( GLX_STEREO );
#endif
    if( getIAttribute( Window::IATTR_HINT_DOUBLEBUFFER ) == ON ||
        ( getIAttribute( Window::IATTR_HINT_DOUBLEBUFFER ) == AUTO && 
          getIAttribute( Window::IATTR_HINT_DRAWABLE )     == WINDOW ))

        attributes.push_back( GLX_DOUBLEBUFFER );

    attributes.push_back( None );

    // build backoff list, least important attribute last
    std::vector<int> backoffAttributes;
    if( getIAttribute( Window::IATTR_HINT_DRAWABLE ) == WINDOW )
    {
        if( getIAttribute( Window::IATTR_HINT_DOUBLEBUFFER ) == AUTO )
            backoffAttributes.push_back( GLX_DOUBLEBUFFER );

#ifndef Darwin
        if( getIAttribute( Window::IATTR_HINT_STEREO ) == AUTO )
            backoffAttributes.push_back( GLX_STEREO );
#endif
    }

    if( stencilSize == AUTO )
        backoffAttributes.push_back( GLX_STENCIL_SIZE );

    // Choose visual
    const int    screen  = DefaultScreen( _xDisplay );
    XVisualInfo *visInfo = glXChooseVisual( _xDisplay, screen, 
                                            &attributes.front( ));

    while( !visInfo && !backoffAttributes.empty( ))
    {   // Gradually remove backoff attributes
        const int attribute = backoffAttributes.back();
        backoffAttributes.pop_back();

        std::vector<int>::iterator iter = find( attributes.begin(),
                                                attributes.end(), attribute );
        EQASSERT( iter != attributes.end( ));
        if( *iter == GLX_STENCIL_SIZE ) // two-elem attribute
            attributes.erase( iter, iter+2 );
        else                            // one-elem attribute
            attributes.erase( iter );

        visInfo = glXChooseVisual( _xDisplay, screen, &attributes.front( ));
    }

    if ( !visInfo )
        setError( ERROR_SYSTEMWINDOW_PIXELFORMAT_NOTFOUND );
    else
        EQINFO << "Using visual 0x" << std::hex << visInfo->visualid
               << std::dec << std::endl;

    return visInfo;
}


GLXContext GLXWindow::createGLXContext( XVisualInfo* visualInfo )
{
    if( !_xDisplay )
    {
        setError( ERROR_GLXWINDOW_NO_DISPLAY );
        return 0;
    }
    if( !visualInfo )
    {
        setError( ERROR_SYSTEMWINDOW_NO_PIXELFORMAT );
        return 0;
    }

    GLXContext    shareCtx    = 0;
    const Window* shareWindow = getWindow()->getSharedContextWindow();
    const SystemWindow* sysWindow = 
        shareWindow ? shareWindow->getSystemWindow() :0;
    if( sysWindow )
    {
        EQASSERT( dynamic_cast< const GLXWindowIF* >( sysWindow ));
        const GLXWindowIF* shareGLXWindow = static_cast< const GLXWindow* >(
                                                sysWindow );
        shareCtx = shareGLXWindow->getGLXContext();
    }

    GLXContext context = glXCreateContext( _xDisplay, visualInfo, shareCtx,
                                           True );
    if( !context )
    {
        setError( ERROR_GLXWINDOW_CREATECONTEXT_FAILED );
        return 0;
    }

    return context;
}

bool GLXWindow::configInitGLXDrawable( XVisualInfo* visualInfo )
{
    switch( getIAttribute( Window::IATTR_HINT_DRAWABLE ))
    {
        case PBUFFER:
            return configInitGLXPBuffer( visualInfo );

        case FBO:
        {
            const PixelViewport pvp( 0, 0, 1, 1 );
            setXDrawable( _createGLXWindow( visualInfo, pvp ));
            return (_xDrawable != 0 );
        }

        default:
            EQWARN << "Unknown drawable type "
                   << getIAttribute( Window::IATTR_HINT_DRAWABLE )
                   << ", using window" << std::endl;
            // no break;
        case UNDEFINED:
        case WINDOW:
            return configInitGLXWindow( visualInfo );
    }
}

bool GLXWindow::configInitGLXWindow( XVisualInfo* visualInfo )
{
    if( !_xDisplay )
    {
        setError( ERROR_GLXWINDOW_NO_DISPLAY );
        return false;
    }
    
    PixelViewport pvp = getWindow()->getPixelViewport();
    if( getIAttribute( Window::IATTR_HINT_FULLSCREEN ) == ON )
    {
        const int screen = DefaultScreen( _xDisplay );
    
        pvp.h = DisplayHeight( _xDisplay, screen );
        pvp.w = DisplayWidth( _xDisplay, screen );
        pvp.x = 0;
        pvp.y = 0;
        
        getWindow()->setPixelViewport( pvp );
    }
    
    XID drawable = _createGLXWindow( visualInfo, pvp );
    if( !drawable )
        return false;

    // map and wait for MapNotify event
    XMapWindow( _xDisplay, drawable );
    
    XEvent event;
    XIfEvent( _xDisplay, &event, WaitForNotify, (XPointer)(drawable) );
    
    XMoveResizeWindow( _xDisplay, drawable, pvp.x, pvp.y, pvp.w, pvp.h );
    XFlush( _xDisplay );
    
    // Grab keyboard focus in fullscreen mode
    if( getIAttribute( Window::IATTR_HINT_FULLSCREEN ) == ON )
        XGrabKeyboard( _xDisplay, drawable, True, GrabModeAsync, GrabModeAsync, 
                      CurrentTime );
    
    setXDrawable( drawable );
    
    EQINFO << "Created X11 drawable " << drawable << std::endl;
    return true;
}
    
XID GLXWindow::_createGLXWindow( XVisualInfo* visualInfo , 
                                 const PixelViewport& pvp )
{
    EQASSERT( getIAttribute( Window::IATTR_HINT_DRAWABLE ) != PBUFFER );

    if( !visualInfo )
    {
        setError( ERROR_SYSTEMWINDOW_NO_PIXELFORMAT );
        return 0;
    }

    if( !_xDisplay )
    {
        setError( ERROR_GLXWINDOW_NO_DISPLAY );
        return 0;
    }

    const int            screen = DefaultScreen( _xDisplay );
    XID                  parent = RootWindow( _xDisplay, screen );
    XSetWindowAttributes wa;
    wa.colormap          = XCreateColormap( _xDisplay, parent, visualInfo->visual,
                                            AllocNone );
    wa.background_pixmap = None;
    wa.border_pixel      = 0;
    wa.event_mask        = StructureNotifyMask | VisibilityChangeMask |
                           ExposureMask | KeyPressMask | KeyReleaseMask |
                           PointerMotionMask | ButtonPressMask |
                           ButtonReleaseMask;

    if( getIAttribute( Window::IATTR_HINT_DECORATION ) != OFF )
        wa.override_redirect = False;
    else
        wa.override_redirect = True;

    XID drawable = XCreateWindow( _xDisplay, parent, 
                                  pvp.x, pvp.y, pvp.w, pvp.h,
                                  0, visualInfo->depth, InputOutput,
                                  visualInfo->visual, 
                                  CWBackPixmap | CWBorderPixel |
                                  CWEventMask | CWColormap | CWOverrideRedirect,
                                  &wa );

    if ( !drawable )
    {
        setError( ERROR_GLXWINDOW_CREATEWINDOW_FAILED );
        return 0;
    }   

    std::stringstream windowTitle;
    const std::string& name = getWindow()->getName();

    if( name.empty( ))
    {
        windowTitle << "Equalizer";
#ifndef NDEBUG
        windowTitle << " (" << getpid() << ")";
#endif
    }
    else
        windowTitle << name;

    XStoreName( _xDisplay, drawable, windowTitle.str().c_str( ));

    // Register for close window request from the window manager
    Atom deleteAtom = XInternAtom( _xDisplay, "WM_DELETE_WINDOW", False );
    XSetWMProtocols( _xDisplay, drawable, &deleteAtom, 1 );

    return drawable;
}

bool GLXWindow::configInitGLXPBuffer( XVisualInfo* visualInfo )
{
    EQASSERT( getIAttribute( Window::IATTR_HINT_DRAWABLE ) == PBUFFER )

    if( !visualInfo )
    {
        setError( ERROR_SYSTEMWINDOW_NO_PIXELFORMAT );
        return false;
    }

    if( !_xDisplay )
    {
        setError( ERROR_GLXWINDOW_NO_DISPLAY );
        return false;
    }

    // Check for GLX >= 1.3
    int major = 0;
    int minor = 0;
    if( !glXQueryVersion( _xDisplay, &major, &minor ))
    {
        setError( ERROR_GLXWINDOW_GLXQUERYVERSION_FAILED );
        return false;
    }

    if( major < 1 || (major == 1 && minor < 3 ))
    {
        setError( ERROR_GLXWINDOW_GLX_1_3_REQUIRED );
        return false;
    }

    // Find FB config for X visual
    const int    screen   = DefaultScreen( _xDisplay );
    int          nConfigs = 0;
    GLXFBConfig* configs  = glXGetFBConfigs( _xDisplay, screen, &nConfigs );
    GLXFBConfig  config   = 0;

    for( int i = 0; i < nConfigs; ++i )
    {
        int visualID;
        if( glXGetFBConfigAttrib( _xDisplay, configs[i], GLX_VISUAL_ID, 
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
        setError( ERROR_GLXWINDOW_NO_FBCONFIG );
        return false;
    }

    // Create PBuffer
    const PixelViewport& pvp = getWindow()->getPixelViewport();
    const int attributes[] = { GLX_PBUFFER_WIDTH, pvp.w,
                               GLX_PBUFFER_HEIGHT, pvp.h,
                               GLX_LARGEST_PBUFFER, True,
                               GLX_PRESERVED_CONTENTS, True,
                               0 };

    XID pbuffer = glXCreatePbuffer( _xDisplay, config, attributes );
    if ( !pbuffer )
    {
        setError( ERROR_GLXWINDOW_CREATEPBUFFER_FAILED );
        return false;
    }   
   
    XFlush( _xDisplay );
    setXDrawable( pbuffer );

    EQINFO << "Created X11 PBuffer " << pbuffer << std::endl;
    return true;
}


void GLXWindow::setXDrawable( XID drawable )
{
    if( _xDrawable == drawable )
        return;

    if( _xDrawable )
        exitEventHandler();

    _xDrawable = drawable;

    if( !drawable )
        return;

    initEventHandler();

    // query pixel viewport of window
    EQASSERT( _xDisplay );

    switch( getIAttribute( Window::IATTR_HINT_DRAWABLE ))
    {
        case PBUFFER:
        {
            unsigned width = 0;
            unsigned height = 0;
            glXQueryDrawable( _xDisplay, drawable, GLX_WIDTH,  &width );
            glXQueryDrawable( _xDisplay, drawable, GLX_HEIGHT, &height );

            getWindow()->setPixelViewport( 
                PixelViewport( 0, 0, int32_t( width ), int32_t( height )));
            break;
        }
        case WINDOW:
        {
            XWindowAttributes wa;
            XGetWindowAttributes( _xDisplay, drawable, &wa );
    
            // position is relative to parent: translate to absolute coords
            ::Window root, parent, *children;
            unsigned nChildren;
    
            XQueryTree( _xDisplay, drawable, &root, &parent, &children,
                        &nChildren );
            if( children != 0 )
                XFree( children );

            int x,y;
            ::Window childReturn;
            XTranslateCoordinates( _xDisplay, parent, root, wa.x, wa.y, &x, &y,
                                   &childReturn );

            getWindow()->setPixelViewport( PixelViewport( x, y,
                                                      wa.width, wa.height ));
            break;
        }
        default:
            EQUNIMPLEMENTED;
        case FBO:
            EQASSERT( getWindow()->getPixelViewport().hasArea( ));
    }
}


void GLXWindow::setGLXContext( GLXContext context )
{
    _glXContext = context;
}

void GLXWindow::configExit( )
{
    if( !_xDisplay ) 
        return;

    leaveNVSwapBarrier();
    configExitFBO();
    exitGLEW();

    glXMakeCurrent( _xDisplay, None, 0 );

    GLXContext context  = getGLXContext();
    XID        drawable = getXDrawable();

    setGLXContext( 0 );
    setXDrawable( 0 );
    XSync( _xDisplay, False ); // WAR assert in glXDestroyContext/xcb_io.c:183

    if( context )
        glXDestroyContext( _xDisplay, context );

    if( drawable )
    {
        if( getIAttribute( Window::IATTR_HINT_DRAWABLE ) == PBUFFER )
            glXDestroyPbuffer( _xDisplay, drawable );
        else
            XDestroyWindow( _xDisplay, drawable );
    }

    EQINFO << "Destroyed GLX context and X drawable " << std::endl;
}

void GLXWindow::makeCurrent() const
{
    EQASSERT( _xDisplay );

    glXMakeCurrent( _xDisplay, _xDrawable, _glXContext );
    GLXWindowIF::makeCurrent();
    if( _glXContext )
    {
        EQ_GL_ERROR( "After glXMakeCurrent" );
    }
}

void GLXWindow::swapBuffers()
{
    EQASSERT( _xDisplay );
    glXSwapBuffers( _xDisplay, _xDrawable );
}

void GLXWindow::joinNVSwapBarrier( const uint32_t group, const uint32_t barrier)
{
    if( group == 0 && barrier == 0 )
        return;

#if 1
    EQWARN << "Entering untested function GLXWindow::joinNVSwapBarrier"
           << std::endl;

    if ( !GLXEW_NV_swap_group )
    {
        EQWARN << "NV Swap group extension not supported" << std::endl;
        return;
    }

    const int screen = DefaultScreen( _xDisplay );
    uint32_t maxBarrier = 0;
    uint32_t maxGroup = 0;
    
    glXQueryMaxSwapGroupsNV( _xDisplay, screen, &maxGroup, &maxBarrier );

    if( group > maxGroup )
    {
        EQWARN << "Failed to initialize GLX_NV_swap_group: requested group "
               << group << " greater than maxGroups (" << maxGroup << ")"
               << std::endl;
        return;
    }

    if( barrier > maxBarrier )
    {
        EQWARN << "Failed to initialize GLX_NV_swap_group: requested barrier "
               << barrier << "greater than maxBarriers (" << maxBarrier << ")"
               << std::endl;
        return;
    }

    if( !glXJoinSwapGroupNV( _xDisplay, _xDrawable, group ))
    {
        EQWARN << "Failed to join swap group " << group << std::endl;
        return;
    }

    _glXNVSwapGroup = group;

    if( !glXBindSwapBarrierNV( _xDisplay, group, barrier ))
    {
        EQWARN << "Failed to bind swap barrier " << barrier << std::endl;
        return;
    }
    
    EQINFO << "Joined swap group " << group << " and barrier " << barrier
           << std::endl;
#else
    EQUNIMPLEMENTED;
#endif
}

void GLXWindow::leaveNVSwapBarrier()
{
    if( _glXNVSwapGroup == 0 )
        return;

#if 1
    glXBindSwapBarrierNV( _xDisplay, _glXNVSwapGroup, 0 );
    glXJoinSwapGroupNV( _xDisplay, _xDrawable, 0 );
    
    _glXNVSwapGroup = 0;
#else
    EQUNIMPLEMENTED;
#endif
}

void GLXWindow::initEventHandler()
{
    EQASSERT( !_glXEventHandler );
    _glXEventHandler = new GLXEventHandler( this );
}

void GLXWindow::exitEventHandler()
{
    delete _glXEventHandler;
    _glXEventHandler = 0;
}

}
