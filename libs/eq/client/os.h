
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

#include <eq/client/api.h>
#include <eq/client/gl.h>
#include <lunchbox/os.h>
#include <lunchbox/types.h>

#include <string>

/**
 * @file eq/client/os.h
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
#  ifdef check // undo global namespace pollution (AssertMacros.h)
#    undef check
#  endif
#endif

#ifdef WGL
#  include <wingdi.h>

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

