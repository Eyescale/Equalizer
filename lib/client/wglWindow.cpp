
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com>
                          , Makhinya Maxim
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#include "wglWindow.h"

#include "global.h"
#include "pipe.h"
#include "wglEventHandler.h"
#include "wglPipe.h"

#include <eq/base/log.h>

using namespace std;

namespace eq
{

WGLWindow::WGLWindow( Window* parent )
    : WGLWindowIF( parent )
    , _wglWindow( 0 )
    , _wglPBuffer( 0 )
    , _wglContext( 0 )
    , _wglDC( 0 )
    , _wglDCType( WGL_DC_NONE )
    , _wglAffinityDC( 0 )
    , _wglEventHandler( 0 )
{
    
}

WGLWindow::~WGLWindow( )
{
    
}

void WGLWindow::configExit( )
{
    wglMakeCurrent( 0, 0 );

    HGLRC context        = getWGLContext();
    HWND  hWnd           = getWGLWindowHandle();
    HPBUFFERARB hPBuffer = getWGLPBufferHandle();

    leaveNVSwapBarrier();
    exitWGLAffinityDC();
    setWGLDC( 0, WGL_DC_NONE );
    setWGLContext( 0 );
    setWGLWindowHandle( 0 );
    setWGLPBufferHandle( 0 );

    if( context )
        wglDeleteContext( context );

    if( hWnd )
    {
        // Re-enable screen saver
        if( getIAttribute( Window::IATTR_HINT_SCREENSAVER ) != ON )
            SystemParametersInfo( SPI_SETSCREENSAVEACTIVE, _screenSaverActive, 
                                  0, 0 );
        
        char className[256] = {0};
        GetClassName( hWnd, className, 255 );
        DestroyWindow( hWnd );

        if( strlen( className ) > 0 )
            UnregisterClass( className, GetModuleHandle( 0 ));
    }
    if( hPBuffer )
        wglDestroyPbufferARB( hPBuffer );

    if( getIAttribute( Window::IATTR_HINT_FULLSCREEN ) == ON )
        ChangeDisplaySettings( 0, 0 );

    EQINFO << "Destroyed WGL context and window" << std::endl;
}

void WGLWindow::makeCurrent() const
{
    EQCHECK( wglMakeCurrent( _wglDC, _wglContext ));
    WGLWindowIF::makeCurrent();
}

void WGLWindow::swapBuffers()
{
    ::SwapBuffers( _wglDC );
}

void WGLWindow::setWGLContext( HGLRC context )
{
    _wglContext = context; 
}

void WGLWindow::setWGLWindowHandle( HWND handle )
{
    if( _wglWindow == handle )
        return;

    if( _wglWindow )
        exitEventHandler();

    if( !handle )
    {
        setWGLDC( 0, WGL_DC_NONE );
        _wglWindow = 0;
        return;
    }

    _wglWindow = handle;
    setWGLDC( GetDC( handle ), WGL_DC_WINDOW );

    initEventHandler();

    // query pixel viewport of window
    WINDOWINFO windowInfo;
    windowInfo.cbSize = sizeof( windowInfo );

    GetWindowInfo( handle, &windowInfo );

    PixelViewport pvp;
    pvp.x = windowInfo.rcClient.left;
    pvp.y = windowInfo.rcClient.top;
    pvp.w = windowInfo.rcClient.right  - windowInfo.rcClient.left;
    pvp.h = windowInfo.rcClient.bottom - windowInfo.rcClient.top;
    _window->setPixelViewport( pvp );
}

void WGLWindow::setWGLPBufferHandle( HPBUFFERARB handle )
{
    if( _wglPBuffer == handle )
        return;

    if( !handle )
    {
        setWGLDC( 0, WGL_DC_NONE );
        _wglPBuffer = 0;
        return;
    }

    _wglPBuffer = handle;
    setWGLDC( wglGetPbufferDCARB( handle ), WGL_DC_PBUFFER );

    // query pixel viewport of PBuffer
    int w,h;
    wglQueryPbufferARB( handle, WGL_PBUFFER_WIDTH_ARB, &w );
    wglQueryPbufferARB( handle, WGL_PBUFFER_HEIGHT_ARB, &h );

    PixelViewport pvp;
    pvp.w = w;
    pvp.h = h;
    _window->setPixelViewport( pvp );
}

void WGLWindow::setWGLDC( HDC dc, const WGLDCType type )
{
    if( ( type != WGL_DC_NONE && dc == 0 ) ||
        ( type == WGL_DC_NONE && dc != 0 ))
    {
        EQABORT( "Illegal combination of WGL device context and type" );
        return;
    }

    switch( _wglDCType )
    {
        case WGL_DC_NONE:
            break;

        case WGL_DC_WINDOW:
            EQASSERT( _wglWindow );
            EQASSERT( _wglDC );
            ReleaseDC( _wglWindow, _wglDC );
            break;
            
        case WGL_DC_PBUFFER:
            EQASSERT( _wglPBuffer );
            EQASSERT( _wglDC );
            wglReleasePbufferDCARB( _wglPBuffer, _wglDC );
            break;

        case WGL_DC_AFFINITY:
            EQASSERT( _wglDC );
            wglDeleteDCNV( _wglDC );
            break;
                
        case WGL_DC_DISPLAY:
            EQASSERT( _wglDC );
            DeleteDC( _wglDC );
            break;

        default:
            EQUNIMPLEMENTED;
    }

    _wglDC     = dc;
    _wglDCType = type;
}

//---------------------------------------------------------------------------
// WGL init
//---------------------------------------------------------------------------
bool WGLWindow::configInit()
{
    if( !initWGLAffinityDC( ))
    {
        _window->setErrorMessage( "Can't create affinity dc" );
        return false;
    }

    int pixelFormat = chooseWGLPixelFormat();
    if( pixelFormat == 0 )
    {
        exitWGLAffinityDC();
        return false;
    }

    if( !configInitWGLDrawable( pixelFormat ))
    {
        exitWGLAffinityDC();
        return false;
    }

    if( !_wglDC )
    {
        exitWGLAffinityDC();
        setWGLDC( 0, WGL_DC_NONE );
        _window->setErrorMessage(
            "configInitWGLDrawable did not set a WGL drawable" );
        return false;
    }

    HGLRC context = createWGLContext();
    if( !context )
    {
        configExit();
        return false;
    }

    setWGLContext( context );
    makeCurrent();
    _initGlew();

    if( getIAttribute( Window::IATTR_HINT_SWAPSYNC ) != AUTO )
    {
        if( WGLEW_EXT_swap_control )
        {
            // set vsync on/off
            const GLint vsync = 
                ( getIAttribute( Window::IATTR_HINT_SWAPSYNC )==OFF ) ? 0 : 1;
            wglSwapIntervalEXT( vsync );
        }
        else
            EQWARN << "WGLEW_EXT_swap_control not supported, ignoring window "
                   << "swapsync hint" << std::endl;
    }
    
    if( !joinNVSwapBarrier( ))
    {
        _window->setErrorMessage( "Joining NV_swap_group failed" );
        return false;
    }

    if( getIAttribute( Window::IATTR_HINT_DRAWABLE ) == FBO )
        return configInitFBO();

    return true;
}

bool WGLWindow::configInitWGLDrawable( int pixelFormat )
{
    switch( getIAttribute( Window::IATTR_HINT_DRAWABLE ))
    {
        case PBUFFER:
            return configInitWGLPBuffer( pixelFormat );

        case FBO:
            return configInitWGLFBO( pixelFormat );

        default:
            EQWARN << "Unknown drawable type "
                   << getIAttribute(Window::IATTR_HINT_DRAWABLE )
                   << ", creating a window" << std::endl;
            // no break;
        case UNDEFINED:
        case WINDOW:
            return configInitWGLWindow( pixelFormat );
    }
}

bool WGLWindow::configInitWGLFBO( int pixelFormat )
{
    if( _wglAffinityDC )
    {
        // move affinity DC to be our main DC
        // deletion is now taken care of by setWGLDC( 0 )
        setWGLDC( _wglAffinityDC, WGL_DC_AFFINITY );
        _wglAffinityDC = 0;
    }
    else // no affinity, use DC of nth device
    {
        const PixelViewport pvp( 0, 0, 1, 1 );
        _wglWindow = _createWGLWindow( pixelFormat, pvp );
        if( !_wglWindow )
            return false;

        const HDC dc = GetDC( _wglWindow );
        
        if( !dc )
            return false;
        
        setWGLDC( dc, WGL_DC_WINDOW );
    }

    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize        = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion     = 1;

    DescribePixelFormat( _wglDC, pixelFormat, sizeof( pfd ), &pfd );
    if( !SetPixelFormat( _wglDC, pixelFormat, &pfd ))
    {
        _window->setErrorMessage( "Can't set window pixel format: " + 
            base::getErrorString( GetLastError( )));
        return false;
    }

    return true;
}

bool WGLWindow::configInitWGLWindow( int pixelFormat )
{
    // adjust window size (adds border pixels)
    const PixelViewport& pvp = _window->getPixelViewport();
    HWND hWnd = _createWGLWindow( pixelFormat, pvp );
    if( !hWnd )
        return false;
        
    HDC windowDC = GetDC( hWnd );
    
    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize        = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion     = 1;

    DescribePixelFormat( _wglAffinityDC ? _wglAffinityDC : windowDC, 
                         pixelFormat, sizeof(pfd), &pfd );
    if( !SetPixelFormat( windowDC, pixelFormat, &pfd ))
    {
        ReleaseDC( hWnd, windowDC );
        _window->setErrorMessage( "Can't set window pixel format: " + 
            base::getErrorString( GetLastError( )));
        return false;
    }
    ReleaseDC( hWnd, windowDC );

    ShowWindow( hWnd, SW_SHOW );
    UpdateWindow( hWnd );

    if( getIAttribute( Window::IATTR_HINT_SCREENSAVER ) != ON )
    {
        // Disable screen saver
        SystemParametersInfo( SPI_GETSCREENSAVEACTIVE, 0, &_screenSaverActive,
                              0 );
        SystemParametersInfo( SPI_SETSCREENSAVEACTIVE, FALSE, 0, 0 );
        
        // Wake up monitor
        PostMessage( HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, -1 );
    }

    setWGLWindowHandle( hWnd );
    return true;
}

HWND WGLWindow::_createWGLWindow( int pixelFormat, const PixelViewport& pvp  )
{
    // window class
    const std::string& name = _window->getName();

    std::ostringstream className;
    className << (name.empty() ? std::string("Equalizer") : name) 
              << (void*)this;
    const std::string& classStr = className.str();
                                  
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
        _window->setErrorMessage( "Can't register window class: " + 
                                  base::getLastErrorString( ));
        return false;
    }

    // window
    DWORD windowStyleEx = WS_EX_APPWINDOW;
    DWORD windowStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW;

    if( getIAttribute( Window::IATTR_HINT_DECORATION ) == OFF )
        windowStyle = WS_POPUP;

    if( getIAttribute( Window::IATTR_HINT_FULLSCREEN ) == ON )
    {
        DEVMODE deviceMode = {0};
        deviceMode.dmSize = sizeof( DEVMODE );
        EnumDisplaySettings( 0, ENUM_CURRENT_SETTINGS, &deviceMode );

        if( ChangeDisplaySettings( &deviceMode, CDS_FULLSCREEN ) != 
            DISP_CHANGE_SUCCESSFUL )
        {
            _window->setErrorMessage( "Can't switch to fullscreen mode: " + 
                base::getErrorString( GetLastError( )));
            return false;
        }
        windowStyle = WS_POPUP | WS_MAXIMIZE;
    }

    // adjust window size (adds border pixels)
    RECT rect;
    rect.left   = pvp.x;
    rect.top    = pvp.y;
    rect.right  = pvp.x + pvp.w;
    rect.bottom = pvp.y + pvp.h;
    AdjustWindowRectEx( &rect, windowStyle, FALSE, windowStyleEx );

    HWND hWnd = CreateWindowEx( windowStyleEx,
                                wc.lpszClassName, 
                                name.empty() ? "Equalizer" : name.c_str(),
                                windowStyle, rect.left, rect.top, 
                                rect.right - rect.left, rect.bottom - rect.top,
                                0, 0, // parent, menu
                                instance, 0 );
    if( !hWnd )
    {
        _window->setErrorMessage( "Can't create window: " );
        return false;
    }

    return hWnd;
}

bool WGLWindow::configInitWGLPBuffer( int pixelFormat )
{
    if( !WGLEW_ARB_pbuffer )
    {
        _window->setErrorMessage( "WGL_ARB_pbuffer not supported" );
        return false;
    }

    const eq::PixelViewport& pvp = _window->getPixelViewport();
    EQASSERT( pvp.isValid( ));

    const HDC displayDC    = createWGLDisplayDC();
    if( !displayDC )
        return false;

    const HDC dc           = _wglAffinityDC ? _wglAffinityDC : displayDC;
    const int attributes[] = { WGL_PBUFFER_LARGEST_ARB, TRUE, 0 };

    HPBUFFERARB pBuffer = wglCreatePbufferARB( dc, pixelFormat, pvp.w, pvp.h,
                                               attributes );
    DeleteDC( displayDC );
    if( !pBuffer )
    {
        _window->setErrorMessage( "Can't create PBuffer: " + 
            base::getErrorString( GetLastError( )));
        return false;
    }

    setWGLPBufferHandle( pBuffer );
    return true;
}

void WGLWindow::exitWGLAffinityDC()
{
    if( !_wglAffinityDC )
        return;

    wglDeleteDCNV( _wglAffinityDC );
    _wglAffinityDC = 0;
}

bool WGLWindow::initWGLAffinityDC()
{
    // We need to create one DC per window, since the window DC pixel format and
    // the affinity RC pixel format have to match, and each window has
    // potentially a different pixel format.
    Pipe* pipe    = getPipe();
    EQASSERT( pipe );
    EQASSERT( pipe->getOSPipe( ));
    EQASSERT( dynamic_cast< WGLPipe* >( pipe->getOSPipe( )));

    WGLPipe* osPipe = static_cast< WGLPipe* >( pipe->getOSPipe( ));

    return osPipe->createWGLAffinityDC( _wglAffinityDC );
}

HDC WGLWindow::getWGLAffinityDC()
{
    if( _wglAffinityDC )
        return _wglAffinityDC;
    if( _wglDCType == WGL_DC_AFFINITY )
        return _wglDC;
    return 0;
}

HDC WGLWindow::createWGLDisplayDC()
{
    Pipe* pipe    = getPipe();
    EQASSERT( pipe );
    EQASSERT( pipe->getOSPipe( ));
    EQASSERT( dynamic_cast< WGLPipe* >( pipe->getOSPipe( )));

    WGLPipe* osPipe = static_cast< WGLPipe* >( pipe->getOSPipe( ));

    return osPipe->createWGLDisplayDC();
}

int WGLWindow::chooseWGLPixelFormat()
{
    EQASSERT( WGLEW_ARB_pixel_format );

    std::vector< int > attributes;
    attributes.push_back( WGL_SUPPORT_OPENGL_ARB );
    attributes.push_back( 1 );
    attributes.push_back( WGL_ACCELERATION_ARB );
    attributes.push_back( WGL_FULL_ACCELERATION_ARB );

    const int colorSize = getIAttribute( Window::IATTR_PLANES_COLOR );
    const int colorBits = colorSize>0 ? colorSize : 8;

    if( colorSize > 0 || colorSize == AUTO ||
        getIAttribute( Window::IATTR_HINT_DRAWABLE ) == FBO )
    {
        attributes.push_back( WGL_COLOR_BITS_ARB );
        attributes.push_back( colorBits * 3 );
    }
    else if ( colorSize == RGBA16F || colorSize == RGBA32F )
    {
        const int colorBits = colorSize == RGBA16F ? 16 :  32;

        if ( !WGLEW_ARB_pixel_format_float )
        {
            _window->setErrorMessage( "Floating-point framebuffer unsupported" );
            return 0;
        }
        attributes.push_back( WGL_PIXEL_TYPE_ARB );
        attributes.push_back( WGL_TYPE_RGBA_FLOAT_ARB );
        
        attributes.push_back( WGL_COLOR_BITS_ARB );
        attributes.push_back( colorBits * 4);
    }

    const int alphaSize = getIAttribute( Window::IATTR_PLANES_ALPHA );
    if( alphaSize > 0 || alphaSize == AUTO )
    {
        attributes.push_back( WGL_ALPHA_BITS_ARB );
        attributes.push_back( alphaSize>0 ? alphaSize : 8 );
    }

    const int depthSize = getIAttribute( Window::IATTR_PLANES_DEPTH );
    if( depthSize > 0  || depthSize == AUTO )
    {
        attributes.push_back( WGL_DEPTH_BITS_ARB );
        attributes.push_back( depthSize>0 ? depthSize : 24 );
    }

    const int stencilSize = getIAttribute( Window::IATTR_PLANES_STENCIL );
    if( stencilSize >0 || stencilSize == AUTO )
    {
        attributes.push_back( WGL_STENCIL_BITS_ARB );
        attributes.push_back( stencilSize>0 ? stencilSize : 1 );
    }

    const int accumSize  = getIAttribute( Window::IATTR_PLANES_ACCUM );
    const int accumAlpha = getIAttribute( Window::IATTR_PLANES_ACCUM_ALPHA );
    if( accumSize >= 0 )
    {
        attributes.push_back( WGL_ACCUM_RED_BITS_ARB );
        attributes.push_back( accumSize );
        attributes.push_back( WGL_ACCUM_GREEN_BITS_ARB );
        attributes.push_back( accumSize );
        attributes.push_back( WGL_ACCUM_BLUE_BITS_ARB );
        attributes.push_back( accumSize );
        attributes.push_back( WGL_ACCUM_ALPHA_BITS_ARB );
        attributes.push_back( accumAlpha >= 0 ? accumAlpha : accumSize );
    }
    else if( accumAlpha >= 0 )
    {
        attributes.push_back( WGL_ACCUM_ALPHA_BITS_ARB );
        attributes.push_back( accumAlpha );
    }

    const int samplesSize  = getIAttribute( Window::IATTR_PLANES_SAMPLES );
    if( samplesSize >= 0 )
    {
        if( WGLEW_ARB_multisample )
        {
            attributes.push_back( WGL_SAMPLE_BUFFERS_ARB );
            attributes.push_back( 1 );
            attributes.push_back( WGL_SAMPLES_ARB );
            attributes.push_back( samplesSize );
        }
        else
            EQWARN << "WGLEW_ARB_multisample not supported, ignoring samples "
                   << "attribute" << std::endl;
    }

    if( getIAttribute( Window::IATTR_HINT_STEREO ) == ON ||
        ( getIAttribute( Window::IATTR_HINT_STEREO )   == AUTO && 
          getIAttribute( Window::IATTR_HINT_DRAWABLE ) == WINDOW ))
    {
        attributes.push_back( WGL_STEREO_ARB );
        attributes.push_back( 1 );
    }

    if( getIAttribute( Window::IATTR_HINT_DOUBLEBUFFER ) == ON ||
        ( getIAttribute( Window::IATTR_HINT_DOUBLEBUFFER ) == AUTO && 
          getIAttribute( Window::IATTR_HINT_DRAWABLE )     == WINDOW ))
    {
        attributes.push_back( WGL_DOUBLE_BUFFER_ARB );
        attributes.push_back( 1 );
    }

    if( getIAttribute( Window::IATTR_HINT_DRAWABLE ) == PBUFFER &&
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
    std::vector<int> backoffAttributes;
    if( getIAttribute( Window::IATTR_HINT_DRAWABLE ) == WINDOW )
    {
        if( getIAttribute( Window::IATTR_HINT_DOUBLEBUFFER ) == AUTO )
            backoffAttributes.push_back( WGL_DOUBLE_BUFFER_ARB );

        if( getIAttribute( Window::IATTR_HINT_STEREO ) == AUTO )
            backoffAttributes.push_back( WGL_STEREO_ARB );
    }

    if( stencilSize == AUTO )
        backoffAttributes.push_back( WGL_STENCIL_BITS_ARB );

    HDC screenDC    = GetDC( 0 );
    HDC pfDC        = _wglAffinityDC ? _wglAffinityDC : screenDC;
    int pixelFormat = 0;

    while( true )
    {
        UINT nFormats = 0;
    if( !wglChoosePixelFormatARB( pfDC, &attributes[0], 0, 1,
                                      &pixelFormat, &nFormats ))
        {
            EQWARN << "wglChoosePixelFormat failed: " 
                << base::getLastErrorString() << std::endl;
        }

        if( (pixelFormat && nFormats > 0) ||  // found one or
            backoffAttributes.empty( ))       // nothing else to try

            break;

        // Gradually remove back off attributes
        const int attribute = backoffAttributes.back();
        backoffAttributes.pop_back();

        std::vector<GLint>::iterator iter = find( attributes.begin(), 
            attributes.end(), attribute );
        EQASSERT( iter != attributes.end( ));

        attributes.erase( iter, iter+2 ); // remove two items (attr, value)

    }

    ReleaseDC( 0, screenDC );

    if( pixelFormat == 0 )
    {
        _window->setErrorMessage( "Can't find matching pixel format: " + 
            base::getErrorString( GetLastError( )));
        return 0;
    }
 
    if( _wglAffinityDC ) // set pixel format on given device context
    {
        PIXELFORMATDESCRIPTOR pfd = {0};
        pfd.nSize        = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion     = 1;

        DescribePixelFormat( _wglAffinityDC, pixelFormat, sizeof(pfd), &pfd );
        if( !SetPixelFormat( _wglAffinityDC, pixelFormat, &pfd ))
        {
            _window->setErrorMessage( "Can't set device pixel format: " + 
                base::getErrorString( GetLastError( )));
            return 0;
        }
    }
    
    return pixelFormat;
}

HGLRC WGLWindow::createWGLContext()
{
    EQASSERT( _wglDC );

    // create context
    HGLRC context = wglCreateContext(_wglAffinityDC ? _wglAffinityDC : _wglDC );
    if( !context )
    {
        _window->setErrorMessage( "Can't create OpenGL context: " + 
            base::getErrorString( GetLastError( )));
        return 0;
    }

    // share context
    const Window* shareWindow = _window->getSharedContextWindow();
    if( shareWindow )
    {
        const OSWindow*  shareOSWindow = shareWindow->getOSWindow();

        EQASSERT( dynamic_cast< const WGLWindow* >( shareOSWindow ));
        const WGLWindow* shareWGLWindow = static_cast< const WGLWindow* >(
                                              shareOSWindow );
        HGLRC shareCtx = shareWGLWindow->getWGLContext();

        if( shareCtx && !wglShareLists( shareCtx, context ))
            EQWARN << "Context sharing failed: " << base::getLastErrorString()
                   << endl;
    }

    return context;
}

void WGLWindow::initEventHandler()
{
    EQASSERT( !_wglEventHandler );
    _wglEventHandler = new WGLEventHandler( this );
}

void WGLWindow::exitEventHandler()
{
    delete _wglEventHandler;
    _wglEventHandler = 0;
}

bool WGLWindow::processEvent( const WGLWindowEvent& event )
{
    if( event.type == Event::EXPOSE )
    {
        EQASSERT( _wglWindow ); // PBuffers should not generate paint events

        // Invalidate update rectangle
        PAINTSTRUCT ps;
        BeginPaint( _wglWindow, &ps );
        EndPaint(   _wglWindow, &ps );
    }

    return WGLWindowIF::processEvent( event );
}

bool WGLWindow::joinNVSwapBarrier()
{
    const uint32_t group   = _window->getNVSwapGroup();
    const uint32_t barrier = _window->getNVSwapBarrier();
    if( group == 0 && barrier == 0 )
        return true;

    if( !WGLEW_NV_swap_group )
    {
        EQWARN << " NV Swap group not supported: " << endl;
        return true;
    }

    const HDC dc = _wglAffinityDC ? _wglAffinityDC : _wglDC;

    uint32_t maxBarrier = 0;
    uint32_t maxGroup = 0;
    wglQueryMaxSwapGroupsNV( dc, &maxGroup, &maxBarrier );

    if( group > maxGroup )
    {
        EQWARN << "Failed to initialize WGL_NV_swap_group: requested group "
               << "greater than maxGroups " << std::endl;
        return false;
    }

    if( barrier > maxBarrier )
    {
        EQWARN << "Failed to initialize WGL_NV_swap_group: requested barrier "
               << "greater than maxBarriers " << std::endl;
        return false;
    }

    if( !wglJoinSwapGroupNV( dc, group ))
    {
        EQWARN << "Failed to join swap group " << group << std::endl;
        return false;
    }

    if( !wglBindSwapBarrierNV( group, barrier ))
    {
        EQWARN << "Failed to bind swap barrier " << barrier << std::endl;
        return false;
    }
    
    return true;
}

void WGLWindow::leaveNVSwapBarrier()
{
    const uint32_t group   = _window->getNVSwapGroup();
    const uint32_t barrier = _window->getNVSwapBarrier();
    if( group == 0 && barrier == 0 )
        return;

    EQASSERT( group );

    if( !WGLEW_NV_swap_group )
        return;

    const HDC dc = _wglAffinityDC ? _wglAffinityDC : _wglDC;

    uint32_t maxBarrier = 0;
    uint32_t maxGroup = 0;
    wglQueryMaxSwapGroupsNV( dc, &maxGroup, &maxBarrier );

    if( group > maxGroup || barrier > maxBarrier )
        return;

    if( barrier )
        wglBindSwapBarrierNV( group, 0 );

    wglJoinSwapGroupNV( dc, 0 );
}
}
