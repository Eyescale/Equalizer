
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WINDOWSYSTEM_H
#define EQ_WINDOWSYSTEM_H

#include <eq/base/base.h>

#include <string>

namespace eq
{
    enum WindowSystem
    {
        WINDOW_SYSTEM_NONE = 0, // must be first
        WINDOW_SYSTEM_AGL,
        WINDOW_SYSTEM_GLX,
        WINDOW_SYSTEM_WGL,
        WINDOW_SYSTEM_ALL      // must be last
    };

    EQ_EXPORT std::ostream& operator << ( std::ostream& os, 
                                          const WindowSystem ws );
}

// window system and OS-dependent includes and definitions below.
#define GL_GLEXT_PROTOTYPES

#ifdef GLX
#  ifdef Darwin
#    include <OpenGL/gl.h>
#  else
#    include <GL/gl.h>
#  endif
#  include <X11/Xlib.h>
#  include <GL/glx.h>
#endif

#ifdef AGL
#  if defined(__i386__) // WAR compile error
#    undef Status 
#  endif
#  define Cursor CGLCursor   // avoid name clash with X11 'Cursor'
#  include <ApplicationServices/ApplicationServices.h>
#  include <AGL/agl.h>
#  include <Carbon/Carbon.h>
#endif

#ifdef WGL
#  include <wingdi.h>
#  include <gl/GL.h>
#  include <eq/client/glext.h>
#  include <eq/client/wglext.h>
#endif

//----- Missing definitions due to old (w)glext.h
#ifndef GL_TEXTURE_3D
#  define 	GL_TEXTURE_3D   0x806F
#endif

#ifdef WGL
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
#endif // WGL
//-----

#ifndef GLX
typedef void Display;
typedef void XErrorEvent;
typedef unsigned long XID;
typedef void* GLXContext;
#endif

#ifndef AGL
typedef int32_t CGDirectDisplayID;
typedef void*   WindowRef;
typedef void*   EventHandlerRef;
typedef void*   AGLContext;
#endif

#ifndef WGL
typedef void* HDC;
typedef void* HWND;
typedef void* HGLRC;
#endif

// Error-check macros
#ifdef NDEBUG
#  define EQ_GL_ERROR( when ) 
#  define EQ_GL_CALL( code ) { code; }

#else // NDEBUG

namespace eq
{
EQ_EXPORT void debugGLError( const std::string& when, const GLenum error, 
                   const char* file, const int line );
}

#  define EQ_GL_ERROR( when )                                     \
    {                                                             \
        const GLenum error = glGetError();                        \
        if( error )                                               \
            eq::debugGLError( when, error, __FILE__, __LINE__ );  \
    }

#  define EQ_GL_CALL( code )                              \
    {                                                     \
        EQ_GL_ERROR( std::string( "before " ) + #code );  \
        code;                                             \
        EQ_GL_ERROR( std::string( "after " ) + #code );   \
    }

#endif // NDEBUG


// Definitions missing when the OS has an old glext.h

#ifndef GL_DEPTH_STENCIL_NV
#  define GL_DEPTH_STENCIL_NV               0x84F9
#endif
#ifndef GL_UNSIGNED_INT_24_8_NV
#  define GL_UNSIGNED_INT_24_8_NV           0x84FA
#endif

#ifndef GL_VERSION_2_0
typedef char GLchar;
#endif

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif

#ifndef GL_VERTEX_SHADER
#  define GL_VERTEX_SHADER                  0x8B31
#endif
#ifndef GL_FRAGMENT_SHADER
#  define GL_FRAGMENT_SHADER                0x8B30
#endif
#ifndef GL_COMPILE_STATUS
#  define GL_COMPILE_STATUS                 0x8B81
#endif
#ifndef GL_LINK_STATUS
#  define GL_LINK_STATUS                    0x8B82
#endif

#ifndef PFNGLDELETEBUFFERSPROC
typedef void (APIENTRYP PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint *buffers);
typedef void (APIENTRYP PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRYP PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRYP PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
#endif

#ifndef PFNGLATTACHSHADERPROC
typedef void (APIENTRYP PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (APIENTRYP PFNGLCOMPILESHADERPROC) (GLuint shader);
typedef GLuint (APIENTRYP PFNGLCREATEPROGRAMPROC) (void);
typedef GLuint (APIENTRYP PFNGLCREATESHADERPROC) (GLenum type);
typedef void (APIENTRYP PFNGLDELETEPROGRAMPROC) (GLuint program);
typedef void (APIENTRYP PFNGLDELETESHADERPROC) (GLuint shader);
typedef void (APIENTRYP PFNGLDETACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (APIENTRYP PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void (APIENTRYP PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar* *string, const GLint *length);
typedef void (APIENTRYP PFNGLUSEPROGRAMPROC) (GLuint program);
#endif

#ifndef PFNGLBLENDFUNCSEPARATEPROC
typedef void (APIENTRYP PFNGLBLENDFUNCSEPARATEPROC) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
#endif

#endif // EQ_WINDOWSYSTEM_H

