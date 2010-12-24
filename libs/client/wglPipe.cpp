
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

#include "wglPipe.h"

#include "global.h"
#include "pipe.h"
#include "wglEventHandler.h"

namespace eq
{

WGLPipe::WGLPipe( Pipe* parent )
        : SystemPipe( parent )
        , _wglewContext( new WGLEWContext )
{
}

WGLPipe::~WGLPipe( )
{
    delete _wglewContext;
}

//---------------------------------------------------------------------------
// WGL init
//---------------------------------------------------------------------------
bool WGLPipe::configInit()
{
    if ( !_configInitWGLEW() )
        return false;

    PixelViewport pvp = getPipe()->getPixelViewport();
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

        pvp.x = 0;
        pvp.y = 0;
        if( dc )
        {
            pvp.w = GetDeviceCaps( dc, HORZRES );
            pvp.h = GetDeviceCaps( dc, VERTRES );
            DeleteDC( dc );
        }
        else
        {
            EQWARN << "Can't create display dc query pipe resolution: "
                   << co::base::sysError << std::endl;
            pvp.w = 2048;
            pvp.h = 2048;
        }
    }

    getPipe()->setPixelViewport( pvp );
    EQINFO << "Pipe pixel viewport " << pvp << std::endl;
    return true;
}


void WGLPipe::configExit()
{
    getPipe()->setPixelViewport( eq::PixelViewport( )); // invalidate
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
        setError( ERROR_WGL_CREATEAFFINITYDC_FAILED );
        EQWARN << getError() << ": " << co::base::sysError << std::endl;
        return false;
    }

    return true;
}

HDC WGLPipe::createWGLDisplayDC()
{
    uint32_t device = getPipe()->getDevice();
    if( device == EQ_UNDEFINED_UINT32 )
        device = 0;

    DISPLAY_DEVICE devInfo;
    devInfo.cb = sizeof( devInfo );

    if( !EnumDisplayDevices( 0, device, &devInfo, 0 ))
    {
        setError( ERROR_WGLPIPE_ENUMDISPLAYS_FAILED );
        EQWARN << getError() << ": " << co::base::sysError << std::endl;
        return 0;
    }

    const HDC displayDC = CreateDC( "DISPLAY", devInfo.DeviceName, 0, 0 );
    if( !displayDC )
    {
        setError( ERROR_WGLPIPE_CREATEDC_FAILED );
        EQWARN << getError() << ": " << co::base::sysError << std::endl;
        return 0;
    }

    return displayDC;
}

bool WGLPipe::_getGPUHandle( HGPUNV& handle )
{
    handle = 0;

    const uint32_t device = getPipe()->getDevice();
    if( device == EQ_UNDEFINED_UINT32 )
        return true;

    if( !WGLEW_NV_gpu_affinity )
    {
        EQWARN <<"WGL_NV_gpu_affinity unsupported, ignoring pipe device setting"
               << std::endl;
        return true;
    }

    if( !wglEnumGpusNV( device, &handle ))
    {
        setError( ERROR_WGLPIPE_ENUMGPUS_FAILED );
        EQWARN << getError() << ": " << co::base::sysError << std::endl;
        return false;
    }

    return true;
}

bool WGLPipe::_configInitWGLEW()
{
    //----- Create and make current a temporary GL context to initialize WGLEW

    // window class
    std::ostringstream className;
    className << "TMP" << (void*)this;
    const std::string& classStr = className.str();
                                  
    HINSTANCE instance = GetModuleHandle( 0 );
    WNDCLASS  wc       = { 0 };
    wc.lpfnWndProc   = DefWindowProc;
    wc.hInstance     = instance; 
    wc.hIcon         = LoadIcon( 0, IDI_WINLOGO );
    wc.hCursor       = LoadCursor( 0, IDC_ARROW );
    wc.lpszClassName = classStr.c_str();       

    if( !RegisterClass( &wc ))
    {
        setError( ERROR_WGLPIPE_REGISTERCLASS_FAILED );
        EQWARN << getError() << ": " << co::base::sysError << std::endl;
        return false;
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
        setError( ERROR_SYSTEMPIPE_CREATEWINDOW_FAILED );
        EQWARN << getError() << ": " << co::base::sysError << std::endl;
        UnregisterClass( classStr.c_str(),  instance );
        return false;
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
        setError( ERROR_SYSTEMPIPE_PIXELFORMAT_NOTFOUND );
        EQWARN << getError() << ": " << co::base::sysError << std::endl;
        DestroyWindow( hWnd );
        UnregisterClass( classStr.c_str(),  instance );
        return false;
    }
 
    if( !SetPixelFormat( dc, pf, &pfd ))
    {
        setError( ERROR_WGLPIPE_SETPF_FAILED );
        EQWARN << getError() << ": " << co::base::sysError << std::endl;
        ReleaseDC( hWnd, dc );
        DestroyWindow( hWnd );
        UnregisterClass( classStr.c_str(),  instance );
        return false;
    }

    // context
    HGLRC context = wglCreateContext( dc );
    if( !context )
    {
        setError( ERROR_SYSTEMPIPE_CREATECONTEXT_FAILED );
        EQWARN << getError() << ": " << co::base::sysError << std::endl;
        ReleaseDC( hWnd, dc );
        DestroyWindow( hWnd );
        UnregisterClass( classStr.c_str(),  instance );
        return false;
    }

    HDC   oldDC      = wglGetCurrentDC();
    HGLRC oldContext = wglGetCurrentContext();

    wglMakeCurrent( dc, context );

    const GLenum result = wglewInit();
    bool success = result == GLEW_OK;
    if( !success )
    {
        setError( ERROR_WGLPIPE_WGLEWINIT_FAILED );
        EQWARN << getError() << ": " << result << std::endl;
    }
    else
    {
        EQINFO << "Pipe WGLEW initialization successful" << std::endl;
        success = configInitGL();
    }

    wglDeleteContext( context );
    ReleaseDC( hWnd, dc );
    DestroyWindow( hWnd );
    UnregisterClass( classStr.c_str(),  instance );
    wglMakeCurrent( oldDC, oldContext );

    return success;
}

} //namespace eq

