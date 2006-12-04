
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_WINDOWSYSTEM_H
#define EQ_WINDOWSYSTEM_H

#ifdef GLX
#ifdef WIN32
#  include "win32_x11.h"
#  include "win32_glx.h"
#else
#  include <X11/Xlib.h>
#  include <GL/glx.h>
#endif
#endif
#ifdef CGL
#  if defined(__i386__) // WAR compile error
#    undef Status 
#  endif 
#  define Cursor CGLCursor // avoid name clash with X11 'Cursor'
#  include <ApplicationServices/ApplicationServices.h>
#  include <OpenGL/OpenGL.h>
#  undef Cursor
#endif

#ifndef GLX
typedef void Display;
typedef void XErrorEvent;
#endif
#ifndef CGL
typedef int32_t CGDirectDisplayID;
#endif


namespace eq
{
    enum WindowSystem
    {
        WINDOW_SYSTEM_NONE = 0, // must be first
        WINDOW_SYSTEM_GLX,
        WINDOW_SYSTEM_CGL,
        WINDOW_SYSTEM_ALL      // must be last
    };
}

#endif // EQ_WINDOWSYSTEM_H

