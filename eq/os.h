
/* Copyright (c) 2006-2014, Stefan Eilemann <eile@equalizergraphics.com>
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
#include <eq/gl.h>
#include <lunchbox/os.h>
#include <lunchbox/types.h>

#include <string>

/**
 * @file eq/os.h
 *
 * Includes operating system headers for OpenGL and the used window system(s)
 * correctly. Include this file first if you have problems with OpenGL
 * definitions.
 */

/** @cond IGNORE */
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
#  define _WIN32_WINNT 0x501 // XP
#endif
#endif

#ifdef AGL
#  ifdef __LP64__ // AGL is not supported in 64 bit
#    undef AGL
#  else
#    define Cursor CGLCursor   // avoid name clash with X11 'Cursor'
#    include <ApplicationServices/ApplicationServices.h>
#    include <AGL/agl.h>
#    include <Carbon/Carbon.h>
#    define EQ_AGL_MENUBARHEIGHT 22
#    ifdef check // undo global namespace pollution (AssertMacros.h)
#      undef check
#    endif
#  endif
#endif

#ifdef WGL
#  include <wingdi.h>

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

#ifndef WGL_ARB_pbuffer
#   define WGL_ARB_pbuffer 1

#   define WGL_DRAW_TO_PBUFFER_ARB 0x202D
#   define WGL_MAX_PBUFFER_PIXELS_ARB 0x202E
#   define WGL_MAX_PBUFFER_WIDTH_ARB 0x202F
#   define WGL_MAX_PBUFFER_HEIGHT_ARB 0x2030
#   define WGL_PBUFFER_LARGEST_ARB 0x2033
#   define WGL_PBUFFER_WIDTH_ARB 0x2034
#   define WGL_PBUFFER_HEIGHT_ARB 0x2035
#   define WGL_PBUFFER_LOST_ARB 0x2036

DECLARE_HANDLE(HPBUFFERARB);

typedef HPBUFFERARB (WINAPI * PFNWGLCREATEPBUFFERARBPROC) (HDC hDC, int iPixelFormat, int iWidth, int iHeight, const int* piAttribList);
typedef BOOL (WINAPI * PFNWGLDESTROYPBUFFERARBPROC) (HPBUFFERARB hPbuffer);
typedef HDC (WINAPI * PFNWGLGETPBUFFERDCARBPROC) (HPBUFFERARB hPbuffer);
typedef BOOL (WINAPI * PFNWGLQUERYPBUFFERARBPROC) (HPBUFFERARB hPbuffer, int iAttribute, int* piValue);
typedef int (WINAPI * PFNWGLRELEASEPBUFFERDCARBPROC) (HPBUFFERARB hPbuffer, HDC hDC);

#   define wglCreatePbufferARB WGLEW_GET_FUN(__wglewCreatePbufferARB)
#   define wglDestroyPbufferARB WGLEW_GET_FUN(__wglewDestroyPbufferARB)
#   define wglGetPbufferDCARB WGLEW_GET_FUN(__wglewGetPbufferDCARB)
#   define wglQueryPbufferARB WGLEW_GET_FUN(__wglewQueryPbufferARB)
#   define wglReleasePbufferDCARB WGLEW_GET_FUN(__wglewReleasePbufferDCARB)

#   define WGLEW_ARB_pbuffer WGLEW_GET_VAR(__WGLEW_ARB_pbuffer)

#   endif /* WGL_ARB_pbuffer */
#endif

#ifndef WGL
typedef void* HDC;
typedef void* HWND;
typedef void* HPBUFFERARB;
typedef void* HGLRC;
#  define PFNWGLDELETEDCNVPROC void*
#  define WINAPI
#endif
/** @endcond */

#endif // EQ_OS_H
