
/* Copyright (c) 2011, Daniel Nachbaur <danielnachbaur@gmail.com> 
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

#include "module.h"

#include <gpusd/gpuInfo.h>
#include <sstream>
#include <GL/glew.h>
#include <GL/wglew.h>


namespace gpusd
{
namespace wgl
{
namespace
{

Module* instance = 0;

static WGLEWContext* wglewContext = new WGLEWContext;
WGLEWContext* wglewGetContext() { return wglewContext; }

std::string getErrorString()
{
    const DWORD error = GetLastError();
    char text[512] = "";
    FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, 0, error, 0, text, 511, 0 );
    const size_t length = strlen( text );
    if( length>2 && text[length-2] == '\r' )
        text[length-2] = '\0';
    return std::string( text );
}

bool _initWGLEW()
{
    //Create and make current a temporary GL context to initialize WGLEW

    // window class
    std::ostringstream className;
    className << "TMP" << (void*)&_initWGLEW;
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
        std::cerr << "Can't register temporary window class: " 
                  << getErrorString() << std::endl;
        return false;
    }

    // window
    DWORD windowStyleEx = WS_EX_APPWINDOW;
    DWORD windowStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
                        WS_OVERLAPPEDWINDOW;

    HWND hWnd = CreateWindowEx( windowStyleEx,
        wc.lpszClassName, "TMP",
        windowStyle, 0, 0, 1, 1,
        0, 0, // parent, menu
        instance, 0 );

    if( !hWnd )
    {
        std::cerr << "Can't create temporary window: "
                  << getErrorString() << std::endl;
        UnregisterClass( classStr.c_str(),  instance );
        return false;
    }

    HDC                   dc  = GetDC( hWnd );
    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize        = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;

    int pf = ChoosePixelFormat( dc, &pfd );
    if( pf == 0 )
    {
        std::cerr << "Can't find temporary pixel format: "
                  << getErrorString() << std::endl;
        DestroyWindow( hWnd );
        UnregisterClass( classStr.c_str(),  instance );
        return false;
    }

    if( !SetPixelFormat( dc, pf, &pfd ))
    {
        std::cerr << "Can't set pixel format: " 
                  << getErrorString() << std::endl;
        ReleaseDC( hWnd, dc );
        DestroyWindow( hWnd );
        UnregisterClass( classStr.c_str(),  instance );
        return false;
    }

    // context
    HGLRC context = wglCreateContext( dc );
    if( !context )
    {
        std::cerr << "Can't create temporary OpenGL context: " 
            << getErrorString() << std::endl;
        ReleaseDC( hWnd, dc );
        DestroyWindow( hWnd );
        UnregisterClass( classStr.c_str(),  instance );
        return false;
    }

    HDC   oldDC      = wglGetCurrentDC();
    HGLRC oldContext = wglGetCurrentContext();

    wglMakeCurrent( dc, context );

    const GLenum result = wglewInit();
    if( result != GLEW_OK )
    {
        std::cerr << "Pipe WGLEW initialization failed with error " 
                  << result << std::endl;
        return false;
    }

    wglDeleteContext( context );
    ReleaseDC( hWnd, dc );
    DestroyWindow( hWnd );
    UnregisterClass( classStr.c_str(),  instance );

    wglMakeCurrent( oldDC, oldContext );
    return true;
}

void _affinityDiscover( GPUInfos& result )
{
    for( UINT device = 0; true; ++device )
    {
        HGPUNV hGPU = 0;
        if( !wglEnumGpusNV( device, &hGPU ))
            break;

        GPU_DEVICE gpuDevice;
        gpuDevice.cb = sizeof( gpuDevice );
        if ( !wglEnumGpuDevicesNV( hGPU, 0, &gpuDevice ))
            break;

        GPUInfo info( "WGLn" );
        info.device = device;
        if( gpuDevice.Flags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP )
        {
            const RECT& rect = gpuDevice.rcVirtualScreen;
            info.pvp[0] = rect.left;
            info.pvp[1]= rect.top;
            info.pvp[2] = rect.right  - rect.left;
            info.pvp[3] = rect.bottom - rect.top; 
        }
        else
        {
            info.pvp[0] = 0;
            info.pvp[1] = 0;
            info.pvp[2] = 4096;
            info.pvp[3] = 4096;
        }
        result.push_back( info );
    }
}

void _associationDiscover( GPUInfos& result )
{
    const UINT maxGPUs = 32;
    UINT gpuIDs[maxGPUs];
    UINT numGPUs = wglGetGPUIDsAMD( maxGPUs, gpuIDs );

    for( UINT device = 0; device < numGPUs; ++device )
    {
        GPUInfo info( "WGLa" );
        info.device = gpuIDs[device];
        info.pvp[0] = 0;
        info.pvp[1] = 0;
        info.pvp[2] = 4096;
        info.pvp[3] = 4096;
        result.push_back( info );
    }
}

void _nativeDiscover( GPUInfos& result )
{
    for( UINT device = 0; true; ++device )
    {
        DISPLAY_DEVICE devInfo;
        devInfo.cb = sizeof( devInfo );

        if( !EnumDisplayDevices( 0, device, &devInfo, 0 ))
            break;

        const HDC displayDC = CreateDC( "DISPLAY", devInfo.DeviceName, 0, 0 );
        if( !displayDC )
            break;

        GPUInfo info( "WGL" );
        info.device = device;
        info.pvp[0] = 0;
        info.pvp[1] = 0;
        if( displayDC )
        {
            info.pvp[2] = GetDeviceCaps( displayDC, HORZRES );
            info.pvp[3] = GetDeviceCaps( displayDC, VERTRES );
            DeleteDC( displayDC );
        }
        else
        {
            info.pvp[2] = 2048;
            info.pvp[3] = 2048;
        }
        result.push_back( info );
    }
}

}

void Module::use()
{
    if( !instance )
        instance = new Module;
}

GPUInfos Module::discoverGPUs_() const
{
    GPUInfos result;

    if ( !_initWGLEW( ))
        return result;

    if ( WGLEW_NV_gpu_affinity )
        _affinityDiscover( result );
    else if ( WGLEW_AMD_gpu_association )
        _associationDiscover( result );
    else
        _nativeDiscover( result );

    return result;
}

}
}
