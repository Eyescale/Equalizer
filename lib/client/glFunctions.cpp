
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "glFunctions.h"
#include <eq/base/debug.h>
#include <string.h>

namespace eq
{

#ifndef WGL
#   define wglGetProcAddress( foo ) 0
#endif
#ifndef GLX
#   define glXGetProcAddress( foo ) 0
#endif
#ifdef AGL
#   define AGLFUNC( name ) &name
#else
#   define AGLFUNC( name ) 0;
#endif

// glXGetProcAddress does not seem to work under Darwin/X11
#ifdef GLX
#   ifdef Darwin
#       define GLXFUNC( name ) &name
#   else
#       define GLXFUNC( name ) glXGetProcAddress( (const GLubyte*) #name )
#   endif // Darwin
#endif // GLX

#define QUERY( windowSystem, name, func, type )                         \
    switch( windowSystem )                                              \
    {                                                                   \
        case WINDOW_SYSTEM_WGL:                                         \
            _table.name = (type)( wglGetProcAddress( #func ));          \
            break;                                                      \
        case WINDOW_SYSTEM_GLX:                                         \
            _table.name = (type)( GLXFUNC( func ) );                    \
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
    LOOKUP(     windowSystem, glGenBuffers,    PFNGLGENBUFFERSPROC );
    ARB_BACKUP( windowSystem, glGenBuffers,    PFNGLGENBUFFERSPROC );
    LOOKUP(     windowSystem, glDeleteBuffers, PFNGLDELETEBUFFERSPROC );
    ARB_BACKUP( windowSystem, glDeleteBuffers, PFNGLDELETEBUFFERSPROC );
    LOOKUP(     windowSystem, glBindBuffer,    PFNGLBINDBUFFERPROC );
    ARB_BACKUP( windowSystem, glBindBuffer,    PFNGLBINDBUFFERPROC );
    LOOKUP(     windowSystem, glBufferData,    PFNGLBUFFERDATAPROC );
    ARB_BACKUP( windowSystem, glBufferData,    PFNGLBUFFERDATAPROC );
}

GLFunctions::~GLFunctions()
{
}

// helper function to check for specific OpenGL extensions by name
bool GLFunctions::checkExtension( const char* extensionName ) const
{
    // get the list of supported extensions
    const char* extensionList = 
        reinterpret_cast<const char*>( glGetString( GL_EXTENSIONS ) );
    
    if( !extensionName || !extensionList )
        return false;
    
    while( *extensionList )
    {
        // find the length of the first extension substring
        size_t firstExtensionLength = strcspn( extensionList, " " );
        
        if( strlen( extensionName ) == firstExtensionLength &&
            strncmp( extensionName, extensionList, firstExtensionLength ) == 0 )
        {
            return true;
        }
        
        // move to the next substring
        extensionList += firstExtensionLength + 1;
    }
    
    return false;
}

}
