
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "GLFunctions.h"

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
#  define AGL( name ) &name
#else
#  define AGL( name ) 0;
#endif

#define LOOKUP( windowSystem, name, type )                      \
    switch( windowSystem )                                      \
    {                                                           \
        case WINDOW_SYSTEM_WGL:                                 \
            _table.name = (type)( wglGetProcAddress( #name ));  \
            break;                                              \
        case WINDOW_SYSTEM_GLX:                                 \
            _table.name = (type)( glXGetProcAddress( #name ));  \
            break;                                              \
        case WINDOW_SYSTEM_AGL:                                 \
            _table.name = AGL( name );                          \
            break;                                              \
        default:                                                \
            EQUNIMPLEMENTED;                                    \
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
