/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com>
                          , Makhinya Maxim
   All rights reserved. */

#include "wglWindow.h"

#include "global.h"
#include "wglEventHandler.h"

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
    , _eventHandler( 0 )
{
    
}

WGLWindow::~WGLWindow( )
{
    
}

void WGLWindow::configExit( )
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

    if( getIAttribute( Window::IATTR_HINT_FULLSCREEN ) == ON )
        ChangeDisplaySettings( 0, 0 );

    EQINFO << "Destroyed WGL context and window" << std::endl;
#endif
}

void WGLWindow::makeCurrent() const
{
#ifdef WGL
    wglMakeCurrent( _wglDC, _wglContext );
#endif
}

void WGLWindow::swapBuffers()
{
#ifdef WGL
    SwapBuffers( _wglDC );
#endif
}

void WGLWindow::setWGLContext( HGLRC context )
{
#ifdef WGL
    _wglContext = context; 
#endif
}


void WGLWindow::setWGLWindowHandle( HWND handle )
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

    if( !handle )
        return;

    initEventHandler();
    _wglDC = GetDC( handle );

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
#endif // WGL
}

void WGLWindow::setWGLPBufferHandle( HPBUFFERARB handle )
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

    if( !handle )
        return;

    _wglDC = wglGetPbufferDCARB( handle );

    // query pixel viewport of PBuffer
    int w,h;
    wglQueryPbufferARB( handle, WGL_PBUFFER_WIDTH_ARB, &w );
    wglQueryPbufferARB( handle, WGL_PBUFFER_HEIGHT_ARB, &h );

    PixelViewport pvp;
    pvp.w = w;
    pvp.h = h;
    _window->setPixelViewport( pvp );
#endif // WGL
}


//---------------------------------------------------------------------------
// WGL init
//---------------------------------------------------------------------------
bool WGLWindow::configInit()
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
        _window->setErrorMessage( "configInitWGLDrawable did not set a WGL drawable" );
        return false;
    }

    HGLRC context = createWGLContext( dc );
    setWGLContext( context );

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
    _window->setErrorMessage( "Client library compiled without WGL support" );
    return false;
#endif
}

bool WGLWindow::configInitWGLDrawable( HDC dc, int pixelFormat )
{
    switch( getIAttribute( Window::IATTR_HINT_DRAWABLE ))
    {
        case PBUFFER:
            return configInitWGLPBuffer( dc, pixelFormat );

        default:
            EQWARN << "Unknown drawable type "
                   << getIAttribute(Window::IATTR_HINT_DRAWABLE )
                   << ", creating a window" << std::endl;
            // no break;
        case UNDEFINED:
        case WINDOW:
            return configInitWGLWindow( dc, pixelFormat );
    }
}

bool WGLWindow::configInitWGLWindow( HDC dc, int pixelFormat )
{
#ifdef WGL
    // window class
    const std::string& name = _window->getName();

    std::ostringstream className;
    className << (name.empty() ? std::string("Equalizer") : name) << (void*)this;
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
    const PixelViewport& pvp = _window->getPixelViewport();
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
        _window->setErrorMessage( "Can't create window: " + 
                         base::getErrorString( GetLastError( )));
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
        _window->setErrorMessage( "Can't set window pixel format: " + 
            base::getErrorString( GetLastError( )));
        return false;
    }
    ReleaseDC( hWnd, windowDC );

    setWGLWindowHandle( hWnd );
    ShowWindow( hWnd, SW_SHOW );
    UpdateWindow( hWnd );

    return true;
#else
    _window->setErrorMessage( "Client library compiled without WGL support" );
    return false;
#endif
}

bool WGLWindow::configInitWGLPBuffer( HDC overrideDC, int pixelFormat )
{
#ifdef WGL
    if( !WGLEW_ARB_pbuffer )
    {
        _window->setErrorMessage( "WGL_ARB_pbuffer not supported" );
        return false;
    }

    const eq::PixelViewport& pvp = _window->getPixelViewport();
    EQASSERT( pvp.isValid( ));

    HDC displayDC = CreateDC( "DISPLAY", 0, 0, 0 );
    HDC dc       = overrideDC ? overrideDC : displayDC;
    const int attributes[] = { WGL_PBUFFER_LARGEST_ARB, TRUE, 0 };

    HPBUFFERARB pBuffer = wglCreatePbufferARB( dc, pixelFormat, pvp.w, pvp.h,
                                               attributes );
    if( !pBuffer )
    {
        _window->setErrorMessage( "Can't create PBuffer: " + 
            base::getErrorString( GetLastError( )));

        DeleteDC( displayDC );
        return false;
    }

    DeleteDC( displayDC );

    setWGLPBufferHandle( pBuffer );
    return true;
#else
    _window->setErrorMessage( "Client library compiled without WGL support" );
    return false;
#endif
}

HDC WGLWindow::getWGLPipeDC( PFNEQDELETEDCPROC& deleteProc )
{
#ifdef WGL
    // per-GPU affinity DC
    // We need to create one DC per window, since the window DC pixel format and
    // the affinity RC pixel format have to match, and each window has
    // potentially a different pixel format.
    Pipe* pipe    = getPipe();
    EQASSERT( pipe );

    HDC affinityDC = 0;
    if( !pipe->createAffinityDC( affinityDC, deleteProc ))
    {
        _window->setErrorMessage( "Can't create affinity dc" );
        return 0;
    }

    return affinityDC;
#else
    _window->setErrorMessage( "Client library compiled without WGL support" );
    return 0;
#endif
}

int WGLWindow::chooseWGLPixelFormat( HDC dc )
{
#ifdef WGL
    EQASSERT( WGLEW_ARB_pixel_format );

    std::vector< int > attributes;
    attributes.push_back( WGL_SUPPORT_OPENGL_ARB );
    attributes.push_back( 1 );
    attributes.push_back( WGL_ACCELERATION_ARB );
    attributes.push_back( WGL_FULL_ACCELERATION_ARB );

    const int colorSize = getIAttribute( Window::IATTR_PLANES_COLOR );
    if( colorSize > 0 || colorSize == AUTO )
    {
        const int colorBits = colorSize>0 ? colorSize : 8;
        attributes.push_back( WGL_COLOR_BITS_ARB );
        attributes.push_back( colorBits * 3 );
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
    HDC pfDC        = dc ? dc : screenDC;
    int pixelFormat = 0;

    while( true )
    {
        UINT nFormats = 0;
        if( !wglChoosePixelFormatARB( pfDC, &attributes[0], 0, 1,
                                      &pixelFormat, &nFormats ))
        {
            EQWARN << "wglChoosePixelFormat failed: " 
                << base::getErrorString( GetLastError( )) << std::endl;
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
 
    if( dc ) // set pixel format on given device context
    {
        PIXELFORMATDESCRIPTOR pfd = {0};
        pfd.nSize        = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion     = 1;

        DescribePixelFormat( dc, pixelFormat, sizeof(pfd), &pfd );
        if( !SetPixelFormat( dc, pixelFormat, &pfd ))
        {
            _window->setErrorMessage( "Can't set device pixel format: " + 
                base::getErrorString( GetLastError( )));
            return 0;
        }
    }
    
    return pixelFormat;
#else
    _window->setErrorMessage( "Client library compiled without WGL support" );
    return 0;
#endif
}

HGLRC WGLWindow::createWGLContext( HDC overrideDC )
{
#ifdef WGL
    HDC dc = overrideDC ? overrideDC : _wglDC;
    EQASSERT( dc );

    // create context
    HGLRC context = wglCreateContext( dc );
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
#else
    _window->setErrorMessage( "Client library compiled without WGL support" );
    return 0;
#endif
}

void WGLWindow::initEventHandler()
{
    EQASSERT( !_eventHandler );
    _eventHandler = new WGLEventHandler( this );
}

void WGLWindow::exitEventHandler()
{
    delete _eventHandler;
    _eventHandler = 0;
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
}
