
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

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <GL/gl.h>

namespace gpusd
{
namespace wgl
{
namespace
{

Module* instance = 0;

DECLARE_HANDLE(HGPUNV);
typedef struct _GPU_DEVICE {
  DWORD cb; 
  CHAR DeviceName[32]; 
  CHAR DeviceString[128]; 
  DWORD Flags; 
  RECT rcVirtualScreen; 
} GPU_DEVICE, *PGPU_DEVICE;

typedef const char* (WINAPI * PFNWGLGETEXTENSIONSSTRINGARBPROC) (HDC hdc);
typedef BOOL (WINAPI * PFNWGLENUMGPUSNVPROC) (UINT iGpuIndex, HGPUNV *phGpu);
typedef BOOL (WINAPI * PFNWGLENUMGPUDEVICESNVPROC) (HGPUNV hGpu,
                                                    UINT iDeviceIndex,
                                                    PGPU_DEVICE lpGpuDevice);
typedef UINT (WINAPI * PFNWGLGETGPUIDSAMDPROC) (UINT maxCount, UINT* ids);

PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB_ = 0;
PFNWGLENUMGPUSNVPROC wglEnumGpusNV_ = 0;
PFNWGLENUMGPUDEVICESNVPROC wglEnumGpuDevicesNV_ = 0;
PFNWGLGETGPUIDSAMDPROC wglGetGPUIDsAMD_ = 0;

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

bool _initGLFuncs()
{
    // Create a temporary GL context to initialize GL function pointers
    // window class
    std::ostringstream className;
    className << "TMP" << (void*)&className;
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
        std::cerr << "Can't create temporary window: " << getErrorString()
                  << std::endl;
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

    wglGetExtensionsStringARB_ = (PFNWGLGETEXTENSIONSSTRINGARBPROC)
        wglGetProcAddress( "wglGetExtensionsStringARB" );
    const char* ext = wglGetExtensionsStringARB_ ?
      wglGetExtensionsStringARB_( dc ) : 0;
    if( ext )
    {
        const std::string extensions( ext );
        if( extensions.find( "WGL_NV_gpu_affinity" ) != std::string::npos )
        {
            wglEnumGpusNV_ = (PFNWGLENUMGPUSNVPROC)
                wglGetProcAddress( "wglEnumGpusNV" );
            wglEnumGpuDevicesNV_ = (PFNWGLENUMGPUDEVICESNVPROC)
                wglGetProcAddress( "wglEnumGpuDevicesNV" );
        }
        if( extensions.find( "WGL_AMD_gpu_association" ) != std::string::npos )
            wglGetGPUIDsAMD_ = (PFNWGLGETGPUIDSAMDPROC)
                wglGetProcAddress( "wglGetGPUIDsAMD" );
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
        if( !wglEnumGpusNV_( device, &hGPU ))
            break;

        GPU_DEVICE gpuDevice;
        gpuDevice.cb = sizeof( gpuDevice );
        if ( !wglEnumGpuDevicesNV_( hGPU, 0, &gpuDevice ))
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
            info.pvp[2] = 8192;
            info.pvp[3] = 8192;
        }
        result.push_back( info );
    }
}

void _associationDiscover( GPUInfos& result )
{
    const UINT maxGPUs = 32;
    UINT gpuIDs[maxGPUs];
    UINT numGPUs = wglGetGPUIDsAMD_( maxGPUs, gpuIDs );

    for( UINT device = 0; device < numGPUs; ++device )
    {
        GPUInfo info( "WGLa" );
        info.device = gpuIDs[device];
        info.pvp[0] = 0;
        info.pvp[1] = 0;
        info.pvp[2] = 8192;
        info.pvp[3] = 8192;
        result.push_back( info );
    }
}

// callback function called by EnumDisplayMonitors for each enabled monitor
BOOL CALLBACK EnumDispProc( HMONITOR hMon, HDC dcMon, RECT* pRcMon,
                            LPARAM lParam )
{
    GPUInfos* result = reinterpret_cast<GPUInfos*>( lParam );
    
    MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    if( !GetMonitorInfo( hMon, &mi ))
        return TRUE;    // continue enumeration
    
    GPUInfo info( "WGL" );
    info.device = unsigned( result->size( ));
    info.pvp[0] = mi.rcMonitor.left;
    info.pvp[1] = mi.rcMonitor.top;
    info.pvp[2] = mi.rcMonitor.left + mi.rcMonitor.right;
    info.pvp[3] = mi.rcMonitor.top + mi.rcMonitor.bottom;    
    result->push_back( info );

    return TRUE;
}

void _displayDiscover( GPUInfos& result )
{
    EnumDisplayMonitors( 0, 0, EnumDispProc,
                         reinterpret_cast<LPARAM>( &result ));
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

    if( !_initGLFuncs( ))
        return result;

    // also finds attached displays
    if( wglEnumGpusNV_ )
    {
        _affinityDiscover( result );
        return result;
    }
    
    _displayDiscover( result );
    
    // does not find GPUs with attached display
    if( wglGetGPUIDsAMD_ )
        _associationDiscover( result );

    return result;
}

}
}
