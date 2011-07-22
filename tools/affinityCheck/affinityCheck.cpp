
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef GLEW_MX
# define GLEW_MX
#endif

#include <eq/GL/glew.h>
#include <eq/GL/wglew.h>
#include <iostream>
#include <sstream>
#include <cassert>
#include <cstdlib>

namespace
{
    static WGLEWContext* wglewContext = new WGLEWContext;
    WGLEWContext* wglewGetContext() { return wglewContext; }

    inline std::string getErrorString()
    {
        const DWORD error = GetLastError();
        char text[512] = "";
        FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, 0, error, 0, text, 511, 0 );
        const size_t length = strlen( text );
        if( length>2 && text[length-2] == '\r' )
            text[length-2] = '\0';
        return std::string( text );
    }

    static bool initWGLEW()
    {
        //Create and make current a temporary GL context to initialize WGLEW

        // window class
        std::ostringstream className;
        className << "TMP" << (void*)&initWGLEW;
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
}

int main( const int argc, char** argv )
{
    if( !initWGLEW( ))
        std::cerr << "WGL extension query failed" << std::endl;
    else if( !WGLEW_NV_gpu_affinity )
        std::cerr << "WGL_NV_gpu_affinity unsupported" << std::endl;
    else for( UINT gpu = 0; true; ++gpu )
    {
        HGPUNV hGPU = 0;
        if( !wglEnumGpusNV( gpu, &hGPU ))
            break;

        GPU_DEVICE gpuDevice;
        gpuDevice.cb = sizeof( gpuDevice );
        const bool found = wglEnumGpuDevicesNV( hGPU, 0, &gpuDevice );
        assert( found );

        std::cout << "GPU " << gpu << ": " << gpuDevice.DeviceString;
 
        if( gpuDevice.Flags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP )
        {
            const RECT& rect = gpuDevice.rcVirtualScreen;
            std::cout << " used on [" << rect.left << ' ' << rect.top << ' '
                      << rect.right  - rect.left << ' ' 
                      << rect.bottom - rect.top << ']';
        }
        else
            std::cout << " offline";

        std::cout << std::endl;
    }

    std::cout << "Press Enter to exit..." << std::endl;
    
    char foo[256];
    std::cin.getline( foo, 256 );
    return EXIT_SUCCESS;
}
