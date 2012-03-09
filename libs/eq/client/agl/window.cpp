
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com>
                      2010, Maxim Makhinya
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
#include "../os.h"
#include "../pipe.h"
#include "../window.h"

#define AGLERROR std::string( (const char*)( aglErrorString( aglGetError( ))))

namespace eq
{
namespace agl
{

Window::Window( eq::Window* parent, CGDirectDisplayID displayID )
    : WindowIF( parent )
    , _aglContext( 0 )
    , _carbonWindow( 0 )
    , _aglPBuffer( 0 )
    , _eventHandler( 0 )
    , _cgDisplayID( displayID )
{
    if( displayID == kCGNullDirectDisplay )
    {
        const eq::Pipe* pipe = getPipe();
        EQASSERT( pipe );
        EQASSERT( pipe->getSystemPipe( ));

        const Pipe* aglPipe = dynamic_cast<const Pipe*>( pipe->getSystemPipe());
        if( aglPipe )
            _cgDisplayID = aglPipe->getCGDisplayID();
    }
}

Window::~Window()
{
}

void Window::configExit( )
{
    WindowRef window = getCarbonWindow();
    setCarbonWindow( 0 );

    AGLPbuffer pbuffer = getAGLPBuffer();
    setAGLPBuffer( 0 );
    
    AGLContext context = getAGLContext();

    if( getIAttribute( eq::Window::IATTR_HINT_FULLSCREEN ) == ON )
    {
        EQASSERT( !window );
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
    
    EQVERB << "Destroyed AGL window and context" << std::endl;
}

void Window::makeCurrent() const
{
    aglSetCurrentContext( _aglContext );
    WindowIF::makeCurrent();
    
    if( _aglContext )
    {
        EQ_GL_ERROR( "After aglSetCurrentContext" );
    }
}

void Window::swapBuffers()
{
    aglSwapBuffers( _aglContext );
}

void Window::joinNVSwapBarrier( const uint32_t group, const uint32_t barrier)
{
    EQWARN << "NV_swap_group not supported on AGL" << std::endl;
}

bool Window::processEvent( const WindowEvent& event )
{
    if( event.type == Event::WINDOW_RESIZE && _aglContext )
        aglUpdateContext( _aglContext );

    return SystemWindow::processEvent( event );
}

void Window::setAGLContext( AGLContext context )
{
    _aglContext = context;
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
        CGDisplayIDToOpenGLDisplayMask( _cgDisplayID );

    // build attribute list
    std::vector<GLint> attributes;

    attributes.push_back( AGL_RGBA );
    attributes.push_back( GL_TRUE );
    attributes.push_back( AGL_ACCELERATED );
    attributes.push_back( GL_TRUE );

    if( getIAttribute( eq::Window::IATTR_HINT_FULLSCREEN ) == ON && 
       (getIAttribute( eq::Window::IATTR_HINT_DRAWABLE )   == WINDOW ||
        getIAttribute( eq::Window::IATTR_HINT_DRAWABLE )   == FBO ))
    {
        attributes.push_back( AGL_FULLSCREEN );
    }

    attributes.push_back( AGL_DISPLAY_MASK );
    attributes.push_back( glDisplayMask );

    GLint colorSize = getIAttribute( eq::Window::IATTR_PLANES_COLOR );
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

        const int alphaSize = getIAttribute( eq::Window::IATTR_PLANES_ALPHA );
        if( alphaSize > 0 || alphaSize == AUTO )
        {
            attributes.push_back( AGL_ALPHA_SIZE );
            attributes.push_back( alphaSize > 0 ? alphaSize : colorSize );
        }
    }

    const int depthSize = getIAttribute( eq::Window::IATTR_PLANES_DEPTH );
    if( depthSize > 0 || depthSize == AUTO )
    { 
        attributes.push_back( AGL_DEPTH_SIZE );
        attributes.push_back( depthSize>0 ? depthSize : 24 );
    }
    const int stencilSize = getIAttribute( eq::Window::IATTR_PLANES_STENCIL );
    if( stencilSize > 0 || stencilSize == AUTO )
    {
        attributes.push_back( AGL_STENCIL_SIZE );
        attributes.push_back( stencilSize>0 ? stencilSize : 1 );
    }
    const int accumSize  = getIAttribute( eq::Window::IATTR_PLANES_ACCUM );
    const int accumAlpha = getIAttribute( eq::Window::IATTR_PLANES_ACCUM_ALPHA );
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

    const int samplesSize  = getIAttribute( eq::Window::IATTR_PLANES_SAMPLES );
    if( samplesSize >= 0 )
    {
        attributes.push_back( AGL_SAMPLE_BUFFERS_ARB );
        attributes.push_back( 1 );
        attributes.push_back( AGL_SAMPLES_ARB );
        attributes.push_back( samplesSize );
    }

    if( getIAttribute( eq::Window::IATTR_HINT_DOUBLEBUFFER ) == ON ||
        ( getIAttribute( eq::Window::IATTR_HINT_DOUBLEBUFFER ) == AUTO && 
          getIAttribute( eq::Window::IATTR_HINT_DRAWABLE )     == WINDOW ))
    {
        attributes.push_back( AGL_DOUBLEBUFFER );
        attributes.push_back( GL_TRUE );
    }
    if( getIAttribute( eq::Window::IATTR_HINT_STEREO ) == ON )
    {
        attributes.push_back( AGL_STEREO );
        attributes.push_back( GL_TRUE );
    }

    attributes.push_back( AGL_NONE );

    // build backoff list, least important attribute last
    std::vector<int> backoffAttributes;
    if( getIAttribute( eq::Window::IATTR_HINT_DOUBLEBUFFER ) == AUTO &&
        getIAttribute( eq::Window::IATTR_HINT_DRAWABLE )     == WINDOW  )

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
        EQASSERT( iter != attributes.end( ));

        attributes.erase( iter, iter+2 ); // remove two item (attr, value)
    }

    if( !pixelFormat )
        setError( ERROR_SYSTEMWINDOW_PIXELFORMAT_NOTFOUND );

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
        setError( ERROR_SYSTEMWINDOW_NO_PIXELFORMAT );
        return 0;
    }

    AGLContext shareCtx = 0;
    const eq::Window* shareWindow = getWindow()->getSharedContextWindow();
    const SystemWindow* sysWindow =
        shareWindow ? shareWindow->getSystemWindow() :0;
    if( sysWindow )
    {
        const WindowIF* aglShareWindow = EQSAFECAST(const WindowIF*, sysWindow);
        shareCtx = aglShareWindow->getAGLContext();
    }
 
    Global::enterCarbon();
    AGLContext context = aglCreateContext( pixelFormat, shareCtx );

    if( !context ) 
    {
        setError( ERROR_AGLWINDOW_CREATECONTEXT_FAILED );
        EQWARN << getError() << ": " << AGLERROR << std::endl;
        Global::leaveCarbon();
        return 0;
    }

    // set vsync on/off
    int32_t swapSync = getIAttribute( eq::Window::IATTR_HINT_SWAPSYNC );
    if( swapSync != AUTO )
    {
        if( swapSync < 0 )
            swapSync = 1;
        aglSetInteger( context, AGL_SWAP_INTERVAL, &swapSync );
    }

    aglSetCurrentContext( context );

    Global::leaveCarbon();

    EQVERB << "Created AGL context " << context << " shared with " << shareCtx
           << std::endl;
    return context;
}


bool Window::configInitAGLDrawable()
{
    switch( getIAttribute( eq::Window::IATTR_HINT_DRAWABLE ))
    {
        case PBUFFER:
            return configInitAGLPBuffer();

        case FBO:
            return configInitFBO();

        default:
            EQWARN << "Unknown drawable type "
                   << getIAttribute( eq::Window::IATTR_HINT_DRAWABLE )
                   << ", using window" << std::endl;
            // no break;
        case UNDEFINED:
        case WINDOW:
            if( getIAttribute( eq::Window::IATTR_HINT_FULLSCREEN ) == ON )
                return configInitAGLFullscreen();
            else
                return configInitAGLWindow();
    }
}

bool Window::configInitAGLPBuffer()
{
    AGLContext context = getAGLContext();
    if( !context )
    {
        setError( ERROR_AGLWINDOW_NO_CONTEXT );
        return false;
    }

    // PBuffer
    const PixelViewport pvp = getWindow()->getPixelViewport();
          AGLPbuffer    pbuffer;
    if( !aglCreatePBuffer( pvp.w, pvp.h, GL_TEXTURE_RECTANGLE_EXT, GL_RGBA,
                           0, &pbuffer ))
    {
        setError( ERROR_AGLWINDOW_CREATEPBUFFER_FAILED );
        EQWARN << getError() << ": " << AGLERROR << std::endl;
        return false;
    }

    // attach to context
    if( !aglSetPBuffer( context, pbuffer, 0, 0, aglGetVirtualScreen( context )))
    {
        setError( ERROR_AGLWINDOW_SETPBUFFER_FAILED );
        EQWARN << getError() << ": " << AGLERROR << std::endl;
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
        setError( ERROR_AGLWINDOW_NO_CONTEXT );
        return false;
    }

    Global::enterCarbon();
    aglEnable( context, AGL_FS_CAPTURE_SINGLE );

    const eq::Pipe* pipe = getPipe();
    const PixelViewport& pipePVP   = pipe->getPixelViewport();
    const PixelViewport& windowPVP = getWindow()->getPixelViewport();
    const PixelViewport& pvp       = pipePVP.isValid() ? pipePVP : windowPVP;

    if( !aglSetFullScreen( context, pvp.w, pvp.h, 0, 0 ))
        EQWARN << "aglSetFullScreen to " << pvp << " failed: " << AGLERROR
               << std::endl;

    // Do focus hell
    ProcessSerialNumber selfProcess = { 0, kCurrentProcess };
    SetFrontProcess( &selfProcess );

    Global::leaveCarbon();

    getWindow()->setPixelViewport( pvp );
    initEventHandler();
    return true;
}

bool Window::configInitAGLWindow()
{
    AGLContext context = getAGLContext();
    if( !context )
    {
        setError( ERROR_AGLWINDOW_NO_CONTEXT );
        return false;
    }

    // window
    const bool decoration =
        getIAttribute(eq::Window::IATTR_HINT_DECORATION) != OFF;
    WindowAttributes winAttributes = ( decoration ? 
                                       kWindowStandardDocumentAttributes :
                                       kWindowNoTitleBarAttribute | 
                                       kWindowNoShadowAttribute   |
                                       kWindowResizableAttribute  ) | 
        kWindowStandardHandlerAttribute | kWindowInWindowMenuAttribute;

    // top, left, bottom, right
    const PixelViewport   pvp = getWindow()->getPixelViewport();
    const int32_t  menuHeight = decoration ? EQ_AGL_MENUBARHEIGHT : 0 ;
    Rect           windowRect = { pvp.y + menuHeight, pvp.x, 
                                  pvp.y + pvp.h + menuHeight,
                                  pvp.x + pvp.w };
    WindowRef      windowRef;

    Global::enterCarbon();
    const OSStatus status = CreateNewWindow( kDocumentWindowClass,
                                             winAttributes,
                                             &windowRect, &windowRef );
    if( status != noErr )
    {
        setError( ERROR_AGLWINDOW_CREATEWINDOW_FAILED );
        EQWARN << getError() << ": " << status << std::endl;
        Global::leaveCarbon();
        return false;
    }

    // window title
    const std::string& name = getWindow()->getName();
    std::stringstream windowTitle;

    if( name.empty( ))
    {
        windowTitle << "Equalizer";
#ifndef NDEBUG
        windowTitle << " (" << getpid() << ")";
#endif;
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
        setError( ERROR_AGLWINDOW_SETWINDOW_FAILED );
        EQWARN << getError() << ": " << AGLERROR << std::endl;
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

void Window::setCarbonWindow( WindowRef window )
{
    EQVERB << "set Carbon window " << window << std::endl;

    if( _carbonWindow == window )
        return;

    if( _carbonWindow )
        exitEventHandler();
    _carbonWindow = window;

    if( !window )
        return;

    initEventHandler();

    Rect rect;
    Global::enterCarbon();
    if( GetWindowBounds( window, kWindowContentRgn, &rect ) == noErr )
    {
        PixelViewport pvp( rect.left, rect.top,
                           rect.right - rect.left, rect.bottom - rect.top );

        if( getIAttribute( eq::Window::IATTR_HINT_DECORATION ) != OFF )
            pvp.y -= EQ_AGL_MENUBARHEIGHT;

        getWindow()->setPixelViewport( pvp );
    }
    Global::leaveCarbon();
}

void Window::setAGLPBuffer( AGLPbuffer pbuffer )
{
    EQVERB << "set AGL PBuffer " << pbuffer << std::endl;

    if( _aglPBuffer == pbuffer )
        return;

    _aglPBuffer = pbuffer;

    if( !pbuffer )
        return;

    GLint         w;
    GLint         h;
    GLenum        target;
    GLenum        format;
    GLint         maxLevel;

    if( aglDescribePBuffer( pbuffer, &w, &h, &target, &format, &maxLevel ))
    {
        EQASSERT( target == GL_TEXTURE_RECTANGLE_EXT );

        const PixelViewport pvp( 0, 0, w, h );
        getWindow()->setPixelViewport( pvp );
    }
}

void Window::initEventHandler()
{
    EQASSERT( !_eventHandler );
    _eventHandler = new EventHandler( this );
}

void Window::exitEventHandler()
{
    delete _eventHandler;
    _eventHandler = 0;
}

}
}
