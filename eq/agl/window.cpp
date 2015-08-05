
/* Copyright (c) 2005-2014, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Maxim Makhinya
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
#ifdef AGL

#include "eventHandler.h"
#include "pipe.h"
#include "windowEvent.h"
#include "../global.h"
#include "../os.h"
#include "../pipe.h"
#include "../window.h"

#include <boost/lexical_cast.hpp>

using boost::lexical_cast;

namespace eq
{
namespace agl
{
namespace
{
std::string aglError()
{
    return (const char*)( aglErrorString( aglGetError( )));
}
}
namespace detail
{
class Window
{
public:
    Window( const CGDirectDisplayID displayID, const bool threaded_ )
        : aglContext( 0 )
        , carbonWindow( 0 )
        , aglPBuffer( 0 )
        , eventHandler( 0 )
        , cgDisplayID( displayID )
        , threaded( threaded_ )
    {}

    /** The AGL context. */
    AGLContext aglContext;

    /** The carbon window reference. */
    WindowRef carbonWindow;

    /** The AGL PBuffer object. */
    AGLPbuffer aglPBuffer;

    /** The AGL event handler. */
    EventHandler* eventHandler;

    /** Carbon display identifier. */
    const CGDirectDisplayID cgDisplayID;

    const bool threaded;
};
}

Window::Window( NotifierInterface& parent, const WindowSettings& settings,
                const CGDirectDisplayID displayID, const bool threaded )
    : WindowIF( parent, settings )
    , _impl( new detail::Window( displayID, threaded ))
{
}

Window::~Window()
{
    delete _impl;
}

void Window::configExit()
{
    WindowRef window = getCarbonWindow();
    setCarbonWindow( 0 );

    AGLPbuffer pbuffer = getAGLPBuffer();
    setAGLPBuffer( 0 );

    AGLContext context = getAGLContext();

    if( getIAttribute( WindowSettings::IATTR_HINT_FULLSCREEN ) == ON )
    {
        LBASSERT( !window );
        exitEventHandler();
    }
    else if( window )
    {
        Global::enterCarbon();
        aglSetWindowRef( context, 0 );
        DisposeWindow( window );
        Global::leaveCarbon();
    }
    if( pbuffer )
        aglDestroyPBuffer( pbuffer );

    configExitFBO();
    exitGLEW();

    if( context )
    {
        Global::enterCarbon();
        aglSetCurrentContext( 0 );
        aglDestroyContext( context );
        Global::leaveCarbon();
        setAGLContext( 0 );
    }

    LBVERB << "Destroyed AGL window and context" << std::endl;
}

void Window::makeCurrent( const bool cache ) const
{
    if( cache && isCurrent( ))
        return;

    aglSetCurrentContext( _impl->aglContext );
    WindowIF::makeCurrent();

    if( _impl->aglContext )
    {
        EQ_GL_ERROR( "After aglSetCurrentContext" );
    }
}

void Window::doneCurrent() const
{
    if( !isCurrent( ))
        return;

    aglSetCurrentContext( 0 );
    WindowIF::doneCurrent();
}

void Window::swapBuffers()
{
    aglSwapBuffers( _impl->aglContext );
}

void Window::joinNVSwapBarrier( const uint32_t, const uint32_t )
{
    LBWARN << "NV_swap_group not supported on AGL" << std::endl;
}

bool Window::processEvent( const WindowEvent& event )
{
    if( event.type == Event::WINDOW_RESIZE && _impl->aglContext )
        aglUpdateContext( _impl->aglContext );

    return SystemWindow::processEvent( event );
}

void Window::setAGLContext( AGLContext context )
{
    _impl->aglContext = context;
}

AGLContext Window::getAGLContext() const
{
    return _impl->aglContext;
}

WindowRef Window::getCarbonWindow() const
{
    return _impl->carbonWindow;
}

AGLPbuffer Window::getAGLPBuffer() const
{
    return _impl->aglPBuffer;
}

CGDirectDisplayID Window::getCGDisplayID() const
{
    return _impl->cgDisplayID;
}

bool Window::isThreaded() const
{
    return _impl->threaded;
}

//---------------------------------------------------------------------------
// AGL init
//---------------------------------------------------------------------------

bool Window::configInit()
{
    AGLPixelFormat pixelFormat = chooseAGLPixelFormat();
    if( !pixelFormat )
        return false;

    AGLContext context = createAGLContext( pixelFormat );
    destroyAGLPixelFormat ( pixelFormat );
    setAGLContext( context );

    if( !context )
        return false;

    makeCurrent();
    initGLEW();
    return configInitAGLDrawable();
}

AGLPixelFormat Window::chooseAGLPixelFormat()
{
    Global::enterCarbon();

    CGOpenGLDisplayMask glDisplayMask =
        CGDisplayIDToOpenGLDisplayMask( _impl->cgDisplayID );

    // build attribute list
    std::vector<GLint> attributes;

    attributes.push_back( AGL_RGBA );
    attributes.push_back( GL_TRUE );
    attributes.push_back( AGL_ACCELERATED );
    attributes.push_back( GL_TRUE );

    if( getIAttribute( WindowSettings::IATTR_HINT_FULLSCREEN ) == ON )
        attributes.push_back( AGL_FULLSCREEN );

    attributes.push_back( AGL_DISPLAY_MASK );
    attributes.push_back( glDisplayMask );

    GLint colorSize = getIAttribute( WindowSettings::IATTR_PLANES_COLOR );
    if( colorSize != OFF )
    {
        switch( colorSize )
        {
          case RGBA16F:
            attributes.push_back( AGL_COLOR_FLOAT );
            colorSize = 16;
            break;
          case RGBA32F:
            attributes.push_back( AGL_COLOR_FLOAT );
            colorSize = 32;
            break;
          case AUTO:
            colorSize = 8;
            break;
          default:
              break;
        }

        attributes.push_back( AGL_RED_SIZE );
        attributes.push_back( colorSize );
        attributes.push_back( AGL_GREEN_SIZE );
        attributes.push_back( colorSize );
        attributes.push_back( AGL_BLUE_SIZE );
        attributes.push_back( colorSize );

        const int alphaSize = getIAttribute( WindowSettings::IATTR_PLANES_ALPHA );
        if( alphaSize > 0 || alphaSize == AUTO )
        {
            attributes.push_back( AGL_ALPHA_SIZE );
            attributes.push_back( alphaSize > 0 ? alphaSize : colorSize );
        }
    }

    const int depthSize = getIAttribute( WindowSettings::IATTR_PLANES_DEPTH );
    if( depthSize > 0 || depthSize == AUTO )
    {
        attributes.push_back( AGL_DEPTH_SIZE );
        attributes.push_back( depthSize>0 ? depthSize : 24 );
    }
    const int stencilSize = getIAttribute( WindowSettings::IATTR_PLANES_STENCIL );
    if( stencilSize > 0 || stencilSize == AUTO )
    {
        attributes.push_back( AGL_STENCIL_SIZE );
        attributes.push_back( stencilSize>0 ? stencilSize : 1 );
    }
    const int accumSize  = getIAttribute( WindowSettings::IATTR_PLANES_ACCUM );
    const int accumAlpha = getIAttribute( WindowSettings::IATTR_PLANES_ACCUM_ALPHA );
    if( accumSize >= 0 )
    {
        attributes.push_back( AGL_ACCUM_RED_SIZE );
        attributes.push_back( accumSize );
        attributes.push_back( AGL_ACCUM_GREEN_SIZE );
        attributes.push_back( accumSize );
        attributes.push_back( AGL_ACCUM_BLUE_SIZE );
        attributes.push_back( accumSize );
        attributes.push_back( AGL_ACCUM_ALPHA_SIZE );
        attributes.push_back( accumAlpha >= 0 ? accumAlpha : accumSize );
    }
    else if( accumAlpha >= 0 )
    {
        attributes.push_back( AGL_ACCUM_ALPHA_SIZE );
        attributes.push_back( accumAlpha );
    }

    const int samplesSize  = getIAttribute( WindowSettings::IATTR_PLANES_SAMPLES );
    if( samplesSize >= 0 )
    {
        attributes.push_back( AGL_SAMPLE_BUFFERS_ARB );
        attributes.push_back( 1 );
        attributes.push_back( AGL_SAMPLES_ARB );
        attributes.push_back( samplesSize );
    }

    if( getIAttribute( WindowSettings::IATTR_HINT_DOUBLEBUFFER ) == ON ||
        ( getIAttribute( WindowSettings::IATTR_HINT_DOUBLEBUFFER ) == AUTO &&
          getIAttribute( WindowSettings::IATTR_HINT_DRAWABLE )     == WINDOW ))
    {
        attributes.push_back( AGL_DOUBLEBUFFER );
        attributes.push_back( GL_TRUE );
    }
    if( getIAttribute( WindowSettings::IATTR_HINT_STEREO ) == ON )
    {
        attributes.push_back( AGL_STEREO );
        attributes.push_back( GL_TRUE );
    }

    attributes.push_back( AGL_NONE );

    // build backoff list, least important attribute last
    std::vector<int> backoffAttributes;
    if( getIAttribute( WindowSettings::IATTR_HINT_DOUBLEBUFFER ) == AUTO &&
        getIAttribute( WindowSettings::IATTR_HINT_DRAWABLE )     == WINDOW  )

        backoffAttributes.push_back( AGL_DOUBLEBUFFER );

    if( stencilSize == AUTO )
        backoffAttributes.push_back( AGL_STENCIL_SIZE );

    // choose pixel format
    AGLPixelFormat pixelFormat = 0;
    while( true )
    {
        pixelFormat = aglCreatePixelFormat( &attributes.front( ));

        if( pixelFormat ||              // found one or
            backoffAttributes.empty( )) // nothing else to try

            break;

        // Gradually remove backoff attributes
        const GLint attribute = backoffAttributes.back();
        backoffAttributes.pop_back();

        std::vector<GLint>::iterator iter = find( attributes.begin(),
                                             attributes.end(), attribute );
        LBASSERT( iter != attributes.end( ));

        attributes.erase( iter, iter+2 ); // remove two item (attr, value)
    }

    if( !pixelFormat )
        sendError( ERROR_SYSTEMWINDOW_PIXELFORMAT_NOTFOUND );

    Global::leaveCarbon();
    return pixelFormat;
}

void Window::destroyAGLPixelFormat( AGLPixelFormat pixelFormat )
{
    if( !pixelFormat )
        return;

    Global::enterCarbon();
    aglDestroyPixelFormat( pixelFormat );
    Global::leaveCarbon();
}

AGLContext Window::createAGLContext( AGLPixelFormat pixelFormat )
{
    if( !pixelFormat )
    {
        sendError( ERROR_SYSTEMWINDOW_NO_PIXELFORMAT );
        return 0;
    }

    const WindowIF* aglShareWindow =
                dynamic_cast< const WindowIF* >( getSharedContextWindow( ));
    AGLContext shareCtx = aglShareWindow ? aglShareWindow->getAGLContext() : 0;

    Global::enterCarbon();
    AGLContext context = aglCreateContext( pixelFormat, shareCtx );

    if( !context )
    {
        sendError( ERROR_AGLWINDOW_CREATECONTEXT_FAILED ) << aglError();
        Global::leaveCarbon();
        return 0;
    }

    _initSwapSync( context );
    aglSetCurrentContext( context );

    Global::leaveCarbon();

    LBVERB << "Created AGL context " << context << " shared with " << shareCtx
           << std::endl;
    return context;
}


bool Window::configInitAGLDrawable()
{
    switch( getIAttribute( WindowSettings::IATTR_HINT_DRAWABLE ))
    {
        case PBUFFER:
            return configInitAGLPBuffer();

        case FBO:
            return configInitFBO();

        default:
            LBWARN << "Unknown drawable type "
                   << getIAttribute( WindowSettings::IATTR_HINT_DRAWABLE )
                   << ", using window" << std::endl;
            // no break;
        case UNDEFINED:
        case WINDOW:
            if( getIAttribute( WindowSettings::IATTR_HINT_FULLSCREEN ) == ON )
                return configInitAGLFullscreen();
            return configInitAGLWindow();

        case OFF:
            return true;
    }
}

bool Window::configInitAGLPBuffer()
{
    AGLContext context = getAGLContext();
    if( !context )
    {
        sendError( ERROR_AGLWINDOW_NO_CONTEXT );
        return false;
    }

    // PBuffer
    const PixelViewport& pvp = getPixelViewport();
    AGLPbuffer pbuffer;
    if( !aglCreatePBuffer( pvp.w, pvp.h, GL_TEXTURE_RECTANGLE_EXT, GL_RGBA,
                           0, &pbuffer ))
    {
        sendError( ERROR_AGLWINDOW_CREATEPBUFFER_FAILED ) << aglError();
        return false;
    }

    // attach to context
    if( !aglSetPBuffer( context, pbuffer, 0, 0, aglGetVirtualScreen( context )))
    {
        sendError( ERROR_AGLWINDOW_SETPBUFFER_FAILED ) << aglError();
        return false;
    }

    setAGLPBuffer( pbuffer );
    return true;
}

bool Window::configInitAGLFullscreen()
{
    AGLContext context = getAGLContext();
    if( !context )
    {
        sendError( ERROR_AGLWINDOW_NO_CONTEXT );
        return false;
    }

    Global::enterCarbon();
    aglEnable( context, AGL_FS_CAPTURE_SINGLE );

    const PixelViewport& pvp = getPixelViewport();
    if( !aglSetFullScreen( context, pvp.w, pvp.h, 0, 0 ))
        LBWARN << "aglSetFullScreen to " << pvp << " failed: " << aglError()
               << std::endl;

    // Do focus hell
    ProcessSerialNumber selfProcess = { 0, kCurrentProcess };
    SetFrontProcess( &selfProcess );

    Global::leaveCarbon();

    setPixelViewport( pvp );

    if( getIAttribute( WindowSettings::IATTR_HINT_DRAWABLE ) != OFF )
        initEventHandler();
    return true;
}

bool Window::configInitAGLWindow()
{
    AGLContext context = getAGLContext();
    if( !context )
    {
        sendError( ERROR_AGLWINDOW_NO_CONTEXT );
        return false;
    }

    // window
    const bool decoration =
        getIAttribute(WindowSettings::IATTR_HINT_DECORATION) != OFF;
    WindowAttributes winAttributes = ( decoration ?
                                       kWindowStandardDocumentAttributes :
                                       kWindowNoTitleBarAttribute |
                                       kWindowNoShadowAttribute   |
                                       kWindowResizableAttribute  ) |
        kWindowStandardHandlerAttribute | kWindowInWindowMenuAttribute;

    // top, left, bottom, right
    const PixelViewport& pvp = getPixelViewport();
    const int32_t menuHeight = decoration ? EQ_AGL_MENUBARHEIGHT : 0 ;
    Rect windowRect = { short( pvp.y + menuHeight ), short( pvp.x ),
                        short( pvp.getYEnd() + menuHeight ),
                               short( pvp.getXEnd( )) };
    WindowRef windowRef;

    Global::enterCarbon();
    const OSStatus status = CreateNewWindow( kDocumentWindowClass,
                                             winAttributes,
                                             &windowRect, &windowRef );
    if( status != noErr )
    {
        sendError( ERROR_AGLWINDOW_CREATEWINDOW_FAILED )
            << lexical_cast< std::string >( status );
        Global::leaveCarbon();
        return false;
    }

    // window title
    const std::string& name = getName();
    std::stringstream windowTitle;

    if( name.empty( ))
    {
        windowTitle << "Equalizer";
#ifndef NDEBUG
        windowTitle << " (" << getpid() << ")";
#endif
    }
    else
        windowTitle << name;

    CFStringRef title = CFStringCreateWithCString( kCFAllocatorDefault,
                                                   windowTitle.str().c_str(),
                                                   kCFStringEncodingMacRoman );
    SetWindowTitleWithCFString( windowRef, title );
    CFRelease( title );

    if( !aglSetWindowRef( context, windowRef ))
    {
        sendError( ERROR_AGLWINDOW_SETWINDOW_FAILED ) << aglError();
        Global::leaveCarbon();
        return false;
    }

    // show
    ShowWindow( windowRef );

    // Do focus hell
    ProcessSerialNumber selfProcess = { 0, kCurrentProcess };
    SetFrontProcess( &selfProcess );

    Global::leaveCarbon();
    setCarbonWindow( windowRef );

    return true;
}

void Window::_initSwapSync( AGLContext context )
{
    if( getIAttribute( WindowSettings::IATTR_HINT_DRAWABLE ) == OFF )
        return;

    int32_t swapSync = getIAttribute( WindowSettings::IATTR_HINT_SWAPSYNC );
    if( swapSync == AUTO )
        return;

    if( swapSync < 0 )
        swapSync = 1;

    aglSetInteger( context, AGL_SWAP_INTERVAL, &swapSync );
}

void Window::setCarbonWindow( WindowRef window )
{
    LBVERB << "set Carbon window " << window << std::endl;

    if( _impl->carbonWindow == window )
        return;

    if( _impl->carbonWindow )
        exitEventHandler();
    _impl->carbonWindow = window;

    if( !window )
        return;

    if( getIAttribute( WindowSettings::IATTR_HINT_DRAWABLE ) == OFF )
        return;

    initEventHandler();

    Rect rect;
    Global::enterCarbon();
    if( GetWindowBounds( window, kWindowContentRgn, &rect ) == noErr )
    {
        PixelViewport pvp( rect.left, rect.top,
                           rect.right - rect.left, rect.bottom - rect.top );

        if( getIAttribute( WindowSettings::IATTR_HINT_DECORATION ) != OFF )
            pvp.y -= EQ_AGL_MENUBARHEIGHT;

        setPixelViewport( pvp );
    }
    Global::leaveCarbon();
}

void Window::setAGLPBuffer( AGLPbuffer pbuffer )
{
    LBVERB << "set AGL PBuffer " << pbuffer << std::endl;

    if( _impl->aglPBuffer == pbuffer )
        return;

    _impl->aglPBuffer = pbuffer;

    if( !pbuffer )
        return;

    GLint         w;
    GLint         h;
    GLenum        target;
    GLenum        format;
    GLint         maxLevel;

    if( aglDescribePBuffer( pbuffer, &w, &h, &target, &format, &maxLevel ))
    {
        LBASSERT( target == GL_TEXTURE_RECTANGLE_EXT );

        const PixelViewport pvp( 0, 0, w, h );
        setPixelViewport( pvp );
    }
}

void Window::initEventHandler()
{
    LBASSERT( !_impl->eventHandler );
    _impl->eventHandler = new EventHandler( this );
}

void Window::exitEventHandler()
{
    delete _impl->eventHandler;
    _impl->eventHandler = 0;
}

}
}
#endif // AGL
