
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

#ifndef KEY_WOW64_64KEY
#  define KEY_WOW64_64KEY 0x0100
#endif

namespace eq
{

WGLPipe::WGLPipe( Pipe* parent )
    : SystemPipe( parent )
    , _wglewContext( new WGLEWContext )
    , _driverVersion( 0.f )
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
        EQASSERT( dc );

        pvp.x = 0;
        pvp.y = 0;
        pvp.w = GetDeviceCaps( dc, HORZRES );
        pvp.h = GetDeviceCaps( dc, VERTRES );

        DeleteDC( dc );
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
        EQWARN << getError() << ": " << base::sysError << std::endl;
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
        EQWARN << getError() << ": " << base::sysError << std::endl;
        return 0;
    }

    const HDC displayDC = CreateDC( "DISPLAY", devInfo.DeviceName, 0, 0 );
    if( !displayDC )
    {
        setError( ERROR_WGLPIPE_CREATEDC_FAILED );
        EQWARN << getError() << ": " << base::sysError << std::endl;
        return 0;
    }

    return displayDC;
}

bool WGLPipe::configInitGL()
{
    _configInitDriverVersion();
    return true;
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
        EQWARN << getError() << ": " << base::sysError << std::endl;
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
        EQWARN << getError() << ": " << base::sysError << std::endl;
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
        setError( ERROR_WGLPIPE_CREATEWINDOW_FAILED );
        EQWARN << getError() << ": " << base::sysError << std::endl;
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
        setError( ERROR_WGLPIPE_CHOOSEPF_FAILED );
        EQWARN << getError() << ": " << base::sysError << std::endl;
        DestroyWindow( hWnd );
        UnregisterClass( classStr.c_str(),  instance );
        return false;
    }
 
    if( !SetPixelFormat( dc, pf, &pfd ))
    {
        setError( ERROR_WGLPIPE_SETPF_FAILED );
        EQWARN << getError() << ": " << base::sysError << std::endl;
        ReleaseDC( hWnd, dc );
        DestroyWindow( hWnd );
        UnregisterClass( classStr.c_str(),  instance );
        return false;
    }

    // context
    HGLRC context = wglCreateContext( dc );
    if( !context )
    {
        setError( ERROR_WGLPIPE_CREATECONTEXT_FAILED );
        EQWARN << getError() << ": " << base::sysError << std::endl;
        ReleaseDC( hWnd, dc );
        DestroyWindow( hWnd );
        UnregisterClass( classStr.c_str(),  instance );
        return false;
    }

    HDC   oldDC      = wglGetCurrentDC();
    HGLRC oldContext = wglGetCurrentContext();

    wglMakeCurrent( dc, context );

    const GLenum result = wglewInit();
    bool success = true;
    if( result != GLEW_OK )
    {
        setError( ERROR_WGLPIPE_WGLEWINIT_FAILED );
        EQWARN << getError() << ": " << base::sysError << std::endl;
        success = false;
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

void WGLPipe::_configInitDriverVersion()
{
    // Query driver version of NVidia driver, used to implement affinity WAR
    const std::string glVendor = (const char*)glGetString( GL_VENDOR );
    if( glVendor.empty( )) // most likely no context - fail
    {
        EQWARN << "glGetString(GL_VENDOR) returned 0" << std::endl;
        return;
    }
    
    size_t nextPos = glVendor.find( "NVIDIA" );
    if( nextPos == std::string::npos ) // not NVidia, unused/unsupported
        return;
    HKEY key;
 
	unsigned char tempStr[512];
	unsigned long size = sizeof(tempStr);
    
    const std::string regPath("SOFTWARE\\NVIDIA Corporation\\Installer");
    const std::string regKey("Version");

    // try to read registry on x64
    if( RegOpenKeyEx( HKEY_LOCAL_MACHINE, regPath.c_str(), 0, 
                      KEY_READ | KEY_WOW64_64KEY, &key ) != ERROR_SUCCESS )
    {
        EQWARN << "RegOpenKeyEx(" << regPath << ") returned error" 
               << std::endl;
        return;
    }

    if( RegQueryValueEx(key,regKey.c_str(),0, 0,tempStr,&size)!=ERROR_SUCCESS )
    {
        // try to Read registre on 32 bits OS
        RegCloseKey(key);
        if( RegOpenKeyEx( HKEY_LOCAL_MACHINE, regPath.c_str(), 0, 
                          KEY_READ , &key ) != ERROR_SUCCESS )
        {
            EQWARN << "RegOpenKeyEx(" << regPath << ") returned error" 
                   << std::endl;
            return;
        }
        if( RegQueryValueEx(key, regKey.c_str(), 0, 
                            0, tempStr,&size)!= ERROR_SUCCESS )
        {
            EQWARN << "RegQueryValueEx(" << regKey << ") returned error" 
                   << std::endl;
            RegCloseKey(key);
            return;
        }
    } 

    RegCloseKey(key);
    
    std::string version;
    version.assign( tempStr, tempStr + size);
    nextPos = version.find_last_of( "." );

    version = version.substr( nextPos - 1 );
    
    _driverVersion = static_cast<float>( atof( version.c_str() ))* 100.f;
}

} //namespace eq

