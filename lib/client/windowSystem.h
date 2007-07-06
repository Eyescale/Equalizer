
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WINDOWSYSTEM_H
#define EQ_WINDOWSYSTEM_H

#include <eq/base/base.h>


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
#  define Cursor CGLCursor // avoid name clash with X11 'Cursor'
#  include <ApplicationServices/ApplicationServices.h>
#  include <AGL/agl.h>
#  undef Cursor
#endif

#ifdef WGL
#  define GL_GLEXT_PROTOTYPES
#  include <wingdi.h>
#  include <gl/GL.h>
#  include <eq/client/glext.h>
#  include <eq/client/wglext.h>
#endif

#ifndef GL_TEXTURE_3D
#  define 	GL_TEXTURE_3D   0x806F
#endif

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

namespace eq
{
    enum WindowSystem
    {
        WINDOW_SYSTEM_NONE = 0, // must be first
        WINDOW_SYSTEM_GLX,
        WINDOW_SYSTEM_AGL,
        WINDOW_SYSTEM_WGL,
        WINDOW_SYSTEM_ALL      // must be last
    };
}

#endif // EQ_WINDOWSYSTEM_H

