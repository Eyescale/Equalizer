
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_WINDOWSYSTEM_H
#define EQ_WINDOWSYSTEM_H

#include <eq/base/base.h>

#include <string>

namespace eq
{
    /** The list of possible window systems. */
    enum WindowSystem
    {
        WINDOW_SYSTEM_NONE = 0, // must be first
        WINDOW_SYSTEM_AGL,  //!< AGL/Carbon
        WINDOW_SYSTEM_GLX,  //!< GLX/X11
        WINDOW_SYSTEM_WGL,  //!< WGL/Win32
        WINDOW_SYSTEM_ALL      // must be last
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                          const WindowSystem ws );
}

// window system and OS-dependent includes and definitions below.
#ifdef EQ_IGNORE_GLEW
#  define GLEWContext void
#  define WGLEWContext void
#else
#  include <GL/glew.h>
#  ifdef WGL
#    include <GL/wglew.h>
#  endif
#endif

#define GL_GLEXT_PROTOTYPES

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
typedef void* WGLEWContext;
#  define PFNWGLDELETEDCNVPROC void*
typedef bool  BOOL;
#  define WINAPI
#endif

// Error-check macros
#ifdef NDEBUG

#  define EQ_GL_ERROR( when ) 
#  define EQ_GL_CALL( code ) { code; }

#else // NDEBUG

namespace eq
{
/** Output an error OpenGL in a human-readable form to EQWARN */
EQ_EXPORT void debugGLError( const std::string& when, const GLenum error, 
                   const char* file, const int line );
}

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
#endif // EQ_WINDOWSYSTEM_H

