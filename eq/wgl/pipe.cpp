
/* Copyright (c) 2009-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2009, Maxim Makhinya
 *                    2010, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "pipe.h"

#include "eventHandler.h"

#include "../global.h"
#include "../pipe.h"

#include <boost/lexical_cast.hpp>

namespace eq
{
namespace wgl
{

Pipe::Pipe( eq::Pipe* parent )
        : SystemPipe( parent )
        , _wglewContext( new WGLEWContext )
{
}

Pipe::~Pipe( )
{
    delete _wglewContext;
}

//---------------------------------------------------------------------------
// WGL init
//---------------------------------------------------------------------------
bool Pipe::configInit()
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
        LBASSERT( found );

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
        pvp.x = 0;
        pvp.y = 0;

        HDC dc = createWGLDisplayDC();
        if( !dc )
            return false;

        uint32_t device = getPipe()->getDevice();
        if( device == LB_UNDEFINED_UINT32 )
            device = 0;

        DISPLAY_DEVICE devInfo;
        devInfo.cb = sizeof( devInfo );
        if( EnumDisplayDevices( 0, device, &devInfo, 0 ))
        {
            DEVMODE devMode;
            devMode.dmSize = sizeof( devMode );
            if( EnumDisplaySettings( devInfo.DeviceName,
                                        ENUM_CURRENT_SETTINGS, &devMode ))
            {
                pvp.x = devMode.dmPosition.x;
                pvp.y = devMode.dmPosition.y;
            }
        }

        pvp.w = GetDeviceCaps( dc, HORZRES );
        pvp.h = GetDeviceCaps( dc, VERTRES );
        DeleteDC( dc );
    }

    getPipe()->setPixelViewport( pvp );
    LBINFO << "Pipe pixel viewport " << pvp << std::endl;
    return true;
}


void Pipe::configExit()
{
    getPipe()->setPixelViewport( eq::PixelViewport( )); // invalidate
}


bool Pipe::createWGLAffinityDC( HDC& affinityDC )
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
        sendError( ERROR_WGL_CREATEAFFINITYDC_FAILED ) << lunchbox::sysError();
        return false;
    }

    return true;
}

HDC Pipe::createWGLDisplayDC()
{
    uint32_t device = getPipe()->getDevice();
    if( device == LB_UNDEFINED_UINT32 )
        device = 0;

    DISPLAY_DEVICE devInfo;
    devInfo.cb = sizeof( devInfo );

    if( !EnumDisplayDevices( 0, device, &devInfo, 0 ))
    {
        sendError( ERROR_WGLPIPE_ENUMDISPLAYS_FAILED ) << lunchbox::sysError();
        return 0;
    }

    const HDC displayDC = CreateDC( "DISPLAY", devInfo.DeviceName, 0, 0 );
    if( !displayDC )
    {
        sendError( ERROR_WGLPIPE_CREATEDC_FAILED ) << lunchbox::sysError();
        return 0;
    }

    return displayDC;
}

bool Pipe::_getGPUHandle( HGPUNV& handle )
{
    handle = 0;

    const uint32_t device = getPipe()->getDevice();
    if( device == LB_UNDEFINED_UINT32 )
        return true;

    if( !WGLEW_NV_gpu_affinity )
    {
        LBWARN <<"WGL_NV_gpu_affinity unsupported, ignoring pipe device setting"
               << std::endl;
        return true;
    }

    if( !wglEnumGpusNV( device, &handle ))
    {
        sendError( ERROR_WGLPIPE_ENUMGPUS_FAILED ) << lunchbox::sysError();
        return false;
    }

    return true;
}

bool Pipe::_configInitWGLEW()
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
        sendError( ERROR_WGLPIPE_REGISTERCLASS_FAILED ) << lunchbox::sysError();
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
        sendError( ERROR_SYSTEMPIPE_CREATEWINDOW_FAILED )
            << lunchbox::sysError();
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
        sendError( ERROR_SYSTEMPIPE_PIXELFORMAT_NOTFOUND )
            << lunchbox::sysError();
        DestroyWindow( hWnd );
        UnregisterClass( classStr.c_str(),  instance );
        return false;
    }

    if( !SetPixelFormat( dc, pf, &pfd ))
    {
        sendError( ERROR_WGLPIPE_SETPF_FAILED ) << lunchbox::sysError();
        ReleaseDC( hWnd, dc );
        DestroyWindow( hWnd );
        UnregisterClass( classStr.c_str(),  instance );
        return false;
    }

    // context
    HGLRC context = wglCreateContext( dc );
    if( !context )
    {
        sendError( ERROR_SYSTEMPIPE_CREATECONTEXT_FAILED )
            << lunchbox::sysError();
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
    if( success )
    {
        LBDEBUG << "Pipe WGLEW initialization successful" << std::endl;
        success = configInitGL();

        const char* glVersion = (const char*)glGetString( GL_VERSION );
        if( success && glVersion )
            _maxOpenGLVersion = static_cast<float>( atof( glVersion ));
    }
    else
        sendError( ERROR_WGLPIPE_WGLEWINIT_FAILED )
            << boost::lexical_cast< std::string >( result );

    wglDeleteContext( context );
    ReleaseDC( hWnd, dc );
    DestroyWindow( hWnd );
    UnregisterClass( classStr.c_str(),  instance );
    wglMakeCurrent( oldDC, oldContext );

    return success;
}

}
}
