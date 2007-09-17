
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

#define QUERY( windowSystem, name, func, type )                         \
    switch( windowSystem )                                              \
    {                                                                   \
        case WINDOW_SYSTEM_WGL:                                         \
            _table.name = (type)( wglGetProcAddress( #func ));          \
            break;                                                      \
        case WINDOW_SYSTEM_GLX:                                         \
            _table.name = (type)( glXGetProcAddress( (const GLubyte*)#func )); \
            break;                                                      \
        case WINDOW_SYSTEM_AGL:                                         \
            _table.name = AGLFUNC( func );                              \
            break;                                                      \
        default:                                                        \
            EQUNIMPLEMENTED;                                            \
    }                                                           

#define LOOKUP( windowSystem, name, type )      \
    QUERY( windowSystem, name, name, type );

#define ARB_BACKUP( windowSystem, name, type )          \
    if( !_table.name )                                  \
        QUERY( windowSystem, name, name ## ARB, type );

GLFunctions::GLFunctions( const WindowSystem windowSystem )
{
    LOOKUP( windowSystem, glGenBuffers,    PFNGLGENBUFFERSPROC );
    ARB_BACKUP( windowSystem, glGenBuffers,    PFNGLGENBUFFERSPROC );
    LOOKUP( windowSystem, glDeleteBuffers, PFNGLDELETEBUFFERSPROC );
    ARB_BACKUP( windowSystem, glDeleteBuffers, PFNGLDELETEBUFFERSPROC );
}

GLFunctions::~GLFunctions()
{
}

}
