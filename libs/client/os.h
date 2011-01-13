
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_OS_H
#define EQ_OS_H

#include <eq/api.h>

#include <string>

/**
 * @file eq/os.h
 * 
 * Includes operating system headers for OpenGL and the used window system(s)
 * correctly. Include this file first if you have problems with OpenGL
 * definitions.
 *
 * Define EQ_IGNORE_GLEW before including any Equalizer header if you have
 * trouble with your system-installed OpenGL header and do not need GLEW.
 */

/** @cond IGNORE */
#ifdef EQ_IGNORE_GLEW
struct GLEWContextStruct;
struct WGLEWContextStruct;
struct GLXEWContextStruct;
typedef struct GLEWContextStruct GLEWContext;
typedef struct WGLEWContextStruct WGLEWContext;
typedef struct GLXEWContextStruct GLXEWContext;
#else
#  include <GL/glew.h>
#  ifdef GLX
#    include <GL/glxew.h>
#  endif
#  ifdef WGL
#    include <GL/wglew.h>
#  endif
#endif

#ifdef GLX
#  include <X11/Xlib.h>
#  include <GL/glx.h>
#  ifndef GLX_SAMPLE_BUFFERS
#    define GLX_SAMPLE_BUFFERS 100000
#  endif
#  ifndef GLX_SAMPLES
#    define GLX_SAMPLES 100001
#  endif
#endif

#ifdef _WIN32
#ifndef _WIN32_WINNT
#  ifdef EQ_USE_MAGELLAN
#    define _WIN32_WINNT 0x501 // XP
#  else
#    define _WIN32_WINNT 0x500 // 2000
#  endif
#endif
#endif

#ifdef AGL
#  if defined(__i386__) // WAR compile error
#    undef Status 
#  endif
#  define Cursor CGLCursor   // avoid name clash with X11 'Cursor'
#  include <ApplicationServices/ApplicationServices.h>
#  include <AGL/agl.h>
#  include <Carbon/Carbon.h>
#  define EQ_AGL_MENUBARHEIGHT 22
#endif

#ifdef WGL
#  include <wingdi.h>
#  include <GL/gl.h>

#  ifndef WGL_ARB_pbuffer
typedef void* HPBUFFERARB;
#  endif

#  ifndef WGL_NV_gpu_affinity
#    define WGL_NV_gpu_affinity 1
DECLARE_HANDLE(HGPUNV);
typedef struct _GPU_DEVICE {
    DWORD  cb;
    CHAR   DeviceName[32];
    CHAR   DeviceString[128];
    DWORD  Flags;
    RECT   rcVirtualScreen;
} GPU_DEVICE, *PGPU_DEVICE;

#    ifdef WGL_WGLEXT_PROTOTYPES
extern BOOL WINAPI wglEnumGpusNV (UINT iIndex, HGPUNV *hGpu);
extern BOOL WINAPI wglEnumGpuDevicesNV (HGPUNV hGpu, UINT iIndex, PGPU_DEVICE pGpuDevice);
extern HDC WINAPI wglCreateAffinityDCNV (const HGPUNV *pGpuList);
extern BOOL WINAPI wglEnumGpusFromAffinityDCNV (HDC hAffinityDC, UINT iIndex, HGPUNV *hGpu);
extern BOOL WINAPI wglDeleteDCNV (HDC hAffinityDC);
#    else
typedef BOOL (WINAPI * PFNWGLENUMGPUSNVPROC) (UINT iIndex, HGPUNV *hGpu);
typedef BOOL (WINAPI * PFNWGLENUMGPUDEVICESNVPROC) (HGPUNV hGpu, UINT iIndex, PGPU_DEVICE pGpuDevice);
typedef HDC (WINAPI * PFNWGLCREATEAFFINITYDCNVPROC) (const HGPUNV *pGpuList);
typedef BOOL (WINAPI * PFNWGLENUMGPUSFROMAFFINITYDCNVPROC) (HDC hAffinityDC, UINT iIndex, HGPUNV *hGpu);
typedef BOOL (WINAPI * PFNWGLDELETEDCNVPROC) (HDC hAffinityDC);
#    endif // WGL_WGLEXT_PROTOTYPES
#  endif // WGL_NV_gpu_affinity
#endif

#ifndef GLX
typedef void Display;
typedef void XErrorEvent;
typedef unsigned long XID;
typedef void* GLXContext;
typedef void  XVisualInfo;
#endif

#ifndef AGL
typedef int32_t CGDirectDisplayID;
typedef void*   WindowRef;
typedef void*   EventHandlerRef;
typedef void*   AGLContext;
typedef void*   AGLPixelFormat;
typedef void*   AGLPbuffer;
#endif

#ifndef WGL
typedef void* HDC;
typedef void* HWND;
typedef void* HPBUFFERARB;
typedef void* HGLRC;
#  define PFNWGLDELETEDCNVPROC void*
#  define WINAPI
#endif

#ifndef GL_TEXTURE_RECTANGLE_ARB
#  define GL_TEXTURE_RECTANGLE_ARB 0x84F5
#endif
/** @endcond */

// Error-check macros
namespace eq
{
/** Output an error OpenGL in a human-readable form to EQWARN */
EQ_API void debugGLError( const std::string& when, const GLenum error, 
                             const char* file, const int line );
}

#ifdef NDEBUG
#  define EQ_GL_ERROR( when ) 
#  define EQ_GL_CALL( code ) { code; }
#else // NDEBUG
#  define EQ_GL_ERROR( when )                                           \
    {                                                                   \
        const GLenum eqGlError = glGetError();                          \
        if( eqGlError )                                                 \
            eq::debugGLError( when, eqGlError, __FILE__, __LINE__ );    \
    }

#  define EQ_GL_CALL( code )                              \
    {                                                     \
        EQ_GL_ERROR( std::string( "before " ) + #code );  \
        code;                                             \
        EQ_GL_ERROR( std::string( "after " ) + #code );   \
    }
#endif // NDEBUG

namespace eq
{
#ifdef GLX
/** 
 * Set the current X display connection.
 *
 * This function stores a per-thread display connection, similar to the current
 * WGL/AGL context. It is used by some  eq and eq::util classes to retrieve the
 * display without having to know the eq::Pipe. The GLXPipe sets it
 * automatically. Applications using the GLX window system with a custom
 * SystemPipe implementation have to set it using this function.
 *
 * @param display the current display connection to use.
 */
void XSetCurrentDisplay( Display* display );

/** @return the current display connection for the calling thread. */
Display* XGetCurrentDisplay();
#endif
}

#endif // EQ_OS_H

