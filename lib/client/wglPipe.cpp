
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "wglPipe.h"

#include "global.h"
#include "pipe.h"
#include "wglEventHandler.h"

namespace eq
{

WGLPipe::WGLPipe( Pipe* parent )
    : OSPipe( parent )
    , _wglewContext( new WGLEWContext )
{
}

WGLPipe::~WGLPipe( )
{
    delete _wglewContext;
    _wglewContext = 0;
}

using namespace std;

//---------------------------------------------------------------------------
// WGL init
//---------------------------------------------------------------------------
bool WGLPipe::configInit()
{
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
        HDC dc = createWGLDisplayDC();
        EQASSERT( dc );

        pvp.x = 0;
        pvp.y = 0;
        pvp.w = GetDeviceCaps( dc, HORZRES );
        pvp.h = GetDeviceCaps( dc, VERTRES );

        DeleteDC( dc );
    }

    _pipe->setPixelViewport( pvp );
    EQINFO << "Pipe pixel viewport " << pvp << endl;
    return true;
}


void WGLPipe::configExit()
{
    _pipe->setPixelViewport( eq::PixelViewport( )); // invalidate
}


bool WGLPipe::createWGLAffinityDC( HDC& affinityDC )
{
    affinityDC = 0;

    HGPUNV hGPU[2] = { 0 };
    if( !_getGPUHandle( hGPU[0] ))
        return false;

    if( hGPU[0] == 0 ) // no affinity DC needed
        return true;

    affinityDC = wglCreateAffinityDCNV( hGPU );
    if( !affinityDC )
    {
        std::stringstream error;
        error << "Can't create affinity DC: " << base::sysError;
        setErrorMessage( error.str( ));
        return false;
    }

    return true;
}

HDC WGLPipe::createWGLDisplayDC()
{
    uint32_t device = _pipe->getDevice();
    if( device == EQ_UNDEFINED_UINT32 )
        device = 0;

    DISPLAY_DEVICE devInfo;
    devInfo.cb = sizeof( devInfo );

    if( !EnumDisplayDevices( 0, device, &devInfo, 0 ))
    {
        std::ostringstream error;
        error << "Can't enumerate display devices: " << base::sysError;
        _pipe->setErrorMessage( error.str( ));
        return 0;
    }

    const HDC displayDC = CreateDC( "DISPLAY", devInfo.DeviceName, 0, 0 );
    if( !displayDC )
    {
        std::ostringstream error;
        error << "Can't create device context: " << base::sysError;
        _pipe->setErrorMessage( error.str( ));
        return 0;
    }

    return displayDC;
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

    if( !wglEnumGpusNV( device, &handle ))
    {
        stringstream error;
        error << "Can't enumerate GPU #" << device;
        setErrorMessage( error.str( ));
        return false;
    }

    return true;
}


void WGLPipe::_configInitWGLEW()
{
    //----- Create and make current a temporary GL context to initialize WGLEW

    // window class
    ostringstream className;
    className << "TMP" << (void*)this;
    const string& classStr = className.str();
                                  
    HINSTANCE instance = GetModuleHandle( 0 );
    WNDCLASS  wc       = { 0 };
    wc.lpfnWndProc   = DefWindowProc;
    wc.hInstance     = instance; 
    wc.hIcon         = LoadIcon( 0, IDI_WINLOGO );
    wc.hCursor       = LoadCursor( 0, IDC_ARROW );
    wc.lpszClassName = classStr.c_str();       

    if( !RegisterClass( &wc ))
    {
        EQWARN << "Can't register temporary window class: " 
               << base::sysError << endl;
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
               << base::sysError << endl;
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
               << base::sysError << endl;
        DestroyWindow( hWnd );
        UnregisterClass( classStr.c_str(),  instance );
        return;
    }
 
    if( !SetPixelFormat( dc, pf, &pfd ))
    {
        EQWARN << "Can't set pixel format: " 
               << base::sysError << endl;
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
                << base::sysError << endl;
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
}


} //namespace eq

