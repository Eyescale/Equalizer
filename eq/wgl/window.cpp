
/* Copyright (c) 2005-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Maxim Makhinya
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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
#include "../window.h"

#include <eq/global.h>
#include <eq/pipe.h>

#include <lunchbox/log.h>

namespace eq
{
namespace wgl
{

/** The type of the Win32 device context. */
enum WGLDCType
{
    WGL_DC_NONE,     //!< No device context is set
    WGL_DC_WINDOW,   //!< The WGL DC belongs to a window
    WGL_DC_PBUFFER,  //!< The WGL DC belongs to a PBuffer
    WGL_DC_AFFINITY, //!< The WGL DC is an affinity DC
    WGL_DC_DISPLAY   //!< The WGL DC is a display DC
};

namespace detail
{
class Window
{
public:
    explicit Window( Pipe& pipe )
        : _wglWindow( 0 )
        , _wglPBuffer( 0 )
        , _wglContext( 0 )
        , _wglDC( 0 )
        , _wglDCType( WGL_DC_NONE )
        , _wglAffinityDC( 0 )
        , _wglEventHandler( 0 )
        , _screenSaverActive( FALSE )
        , _wglNVSwapGroup( 0 )
        , _wglPipe( pipe )
    {}

    WGLEWContext* wglewGetContext()
    {
        return _wglPipe.wglewGetContext();
    }

    /**
     * Set new device context and release the old DC.
     *
     * @param dc the new DC.
     * @param type the type of the new DC.
     */
    void setWGLDC( HDC dc, const WGLDCType type )
    {
        if( ( type != WGL_DC_NONE && dc == 0 ) ||
            ( type == WGL_DC_NONE && dc != 0 ))
        {
            LBABORT( "Illegal combination of WGL device context and type" );
            return;
        }

        switch( _wglDCType )
        {
            case WGL_DC_NONE:
                break;

            case WGL_DC_WINDOW:
                LBASSERT( _wglWindow );
                LBASSERT( _wglDC );
                ReleaseDC( _wglWindow, _wglDC );
                break;

            case WGL_DC_PBUFFER:
                LBASSERT( _wglPBuffer );
                LBASSERT( _wglDC );
                wglReleasePbufferDCARB( _wglPBuffer, _wglDC );
                break;

            case WGL_DC_AFFINITY:
                LBASSERT( _wglDC );
                wglDeleteDCNV( _wglDC );
                break;

            case WGL_DC_DISPLAY:
                LBASSERT( _wglDC );
                DeleteDC( _wglDC );
                break;

            default:
                LBUNIMPLEMENTED;
        }

        _wglDC     = dc;
        _wglDCType = type;
    }

    HWND          _wglWindow;
    HPBUFFERARB   _wglPBuffer;
    HGLRC         _wglContext;

    HDC           _wglDC;
    WGLDCType     _wglDCType;
    HDC           _wglAffinityDC;

    EventHandler* _wglEventHandler;
    BOOL          _screenSaverActive;

    uint32_t      _wglNVSwapGroup;

    Pipe&         _wglPipe;
};
}

Window::Window( NotifierInterface& parent, const WindowSettings& settings,
                Pipe& pipe )
    : WindowIF( parent, settings )
    , _impl( new detail::Window( pipe ))
{
}

Window::~Window( )
{
    delete _impl;
}

void Window::makeCurrent( const bool cache ) const
{
    if( cache && isCurrent( ))
        return;

    LBCHECK( wglMakeCurrent( _impl->_wglDC, _impl->_wglContext ));
    WindowIF::makeCurrent();
    if( _impl->_wglContext )
    {
        EQ_GL_ERROR( "After wglMakeCurrent" );
    }
}

void Window::doneCurrent() const
{
    if( !isCurrent( ))
        return;

    LBCHECK( wglMakeCurrent( 0, 0 ));
    WindowIF::doneCurrent();
}

void Window::swapBuffers()
{
    ::SwapBuffers( _impl->_wglDC );
}

void Window::setWGLContext( HGLRC context )
{
    _impl->_wglContext = context;
}

void Window::setWGLWindowHandle( HWND handle )
{
    if( _impl->_wglWindow == handle )
        return;

    if( _impl->_wglWindow )
        exitEventHandler();

    if( !handle )
    {
        _impl->setWGLDC( 0, WGL_DC_NONE );
        _impl->_wglWindow = 0;
        return;
    }

    _impl->_wglWindow = handle;
    _impl->setWGLDC( GetDC( handle ), WGL_DC_WINDOW );

    if( getIAttribute( eq::WindowSettings::IATTR_HINT_DRAWABLE ) == OFF )
        return;

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
    setPixelViewport( pvp );
}

void Window::setWGLPBufferHandle( HPBUFFERARB handle )
{
    if( _impl->_wglPBuffer == handle )
        return;

    if( !handle )
    {
        _impl->setWGLDC( 0, WGL_DC_NONE );
        _impl->_wglPBuffer = 0;
        return;
    }

    _impl->_wglPBuffer = handle;
    _impl->setWGLDC( wglGetPbufferDCARB( handle ), WGL_DC_PBUFFER );

    // query pixel viewport of PBuffer
    int w,h;
    wglQueryPbufferARB( handle, WGL_PBUFFER_WIDTH_ARB, &w );
    wglQueryPbufferARB( handle, WGL_PBUFFER_HEIGHT_ARB, &h );

    PixelViewport pvp;
    pvp.w = w;
    pvp.h = h;
    setPixelViewport( pvp );
}

HWND Window::getWGLWindowHandle() const
{
    return _impl->_wglWindow;
}

HPBUFFERARB Window::getWGLPBufferHandle() const
{
    return _impl->_wglPBuffer;
}

HDC Window::getWGLDC() const
{
    return _impl->_wglDC;
}

HGLRC Window::getWGLContext() const
{
    return _impl->_wglContext;
}

const EventHandler* Window::getWGLEventHandler() const
{
    return _impl->_wglEventHandler;
}

//---------------------------------------------------------------------------
// WGL init
//---------------------------------------------------------------------------
bool Window::configInit()
{
    if( !initWGLAffinityDC( ))
    {
        sendError( ERROR_WGL_CREATEAFFINITYDC_FAILED );
        return false;
    }

    const int pixelFormat = chooseWGLPixelFormat();
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

    if( !_impl->_wglDC )
    {
        exitWGLAffinityDC();
        _impl->setWGLDC( 0, WGL_DC_NONE );
        sendError( ERROR_WGLWINDOW_NO_DRAWABLE );
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
    _initSwapSync();
    if( getIAttribute( eq::WindowSettings::IATTR_HINT_DRAWABLE ) == FBO )
        return configInitFBO();

    return true;
}

bool Window::configInitWGLDrawable( int pixelFormat )
{
    switch( getIAttribute( eq::WindowSettings::IATTR_HINT_DRAWABLE ))
    {
        case PBUFFER:
            return configInitWGLPBuffer( pixelFormat );

        case FBO:
        case OFF:
            return configInitWGLFBO( pixelFormat );

        default:
            LBWARN << "Unknown drawable type "
                   << getIAttribute(eq::WindowSettings::IATTR_HINT_DRAWABLE )
                   << ", creating a window" << std::endl;
            // no break;
        case UNDEFINED:
        case WINDOW:
            return configInitWGLWindow( pixelFormat );
    }
}

bool Window::configInitWGLFBO( int pixelFormat )
{
    if( _impl->_wglAffinityDC )
    {
        // move affinity DC to be our main DC
        // deletion is now taken care of by setWGLDC( 0 )
        _impl->setWGLDC( _impl->_wglAffinityDC, WGL_DC_AFFINITY );
        _impl->_wglAffinityDC = 0;
    }
    else // no affinity, use window DC
    {
        const PixelViewport pvp( 0, 0, 1, 1 );
        _impl->_wglWindow = _createWGLWindow( pvp );
        if( !_impl->_wglWindow )
            return false;

        const HDC dc = GetDC( _impl->_wglWindow );
        if( !dc )
            return false;

        _impl->setWGLDC( dc, WGL_DC_WINDOW );
    }

    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize        = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion     = 1;

    DescribePixelFormat( _impl->_wglDC, pixelFormat, sizeof( pfd ), &pfd );
    if( !SetPixelFormat( _impl->_wglDC, pixelFormat, &pfd ))
    {
        sendError( ERROR_WGLWINDOW_SETPIXELFORMAT_FAILED )
            << lunchbox::sysError();
        return false;
    }

    return true;
}

bool Window::configInitWGLWindow( int pixelFormat )
{
    // adjust window size (adds border pixels)
    const PixelViewport& pvp = getPixelViewport();
    HWND hWnd = _createWGLWindow( pvp );
    if( !hWnd )
        return false;

    HDC windowDC = GetDC( hWnd );

    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize        = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion     = 1;

    DescribePixelFormat( _impl->_wglAffinityDC ? _impl->_wglAffinityDC
                                               : windowDC,
                         pixelFormat, sizeof(pfd), &pfd );
    if( !SetPixelFormat( windowDC, pixelFormat, &pfd ))
    {
        ReleaseDC( hWnd, windowDC );
        sendError( ERROR_WGLWINDOW_SETPIXELFORMAT_FAILED )
            << lunchbox::sysError();
        return false;
    }
    ReleaseDC( hWnd, windowDC );

    ShowWindow( hWnd, SW_SHOW );
    UpdateWindow( hWnd );

    if( getIAttribute( eq::WindowSettings::IATTR_HINT_SCREENSAVER ) != ON )
    {
        // Disable screen saver
        SystemParametersInfo( SPI_GETSCREENSAVEACTIVE, 0,
                              &_impl->_screenSaverActive, 0 );
        SystemParametersInfo( SPI_SETSCREENSAVEACTIVE, FALSE, 0, 0 );

        // Wake up monitor
        PostMessage( HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, -1 );
    }

    setWGLWindowHandle( hWnd );
    return true;
}

HWND Window::_createWGLWindow( const PixelViewport& pvp  )
{
    // window class
    const std::string& name = getName();

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
        sendError( ERROR_WGLWINDOW_REGISTERCLASS_FAILED )
            << lunchbox::sysError();
        return false;
    }

    // window
    DWORD windowStyleEx = WS_EX_APPWINDOW;
    DWORD windowStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW;

    if( getIAttribute( eq::WindowSettings::IATTR_HINT_DECORATION ) == OFF )
        windowStyle = WS_POPUP;

    if( getIAttribute( eq::WindowSettings::IATTR_HINT_FULLSCREEN ) == ON )
    {
        windowStyleEx |= WS_EX_TOPMOST;

        DEVMODE deviceMode = {0};
        deviceMode.dmSize = sizeof( DEVMODE );
        EnumDisplaySettings( 0, ENUM_CURRENT_SETTINGS, &deviceMode );

        if( ChangeDisplaySettings( &deviceMode, CDS_FULLSCREEN ) !=
            DISP_CHANGE_SUCCESSFUL )
        {
            sendError( ERROR_WGLWINDOW_FULLSCREEN_FAILED )
                << lunchbox::sysError();
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
        sendError( ERROR_WGLWINDOW_CREATEWINDOW_FAILED )
            << lunchbox::sysError();
        return false;
    }

    return hWnd;
}

bool Window::configInitWGLPBuffer( int pf )
{
    if( !WGLEW_ARB_pbuffer )
    {
        sendError( ERROR_WGLWINDOW_ARB_PBUFFER_REQUIRED );
        return false;
    }

    const eq::PixelViewport& pvp = getPixelViewport();
    LBASSERT( pvp.isValid( ));

    HPBUFFERARB pBuffer = 0;
    const int attr[] = { WGL_PBUFFER_LARGEST_ARB, TRUE, 0 };

    if( _impl->_wglAffinityDC )
        pBuffer = wglCreatePbufferARB( _impl->_wglAffinityDC, pf, pvp.w, pvp.h,
                                       attr );
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
        sendError( ERROR_WGLWINDOW_CREATEPBUFFER_FAILED )
            << lunchbox::sysError();
        return false;
    }

    setWGLPBufferHandle( pBuffer );
    return true;
}

void Window::exitWGLAffinityDC()
{
    if( !_impl->_wglAffinityDC )
        return;

    wglDeleteDCNV( _impl->_wglAffinityDC );
    _impl->_wglAffinityDC = 0;
}

bool Window::initWGLAffinityDC()
{
    // We need to create one DC per window, since the window DC pixel format and
    // the affinity RC pixel format have to match, and each window has
    // potentially a different pixel format.
    return _impl->_wglPipe.createWGLAffinityDC( _impl->_wglAffinityDC );
}

void Window::_initSwapSync()
{
    if( getIAttribute( eq::WindowSettings::IATTR_HINT_DRAWABLE ) == OFF )
        return;

    const int32_t swapSync =
            getIAttribute( eq::WindowSettings::IATTR_HINT_SWAPSYNC );
    if( swapSync == AUTO ) // leave it alone
        return;

    if( WGLEW_EXT_swap_control )
        wglSwapIntervalEXT( (swapSync < 0) ? 1 : swapSync );
    else
        LBWARN << "WGL_EXT_swap_control not supported, ignoring window "
               << "swapsync hint " << swapSync << std::endl;
}

void Window::configExit( )
{
    leaveNVSwapBarrier();
    configExitFBO();
    exitGLEW();

    wglMakeCurrent( 0, 0 );

    HGLRC context        = getWGLContext();
    HWND  hWnd           = getWGLWindowHandle();
    HPBUFFERARB hPBuffer = getWGLPBufferHandle();

    exitWGLAffinityDC();
    _impl->setWGLDC( 0, WGL_DC_NONE );
    setWGLContext( 0 );
    setWGLWindowHandle( 0 );
    setWGLPBufferHandle( 0 );

    if( context )
        destroyWGLContext( context );

    if( hWnd )
    {
        // Re-enable screen saver
        if( getIAttribute( eq::WindowSettings::IATTR_HINT_SCREENSAVER ) != ON )
            SystemParametersInfo( SPI_SETSCREENSAVEACTIVE,
                                  _impl->_screenSaverActive, 0, 0 );

        char className[256] = {0};
        GetClassName( hWnd, className, 255 );
        DestroyWindow( hWnd );

        if( strlen( className ) > 0 )
            UnregisterClass( className, GetModuleHandle( 0 ));
    }
    if( hPBuffer )
        wglDestroyPbufferARB( hPBuffer );

    if( getIAttribute( eq::WindowSettings::IATTR_HINT_FULLSCREEN ) == ON )
        ChangeDisplaySettings( 0, 0 );

    LBVERB << "Destroyed WGL context and window" << std::endl;
}

HDC Window::getWGLAffinityDC()
{
    if( _impl->_wglAffinityDC )
        return _impl->_wglAffinityDC;
    if( _impl->_wglDCType == WGL_DC_AFFINITY )
        return _impl->_wglDC;
    return 0;
}

HDC Window::createWGLDisplayDC()
{
    return _impl->_wglPipe.createWGLDisplayDC();
}

int Window::chooseWGLPixelFormat()
{
    HDC screenDC = GetDC( 0 );
    HDC pfDC = _impl->_wglAffinityDC ? _impl->_wglAffinityDC : screenDC;

    const int pixelFormat = (WGLEW_ARB_pixel_format) ?
        _chooseWGLPixelFormatARB( pfDC ) : _chooseWGLPixelFormat( pfDC );

    ReleaseDC( 0, screenDC );

    if( pixelFormat == 0 )
    {
        sendError( ERROR_SYSTEMWINDOW_PIXELFORMAT_NOTFOUND )
            << lunchbox::sysError();
        return 0;
    }

    if( _impl->_wglAffinityDC ) // set pixel format on given device context
    {
        PIXELFORMATDESCRIPTOR pfd = {0};
        pfd.nSize        = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion     = 1;

        DescribePixelFormat( _impl->_wglAffinityDC, pixelFormat, sizeof(pfd),
                             &pfd );
        if( !SetPixelFormat( _impl->_wglAffinityDC, pixelFormat, &pfd ))
        {
            sendError( ERROR_WGLWINDOW_SETAFFINITY_PF_FAILED )
                << lunchbox::sysError();
            return 0;
        }
    }

    return pixelFormat;
}

int Window::_chooseWGLPixelFormat( HDC pfDC )
{
    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize        = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    pfd.iPixelType   = PFD_TYPE_RGBA;

    const int colorSize =
            getIAttribute( eq::WindowSettings::IATTR_PLANES_COLOR );
    if( colorSize > 0 || colorSize == AUTO )
        pfd.cColorBits = colorSize>0 ? colorSize : 1;

    const int alphaSize =
            getIAttribute( eq::WindowSettings::IATTR_PLANES_ALPHA );
    if( alphaSize > 0 || alphaSize == AUTO )
        pfd.cAlphaBits = alphaSize>0 ? alphaSize : 1;

    const int depthSize =
            getIAttribute( eq::WindowSettings::IATTR_PLANES_DEPTH );
    if( depthSize > 0 || depthSize == AUTO )
        pfd.cDepthBits = depthSize>0 ? depthSize : 1;

    const int stencilSize =
            getIAttribute(eq::WindowSettings::IATTR_PLANES_STENCIL );
    if( stencilSize > 0 || stencilSize == AUTO )
        pfd.cStencilBits = stencilSize > 0 ? stencilSize : 1;

    if( getIAttribute( eq::WindowSettings::IATTR_HINT_STEREO ) != OFF )
        pfd.dwFlags |= PFD_STEREO;
    if( getIAttribute( eq::WindowSettings::IATTR_HINT_DOUBLEBUFFER ) != OFF )
        pfd.dwFlags |= PFD_DOUBLEBUFFER;

    int pf = ChoosePixelFormat( pfDC, &pfd );

    if( pf == 0 &&
        getIAttribute( eq::WindowSettings::IATTR_HINT_STEREO ) == AUTO )
    {
        LBINFO << "Visual not available, trying mono visual" << std::endl;
        pfd.dwFlags |= PFD_STEREO_DONTCARE;
        pf = ChoosePixelFormat( pfDC, &pfd );
    }

    if( pf == 0 && stencilSize == AUTO )
    {
        LBINFO << "Visual not available, trying non-stencil visual"
               << std::endl;
        pfd.cStencilBits = 0;
        pf = ChoosePixelFormat( pfDC, &pfd );
    }

    if( pf == 0 &&
        getIAttribute( eq::WindowSettings::IATTR_HINT_DOUBLEBUFFER ) == AUTO )
    {
        LBINFO << "Visual not available, trying single-buffered visual"
               << std::endl;
        pfd.dwFlags |= PFD_DOUBLEBUFFER_DONTCARE;
        pf = ChoosePixelFormat( pfDC, &pfd );
    }

    return pf;
}

int Window::_chooseWGLPixelFormatARB( HDC pfDC )
{
    LBASSERT( WGLEW_ARB_pixel_format );

    std::vector< int > attributes;
    attributes.push_back( WGL_SUPPORT_OPENGL_ARB );
    attributes.push_back( 1 );
    attributes.push_back( WGL_ACCELERATION_ARB );
    attributes.push_back( WGL_FULL_ACCELERATION_ARB );

    const int colorSize = getIAttribute( eq::WindowSettings::IATTR_PLANES_COLOR );
    const int32_t drawableHint =
            getIAttribute( eq::WindowSettings::IATTR_HINT_DRAWABLE );
    if( colorSize > 0 || colorSize == AUTO || drawableHint == FBO ||
        drawableHint == OFF )
    {
        // Create FBO dummy window with 8bpp
        const int colorBits = colorSize>0 ? colorSize : 8;
        attributes.push_back( WGL_RED_BITS_ARB );
        attributes.push_back( colorBits );
        attributes.push_back( WGL_GREEN_BITS_ARB );
        attributes.push_back( colorBits );
        attributes.push_back( WGL_BLUE_BITS_ARB );
        attributes.push_back( colorBits );
    }
    else if( colorSize == RGBA16F || colorSize == RGBA32F )
    {
        if ( !WGLEW_ARB_pixel_format_float )
        {
            sendError( ERROR_SYSTEMWINDOW_ARB_FLOAT_FB_REQUIRED );
            return 0;
        }

        const int colorBits = (colorSize == RGBA16F) ? 16 :  32;
        attributes.push_back( WGL_PIXEL_TYPE_ARB );
        attributes.push_back( WGL_TYPE_RGBA_FLOAT_ARB );
        attributes.push_back( WGL_COLOR_BITS_ARB );
        attributes.push_back( colorBits * 4);
    }

    const int alphaSize =
            getIAttribute( eq::WindowSettings::IATTR_PLANES_ALPHA );
    if( alphaSize > 0 || alphaSize == AUTO )
    {
        attributes.push_back( WGL_ALPHA_BITS_ARB );
        attributes.push_back( alphaSize>0 ? alphaSize : 8 );
    }

    const int depthSize =
            getIAttribute( eq::WindowSettings::IATTR_PLANES_DEPTH );
    if( depthSize > 0 || depthSize == AUTO )
    {
        attributes.push_back( WGL_DEPTH_BITS_ARB );
        attributes.push_back( depthSize>0 ? depthSize : 24 );
    }

    const int stencilSize =
            getIAttribute( eq::WindowSettings::IATTR_PLANES_STENCIL );
    if( stencilSize > 0 || stencilSize == AUTO )
    {
        attributes.push_back( WGL_STENCIL_BITS_ARB );
        attributes.push_back( stencilSize>0 ? stencilSize : 1 );
    }

    const int accumSize  =
            getIAttribute( eq::WindowSettings::IATTR_PLANES_ACCUM );
    const int accumAlpha =
            getIAttribute( eq::WindowSettings::IATTR_PLANES_ACCUM_ALPHA );
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

    const int samplesSize =
            getIAttribute( eq::WindowSettings::IATTR_PLANES_SAMPLES );
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
            LBWARN << "WGLEW_ARB_multisample not supported, ignoring samples "
                   << "attribute" << std::endl;
    }

    if( getIAttribute( eq::WindowSettings::IATTR_HINT_STEREO ) == ON ||
        ( getIAttribute( eq::WindowSettings::IATTR_HINT_STEREO ) == AUTO &&
          drawableHint == WINDOW ))
    {
        attributes.push_back( WGL_STEREO_ARB );
        attributes.push_back( 1 );
    }

    if( getIAttribute( eq::WindowSettings::IATTR_HINT_DOUBLEBUFFER ) == ON ||
        ( getIAttribute( eq::WindowSettings::IATTR_HINT_DOUBLEBUFFER ) == AUTO &&
          drawableHint == WINDOW ))
    {
        attributes.push_back( WGL_DOUBLE_BUFFER_ARB );
        attributes.push_back( 1 );
    }

    if( drawableHint == PBUFFER && WGLEW_ARB_pbuffer )
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
    if( drawableHint == WINDOW )
    {
        if( getIAttribute( eq::WindowSettings::IATTR_HINT_DOUBLEBUFFER ) == AUTO )
            backoffAttributes.push_back( WGL_DOUBLE_BUFFER_ARB );

        if( getIAttribute( eq::WindowSettings::IATTR_HINT_STEREO ) == AUTO )
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
            sendError( ERROR_WGLWINDOW_CHOOSE_PF_ARB_FAILED)
                << lunchbox::sysError();
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
        LBASSERT( iter != attributes.end( ));

        attributes.erase( iter, iter+2 ); // remove two items (attr, value)
    }

    return 0;
}

HGLRC Window::createWGLContext()
{
    LBASSERT( _impl->_wglDC );

    const WindowIF* shareWGLWindow =
            dynamic_cast< const WindowIF* >( getSharedContextWindow( ));
    HGLRC shareCtx = 0;
    if( shareWGLWindow )
        shareCtx = shareWGLWindow->getWGLContext();

    const bool coreContext =
        getIAttribute( WindowSettings::IATTR_HINT_CORE_PROFILE ) == ON;

    // create context
    HGLRC context = 0;
    if( wglCreateContextAttribsARB && coreContext )
    {
        int attribList[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB,
            getIAttribute( WindowSettings::IATTR_HINT_OPENGL_MAJOR ),
            WGL_CONTEXT_MINOR_VERSION_ARB,
            getIAttribute( WindowSettings::IATTR_HINT_OPENGL_MINOR ),
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };

        context = wglCreateContextAttribsARB( _impl->_wglAffinityDC ?
            _impl->_wglAffinityDC : _impl->_wglDC, shareCtx, attribList );
    }
    else
    {
        context = wglCreateContext( _impl->_wglAffinityDC
                                      ? _impl->_wglAffinityDC : _impl->_wglDC );
    }

    if( !context )
    {
        sendError( ERROR_WGLWINDOW_CREATECONTEXT_FAILED )
            << lunchbox::sysError();
        return 0;
    }

    // share context
    if( !coreContext && shareCtx && !wglShareLists( shareCtx, context ))
        LBWARN << "Context sharing failed: " << lunchbox::sysError << std::endl;

    return context;
}

void Window::destroyWGLContext( HGLRC context )
{
    LBASSERT( context );
    wglDeleteContext( context );
}

void Window::initEventHandler()
{
    LBASSERT( !_impl->_wglEventHandler );
    _impl->_wglEventHandler = new EventHandler( this );
}

void Window::exitEventHandler()
{
    delete _impl->_wglEventHandler;
    _impl->_wglEventHandler = 0;
}

bool Window::processEvent( const WindowEvent& event )
{
    switch( event.type )
    {
    case Event::WINDOW_EXPOSE:
    {
        LBASSERT( _impl->_wglWindow ); // PBuffers should not generate paint events

        // Invalidate update rectangle
        PAINTSTRUCT ps;
        BeginPaint( _impl->_wglWindow, &ps );
        EndPaint(   _impl->_wglWindow, &ps );
        break;
    }

    case Event::WINDOW_POINTER_BUTTON_PRESS:
        if( getIAttribute( eq::WindowSettings::IATTR_HINT_GRAB_POINTER ) == ON &&
            // If no other button was pressed already, capture the mouse
            event.pointerButtonPress.buttons == event.pointerButtonPress.button )
        {
            SetCapture( getWGLWindowHandle( ));
            WindowEvent grabEvent = event;
            grabEvent.type = Event::WINDOW_POINTER_GRAB;
            processEvent( grabEvent );
        }
        break;

    case Event::WINDOW_POINTER_BUTTON_RELEASE:
        if( getIAttribute( eq::WindowSettings::IATTR_HINT_GRAB_POINTER ) == ON &&
            // If no button is pressed anymore, release the mouse
            event.pointerButtonRelease.buttons == PTR_BUTTON_NONE )
        {
            // Call early for consistent ordering
            const bool result = SystemWindow::processEvent( event );

            WindowEvent ungrabEvent = event;
            ungrabEvent.type = Event::WINDOW_POINTER_UNGRAB;
            processEvent( ungrabEvent );
            ReleaseCapture();
            return result;
        }
        break;
    }
    return SystemWindow::processEvent( event );
}

void Window::joinNVSwapBarrier( const uint32_t group, const uint32_t barrier)
{
    if( group == 0 )
        return;

    if( !WGLEW_NV_swap_group )
    {
        LBWARN << "NV Swap group extension not supported" << std::endl;
        return;
    }

    uint32_t maxBarrier = 0;
    uint32_t maxGroup = 0;
    wglQueryMaxSwapGroupsNV( _impl->_wglDC, &maxGroup, &maxBarrier );

    if( group > maxGroup )
    {
        LBWARN << "Failed to initialize WGL_NV_swap_group: requested group "
               << group << " greater than maxGroups (" << maxGroup << ")"
               << std::endl;
        return;
    }

    if( barrier > maxBarrier )
    {
        LBWARN << "Failed to initialize WGL_NV_swap_group: requested barrier "
               << barrier << " greater than maxBarriers (" << maxBarrier << ")"
               << std::endl;
        return;
    }

    if( !wglJoinSwapGroupNV( _impl->_wglDC, group ))
    {
        LBWARN << "Failed to join swap group " << group << std::endl;
        return;
    }
    _impl->_wglNVSwapGroup = group;

    if( !wglBindSwapBarrierNV( group, barrier ))
    {
        LBWARN << "Failed to bind swap barrier " << barrier << std::endl;
        return;
    }

    LBINFO << "Joined swap group " << group << " and barrier " << barrier
           << std::endl;
}

void Window::leaveNVSwapBarrier()
{
    if( _impl->_wglNVSwapGroup == 0 )
        return;

    if( !wglBindSwapBarrierNV( _impl->_wglNVSwapGroup, 0 ))
        LBWARN << "Failed to unbind swap barrier" << std::endl;

    if( !wglJoinSwapGroupNV( _impl->_wglDC, 0 ))
        LBWARN << "Failed to unjoin swap group " << std::endl;

    _impl->_wglNVSwapGroup = 0;
}

WGLEWContext* Window::wglewGetContext()
{
    return _impl->wglewGetContext();
}

}
}
