
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com>
                     , Makhinya Maxim
   All rights reserved. */

#include "wglPipe.h"

#include "global.h"
#include "wglEventHandler.h"

namespace eq
{

WGLPipe::WGLPipe( Pipe* parent )
    : OSPipe( parent )
{
}

WGLPipe::~WGLPipe( )
{
}

using namespace std;

//---------------------------------------------------------------------------
// WGL init
//---------------------------------------------------------------------------
bool WGLPipe::configInit()
{
#ifdef WGL
    _configInitWGLEW();

    PixelViewport pvp = _pipe->getPixelViewport();
    if( pvp.isValid( ))
        return true;

    // setup pvp
    // ...using gpu affinity API
    HGPUNV hGPU = 0;
    if( !_getGPUHandle( hGPU ))
        return false;

    if( hGPU != 0 )
    {
        GPU_DEVICE gpuDevice;
        gpuDevice.cb = sizeof( gpuDevice );
        const bool found = wglEnumGpuDevicesNV( hGPU, 0, &gpuDevice );
        EQASSERT( found );

        if( gpuDevice.Flags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP )
        {
            const RECT& rect = gpuDevice.rcVirtualScreen;
            pvp.x = rect.left;
            pvp.y = rect.top;
            pvp.w = rect.right  - rect.left;
            pvp.h = rect.bottom - rect.top; 
        }
        else
        {
            pvp.x = 0;
            pvp.y = 0;
            pvp.w = 4096;
            pvp.h = 4096;
        }
    }
    else // ... using Win32 API
    {
        HDC dc = GetDC( 0 );
        EQASSERT( dc );

        pvp.x = 0;
        pvp.y = 0;
        pvp.w = GetDeviceCaps( dc, HORZRES );
        pvp.h = GetDeviceCaps( dc, VERTRES );
        _pipe->setPixelViewport( pvp );

        ReleaseDC( 0, dc );
    }

    _pipe->setPixelViewport( pvp );
    EQINFO << "Pipe pixel viewport " << pvp << endl;
    return true;
#else
    setErrorMessage( "Client library compiled without WGL support" );
    return false;
#endif
}


void WGLPipe::configExit()
{
#ifdef WGL
    _pipe->setPixelViewport( eq::PixelViewport( )); // invalidate
#endif
}


bool WGLPipe::createAffinityDC( HDC& affinityDC )
{
#ifdef WGL
    affinityDC = 0;

    HGPUNV hGPU[2] = { 0 };
    if( !_getGPUHandle( hGPU[0] ))
        return false;

    affinityDC = wglCreateAffinityDCNV( hGPU );
    if( !affinityDC )
    {
        setErrorMessage( "Can't create affinity DC: " +
                         base::getErrorString( GetLastError( )));
        return false;
    }

    return true;
#else
    return false;
#endif
}

bool WGLPipe::_getGPUHandle( HGPUNV& handle )
{
    handle = 0;

    const uint32_t device = _pipe->getDevice();
    if( device == EQ_UNDEFINED_UINT32 )
        return true;

    if( !WGLEW_NV_gpu_affinity )
    {
        EQWARN <<"WGL_NV_gpu_affinity unsupported, ignoring pipe device setting"
               << endl;
        return true;
    }

    HGPUNV hGPU[2] = { 0 };
    hGPU[1] = 0;
    if( !wglEnumGpusNV( device, hGPU ))
    {
        stringstream error;
        error << "Can't enumerate GPU #" << device;
        setErrorMessage( error.str( ));
        return false;
    }

    handle = hGPU[0];
    return true;
}


void WGLPipe::_configInitWGLEW()
{
#ifdef WGL
    //----- Create and make current a temporary GL context to initialize WGLEW

    // window class
    ostringstream className;
    className << "TMP" << (void*)this;
    const string& classStr = className.str();
                                  
    HINSTANCE instance = GetModuleHandle( 0 );
    WNDCLASS  wc       = { 0 };
    wc.lpfnWndProc   = WGLEventHandler::wndProc;    
    wc.hInstance     = instance; 
    wc.hIcon         = LoadIcon( 0, IDI_WINLOGO );
    wc.hCursor       = LoadCursor( 0, IDC_ARROW );
    wc.lpszClassName = classStr.c_str();       

    if( !RegisterClass( &wc ))
    {
        EQWARN << "Can't register temporary window class: " 
               << base::getErrorString( GetLastError( )) << endl;
        return;
    }

    // window
    DWORD windowStyleEx = WS_EX_APPWINDOW;
    DWORD windowStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW;

    HWND hWnd = CreateWindowEx( windowStyleEx,
                                wc.lpszClassName, "TMP",
                                windowStyle, 0, 0, 1, 1,
                                0, 0, // parent, menu
                                instance, 0 );

    if( !hWnd )
    {
        EQWARN << "Can't create temporary window: "
               << base::getErrorString( GetLastError( )) << endl;
        UnregisterClass( classStr.c_str(),  instance );
        return;
    }

    HDC                   dc  = GetDC( hWnd );
    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize        = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_DRAW_TO_WINDOW |
                       PFD_SUPPORT_OPENGL;

    int pf = ChoosePixelFormat( dc, &pfd );
    if( pf == 0 )
    {
        EQWARN << "Can't find temporary pixel format: "
               << base::getErrorString( GetLastError( )) << endl;
        DestroyWindow( hWnd );
        UnregisterClass( classStr.c_str(),  instance );
        return;
    }
 
    if( !SetPixelFormat( dc, pf, &pfd ))
    {
        EQWARN << "Can't set pixel format: " 
               << base::getErrorString( GetLastError( )) << endl;
        ReleaseDC( hWnd, dc );
        DestroyWindow( hWnd );
        UnregisterClass( classStr.c_str(),  instance );
        return;
    }

    // context
    HGLRC context = wglCreateContext( dc );
    if( !context )
    {
         EQWARN << "Can't create temporary OpenGL context: " 
                << base::getErrorString( GetLastError( )) << endl;
        ReleaseDC( hWnd, dc );
        DestroyWindow( hWnd );
        UnregisterClass( classStr.c_str(),  instance );
        return;
    }

    HDC   oldDC      = wglGetCurrentDC();
    HGLRC oldContext = wglGetCurrentContext();

    wglMakeCurrent( dc, context );

    const GLenum result = wglewInit();
    if( result != GLEW_OK )
        EQWARN << "Pipe WGLEW initialization failed with error " << result 
               << endl;
    else
        EQINFO << "Pipe WGLEW initialization successful" << endl;

    wglDeleteContext( context );
    ReleaseDC( hWnd, dc );
    DestroyWindow( hWnd );
    UnregisterClass( classStr.c_str(),  instance );

    wglMakeCurrent( oldDC, oldContext );
#endif
}


} //namespace eq

