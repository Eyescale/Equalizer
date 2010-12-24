
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
                          , Maxim Makhinya
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

#include "wglWindow.h"

#include "global.h"
#include "pipe.h"
#include "wglEventHandler.h"
#include "wglPipe.h"

#include <co/base/log.h>

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
    , _wglNVSwapGroup( 0 )
{
    
}

WGLWindow::~WGLWindow( )
{
}

void WGLWindow::makeCurrent() const
{
    EQCHECK( wglMakeCurrent( _wglDC, _wglContext ));
    WGLWindowIF::makeCurrent();
    if( _wglContext )
    {
        EQ_GL_ERROR( "After wglMakeCurrent" );
    }
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
    getWindow()->setPixelViewport( pvp );
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
    getWindow()->setPixelViewport( pvp );
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
        setError( ERROR_WGL_CREATEAFFINITYDC_FAILED );
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
        setError( ERROR_WGLWINDOW_NO_DRAWABLE );
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
    initGLEW();

    if( getIAttribute( Window::IATTR_HINT_SWAPSYNC ) != AUTO )
        _initSwapSync();
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
    else // no affinity, use window DC
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
        setError( ERROR_WGLWINDOW_SETPIXELFORMAT_FAILED );
        EQWARN << getError() << ": " << co::base::sysError << std::endl;
        return false;
    }

    return true;
}

bool WGLWindow::configInitWGLWindow( int pixelFormat )
{
    // adjust window size (adds border pixels)
    const PixelViewport& pvp = getWindow()->getPixelViewport();
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
        setError( ERROR_WGLWINDOW_SETPIXELFORMAT_FAILED );
        EQWARN << getError() << ": " << co::base::sysError << std::endl;
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
    const std::string& name = getWindow()->getName();

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
        setError( ERROR_WGLWINDOW_REGISTERCLASS_FAILED );
        EQWARN << getError() << ": " << co::base::sysError << std::endl;
        return false;
    }

    // window
    DWORD windowStyleEx = WS_EX_APPWINDOW;
    DWORD windowStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW;

    if( getIAttribute( Window::IATTR_HINT_DECORATION ) == OFF )
        windowStyle = WS_POPUP;

    if( getIAttribute( Window::IATTR_HINT_FULLSCREEN ) == ON )
    {
        windowStyleEx |= WS_EX_TOPMOST;

        DEVMODE deviceMode = {0};
        deviceMode.dmSize = sizeof( DEVMODE );
        EnumDisplaySettings( 0, ENUM_CURRENT_SETTINGS, &deviceMode );

        if( ChangeDisplaySettings( &deviceMode, CDS_FULLSCREEN ) != 
            DISP_CHANGE_SUCCESSFUL )
        {
            setError( ERROR_WGLWINDOW_FULLSCREEN_FAILED );
            EQWARN << getError() << ": " << co::base::sysError << std::endl;
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
        setError( ERROR_WGLWINDOW_CREATEWINDOW_FAILED );
        EQWARN << getError() << ": " << co::base::sysError << std::endl;
        return false;
    }

    return hWnd;
}

bool WGLWindow::configInitWGLPBuffer( int pf )
{
    if( !WGLEW_ARB_pbuffer )
    {
        setError( ERROR_WGLWINDOW_ARB_PBUFFER_REQUIRED );
        return false;
    }

    const eq::PixelViewport& pvp = getWindow()->getPixelViewport();
    EQASSERT( pvp.isValid( ));

    HPBUFFERARB pBuffer = 0;
    const int attr[] = { WGL_PBUFFER_LARGEST_ARB, TRUE, 0 };

    if( _wglAffinityDC )
        pBuffer = wglCreatePbufferARB( _wglAffinityDC, pf, pvp.w, pvp.h, attr );
    else
    {
        const HDC displayDC = createWGLDisplayDC();
        if( !displayDC )
            return false;

        pBuffer = wglCreatePbufferARB( displayDC, pf, pvp.w, pvp.h, attr );
        DeleteDC( displayDC );
    }

    if( !pBuffer )
    {
        setError( ERROR_WGLWINDOW_CREATEPBUFFER_FAILED );
        EQWARN << getError() << ": " << co::base::sysError << std::endl;
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
    return getWGLPipe()->createWGLAffinityDC( _wglAffinityDC );
}

void WGLWindow::_initSwapSync()
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

void WGLWindow::configExit( )
{
    leaveNVSwapBarrier();
    configExitFBO();
    exitGLEW();
    
    wglMakeCurrent( 0, 0 );

    HGLRC context        = getWGLContext();
    HWND  hWnd           = getWGLWindowHandle();
    HPBUFFERARB hPBuffer = getWGLPBufferHandle();

    exitWGLAffinityDC();
    setWGLDC( 0, WGL_DC_NONE );
    setWGLContext( 0 );
    setWGLWindowHandle( 0 );
    setWGLPBufferHandle( 0 );

    if( context )
        destroyWGLContext( context );

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
    return getWGLPipe()->createWGLDisplayDC();
}

int WGLWindow::chooseWGLPixelFormat()
{
    HDC screenDC = GetDC( 0 );
    HDC pfDC = _wglAffinityDC ? _wglAffinityDC : screenDC;

    int pixelFormat = (WGLEW_ARB_pixel_format) ? 
        _chooseWGLPixelFormatARB( pfDC ) : _chooseWGLPixelFormat( pfDC );

    ReleaseDC( 0, screenDC );

    if( pixelFormat == 0 )
    {
        setError( ERROR_SYSTEMWINDOW_PIXELFORMAT_NOTFOUND );
        EQWARN << getError() << ": " << co::base::sysError << std::endl;
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
            setError( ERROR_WGLWINDOW_SETAFFINITY_PF_FAILED );
            EQWARN << getError() << ": " << co::base::sysError << std::endl;
            return 0;
        }
    }
    
    return pixelFormat;
}

int WGLWindow::_chooseWGLPixelFormat( HDC pfDC )
{
    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize        = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    pfd.iPixelType   = PFD_TYPE_RGBA;

    const Window* window = getWindow();
    const int colorSize = window->getIAttribute( Window::IATTR_PLANES_COLOR );
    if( colorSize > 0 || colorSize == AUTO )
        pfd.cColorBits = colorSize>0 ? colorSize : 1;

    const int alphaSize = window->getIAttribute( Window::IATTR_PLANES_ALPHA );
    if( alphaSize > 0 || alphaSize == AUTO )
        pfd.cAlphaBits = alphaSize>0 ? alphaSize : 1;

    const int depthSize = window->getIAttribute( Window::IATTR_PLANES_DEPTH );
    if( depthSize > 0  || depthSize == AUTO )
        pfd.cDepthBits = depthSize>0 ? depthSize : 1;

    const int stencilSize = window->getIAttribute(Window::IATTR_PLANES_STENCIL);
    if( stencilSize >0 || stencilSize == AUTO )
        pfd.cStencilBits = stencilSize>0 ? stencilSize : 1;

    if( window->getIAttribute( Window::IATTR_HINT_STEREO ) != OFF )
        pfd.dwFlags |= PFD_STEREO;
    if( window->getIAttribute( Window::IATTR_HINT_DOUBLEBUFFER ) != OFF )
        pfd.dwFlags |= PFD_DOUBLEBUFFER;

    int pf = ChoosePixelFormat( pfDC, &pfd );

    if( pf == 0 && window->getIAttribute( Window::IATTR_HINT_STEREO ) == AUTO )
    {        
        EQINFO << "Visual not available, trying mono visual" << std::endl;
        pfd.dwFlags |= PFD_STEREO_DONTCARE;
        pf = ChoosePixelFormat( pfDC, &pfd );
    }

    if( pf == 0 && stencilSize == AUTO )
    {        
        EQINFO << "Visual not available, trying non-stencil visual"
               << std::endl;
        pfd.cStencilBits = 0;
        pf = ChoosePixelFormat( pfDC, &pfd );
    }

    if( pf == 0 &&
        window->getIAttribute( Window::IATTR_HINT_DOUBLEBUFFER ) == AUTO )
    {        
        EQINFO << "Visual not available, trying single-buffered visual" 
               << std::endl;
        pfd.dwFlags |= PFD_DOUBLEBUFFER_DONTCARE;
        pf = ChoosePixelFormat( pfDC, &pfd );
    }

    return pf;
}

int WGLWindow::_chooseWGLPixelFormatARB( HDC pfDC )
{
    EQASSERT( WGLEW_ARB_pixel_format );

    std::vector< int > attributes;
    attributes.push_back( WGL_SUPPORT_OPENGL_ARB );
    attributes.push_back( 1 );
    attributes.push_back( WGL_ACCELERATION_ARB );
    attributes.push_back( WGL_FULL_ACCELERATION_ARB );

    const int colorSize = getIAttribute( Window::IATTR_PLANES_COLOR );

    if( colorSize > 0 || colorSize == AUTO ||
        getIAttribute( Window::IATTR_HINT_DRAWABLE ) == FBO )
    {
        const int colorBits = colorSize>0 ? colorSize : 8;
        attributes.push_back( WGL_RED_BITS_ARB );
        attributes.push_back( colorBits );
        attributes.push_back( WGL_GREEN_BITS_ARB );
        attributes.push_back( colorBits );
        attributes.push_back( WGL_BLUE_BITS_ARB );
        attributes.push_back( colorBits );
    }
    else if ( colorSize == RGBA16F || colorSize == RGBA32F )
    {
        if ( !WGLEW_ARB_pixel_format_float )
        {
            setError( ERROR_WGLWINDOW_ARB_FLOAT_FB_REQUIRED );
            return 0;
        }

        const int colorBits = (colorSize == RGBA16F) ? 16 :  32;
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

    const int samplesSize = getIAttribute( Window::IATTR_PLANES_SAMPLES );
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

    while( true )
    {
        int pixelFormat = 0;
        UINT nFormats = 0;
        if( !wglChoosePixelFormatARB( pfDC, &attributes[0], 0, 1,
            &pixelFormat, &nFormats ))
        {
            setError( ERROR_WGLWINDOW_CHOOSE_PF_ARB_FAILED);
            EQWARN << getError() << ": " << co::base::sysError << std::endl;
            return 0;
        }

        if( (pixelFormat && nFormats > 0) ||  // found one or
            backoffAttributes.empty( ))       // nothing else to try
        {
            return pixelFormat;
        }

        // Gradually remove back off attributes
        const int attribute = backoffAttributes.back();
        backoffAttributes.pop_back();

        std::vector<GLint>::iterator iter = find( attributes.begin(), 
            attributes.end(), attribute );
        EQASSERT( iter != attributes.end( ));

        attributes.erase( iter, iter+2 ); // remove two items (attr, value)
    }

    return 0;
}

HGLRC WGLWindow::createWGLContext()
{
    EQASSERT( _wglDC );

    // create context
    HGLRC context = wglCreateContext( _wglAffinityDC ? _wglAffinityDC :_wglDC );
    if( !context )
    {
        setError( ERROR_WGLWINDOW_CREATECONTEXT_FAILED);
        EQWARN << getError() << ": " << co::base::sysError << std::endl;
        return 0;
    }

    // share context
    const Window* shareWindow = getWindow()->getSharedContextWindow();
    const SystemWindow* sysWindow =
        shareWindow ? shareWindow->getSystemWindow() :0;
    if( sysWindow )
    {
        EQASSERT( dynamic_cast< const WGLWindowIF* >( sysWindow ));
        const WGLWindowIF* shareWGLWindow = static_cast< const WGLWindow* >(
                                                sysWindow );
        HGLRC shareCtx = shareWGLWindow->getWGLContext();

        if( shareCtx && !wglShareLists( shareCtx, context ))
            EQWARN << "Context sharing failed: " << co::base::sysError << std::endl;
    }

    return context;
}

void WGLWindow::destroyWGLContext( HGLRC context )
{
    EQASSERT( context );
    wglDeleteContext( context );
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
    if( event.type == Event::WINDOW_EXPOSE )
    {
        EQASSERT( _wglWindow ); // PBuffers should not generate paint events

        // Invalidate update rectangle
        PAINTSTRUCT ps;
        BeginPaint( _wglWindow, &ps );
        EndPaint(   _wglWindow, &ps );
    }

    return SystemWindow::processEvent( event );
}

void WGLWindow::joinNVSwapBarrier( const uint32_t group, const uint32_t barrier)
{
    if( group == 0 && barrier == 0 )
        return;

    if( !WGLEW_NV_swap_group )
    {
        EQWARN << "NV Swap group extension not supported" << endl;
        return;
    }

    const HDC dc = _wglAffinityDC ? _wglAffinityDC : _wglDC;

    uint32_t maxBarrier = 0;
    uint32_t maxGroup = 0;
    wglQueryMaxSwapGroupsNV( dc, &maxGroup, &maxBarrier );

    if( group > maxGroup )
    {
        EQWARN << "Failed to initialize WGL_NV_swap_group: requested group "
               << group << " greater than maxGroups (" << maxGroup << ")"
               << std::endl;
        return;
    }

    if( barrier > maxBarrier )
    {
        EQWARN << "Failed to initialize WGL_NV_swap_group: requested barrier "
               << barrier << "greater than maxBarriers (" << maxBarrier << ")"
               << std::endl;
        return;
    }

    if( !wglJoinSwapGroupNV( dc, group ))
    {
        EQWARN << "Failed to join swap group " << group << std::endl;
        return;
    }
    _wglNVSwapGroup = group;

    if( !wglBindSwapBarrierNV( group, barrier ))
    {
        EQWARN << "Failed to bind swap barrier " << barrier << std::endl;
        return;
    }
    
    EQINFO << "Joined swap group " << group << " and barrier " << barrier
           << std::endl;
}

void WGLWindow::leaveNVSwapBarrier()
{
    if( _wglNVSwapGroup == 0 )
        return;

    const HDC dc = _wglAffinityDC ? _wglAffinityDC : _wglDC;

    wglBindSwapBarrierNV( _wglNVSwapGroup, 0 );
    wglJoinSwapGroupNV( dc, 0 );

    _wglNVSwapGroup = 0;
}

WGLEWContext* WGLWindow::wglewGetContext()
{
    return getWGLPipe()->wglewGetContext();
}

WGLPipe* WGLWindow::getWGLPipe()
{
    Pipe* pipe = getPipe();
    EQASSERT( pipe );
    EQASSERT( pipe->getSystemPipe( ));
    EQASSERT( dynamic_cast< WGLPipe* >( pipe->getSystemPipe( )));

    return static_cast< WGLPipe* >( pipe->getSystemPipe( ) );
}
}
