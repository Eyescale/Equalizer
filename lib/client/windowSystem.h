
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WINDOWSYSTEM_H
#define EQ_WINDOWSYSTEM_H

#include <eq/base/base.h>


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
}

// window system and OS-dependent includes and definitions below.

#ifdef GLX
#  define GL_GLEXT_PROTOTYPES
#  ifdef Darwin
#    include <OpenGL/gl.h>
#  else
#    include <GL/gl.h>
#  endif
#  include <X11/Xlib.h>
#  include <GL/glx.h>
#  ifdef Darwin
#    if defined(__i386__) // WAR compile error
#      undef Status 
#    endif 
#    define Cursor CGLCursor   // avoid name clash with X11 'Cursor'
#    include <Carbon/Carbon.h> // for SetSystemUIMode / fullscreen setup
#    undef Cursor
#  endif
#endif

#ifdef AGL
#  define GL_GLEXT_PROTOTYPES
#  if defined(__i386__) // WAR compile error
#    undef Status 
#  endif
#  ifdef Cursor 
#    undef Cursor // avoid name clash with X11 'Cursor'
#  endif
#  include <ApplicationServices/ApplicationServices.h>
#  include <AGL/agl.h>
#endif

#ifdef WGL
#  define GL_GLEXT_PROTOTYPES
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
#  ifndef Darwin
typedef int32_t CGDirectDisplayID;
typedef void*   WindowRef;
typedef void*   EventHandlerRef;
#  endif
typedef void*   AGLContext;
#endif

#ifndef WGL
typedef void* HDC;
typedef void* HWND;
typedef void* HGLRC;
#endif

#ifndef GL_DEPTH_STENCIL_NV
#  define GL_DEPTH_STENCIL_NV               0x84F9
#endif
#ifndef GL_UNSIGNED_INT_24_8_NV
#  define GL_UNSIGNED_INT_24_8_NV           0x84FA
#endif

#endif // EQ_WINDOWSYSTEM_H

