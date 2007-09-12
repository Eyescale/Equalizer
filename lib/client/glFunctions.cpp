
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "glFunctions.h"

#include <eq/base/debug.h>

namespace eq
{

#ifndef WGL
#  define wglGetProcAddress( foo ) 0
#endif
#ifndef GLX
#  define glXGetProcAddress( foo ) 0
#endif
#ifdef AGL
#  define AGLFUNC( name ) &name
#else
#  define AGLFUNC( name ) 0;
#endif

#define LOOKUP( windowSystem, name, type )                              \
    switch( windowSystem )                                              \
    {                                                                   \
        case WINDOW_SYSTEM_WGL:                                         \
            _table.name = (type)( wglGetProcAddress( #name ));          \
            break;                                                      \
        case WINDOW_SYSTEM_GLX:                                         \
            _table.name = (type)( glXGetProcAddress( (const GLubyte*)#name )); \
            break;                                                      \
        case WINDOW_SYSTEM_AGL:                                         \
            _table.name = AGLFUNC( name );                              \
            break;                                                      \
        default:                                                        \
            EQUNIMPLEMENTED;                                            \
    }                                                           


GLFunctions::GLFunctions( const WindowSystem windowSystem )
{
    LOOKUP( windowSystem, glGenBuffers,    PFNGLGENBUFFERSPROC );
    LOOKUP( windowSystem, glDeleteBuffers, PFNGLDELETEBUFFERSPROC );
}

GLFunctions::~GLFunctions()
{
}

}
