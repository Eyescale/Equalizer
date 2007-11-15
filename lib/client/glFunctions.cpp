
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
#ifdef AGL
#   define AGLFUNC( name ) &name
#else
#   define AGLFUNC( name ) 0
#endif

// glXGetProcAddress does not seem to work under Darwin/X11
#ifdef GLX
#   define GLXFUNC( name ) &name
#else
#   define GLXFUNC( foo ) 0
#endif // GLX

#define QUERY( name, func, type )                                       \
    {                                                                   \
        switch( windowSystem )                                          \
        {                                                               \
            case WINDOW_SYSTEM_WGL:                                     \
                _table.name = (type)( wglGetProcAddress( #func ));      \
                break;                                                  \
            case WINDOW_SYSTEM_GLX:                                     \
                _table.name = (type)( GLXFUNC( func ) );                \
                break;                                                  \
            case WINDOW_SYSTEM_AGL:                                     \
                _table.name = (type)( AGLFUNC( func ) );                \
                break;                                                  \
            default:                                                    \
                EQUNIMPLEMENTED;                                        \
        }                                                               \
    }

#define LOOKUP( name, type )      \
    QUERY( name, name, type );

#define BACKUP( name, func, type )                            \
    {                                                         \
        if( !_table.name )                                    \
            QUERY( name, func, type );                        \
    }


GLFunctions::GLFunctions( const WindowSystem windowSystem )
{
    // buffer object functions
    LOOKUP( glGenBuffers,    PFNGLGENBUFFERSPROC );
    LOOKUP( glDeleteBuffers, PFNGLDELETEBUFFERSPROC );
    LOOKUP( glBindBuffer,    PFNGLBINDBUFFERPROC );
    LOOKUP( glBufferData,    PFNGLBUFFERDATAPROC );
    
    // program object functions
    LOOKUP( glCreateProgram, PFNGLCREATEPROGRAMPROC );
    LOOKUP( glDeleteProgram, PFNGLDELETEPROGRAMPROC );
    LOOKUP( glLinkProgram,   PFNGLLINKPROGRAMPROC );
    LOOKUP( glUseProgram,    PFNGLUSEPROGRAMPROC );
    LOOKUP( glGetProgramiv,  PFNGLGETPROGRAMIVPROC );
    
    // shader object functions
    LOOKUP( glCreateShader,  PFNGLCREATESHADERPROC );
    LOOKUP( glDeleteShader,  PFNGLDELETESHADERPROC );
    LOOKUP( glAttachShader,  PFNGLATTACHSHADERPROC );
    LOOKUP( glDetachShader,  PFNGLDETACHSHADERPROC );
    LOOKUP( glShaderSource,  PFNGLSHADERSOURCEPROC );
    LOOKUP( glCompileShader, PFNGLCOMPILESHADERPROC );
    LOOKUP( glGetShaderiv,   PFNGLGETSHADERIVPROC );

    // misc functions
    LOOKUP( glBlendFuncSeparate, PFNGLBLENDFUNCSEPARATEPROC );

    if( checkExtension( "GL_ARB_vertex_buffer_object" ))
    {
        // buffer object functions
        BACKUP( glGenBuffers,    glGenBuffersARB,    PFNGLGENBUFFERSPROC );
        BACKUP( glDeleteBuffers, glDeleteBuffersARB, PFNGLDELETEBUFFERSPROC );
        BACKUP( glBindBuffer,    glBindBufferARB,    PFNGLBINDBUFFERPROC );
        BACKUP( glBufferData,    glBufferDataARB,    PFNGLBUFFERDATAPROC );
    }
    
    if( checkExtension( "GL_ARB_shader_objects" ))
    {
        // program object functions
        BACKUP( glCreateProgram, glCreateProgramObjectARB,
                PFNGLCREATEPROGRAMPROC );
        BACKUP( glDeleteProgram, glDeleteObjectARB,    PFNGLDELETEPROGRAMPROC );
        BACKUP( glLinkProgram,   glLinkProgramARB,       PFNGLLINKPROGRAMPROC );
        BACKUP( glUseProgram,    glUseProgramObjectARB,  PFNGLUSEPROGRAMPROC );
        BACKUP( glGetProgramiv,glGetObjectParameterivARB,PFNGLGETPROGRAMIVPROC);
        
        // shader object functions
        BACKUP( glCreateShader, glCreateShaderObjectARB, PFNGLCREATESHADERPROC);
        BACKUP( glDeleteShader,  glDeleteObjectARB,    PFNGLDELETESHADERPROC );
        BACKUP( glAttachShader,  glAttachObjectARB,    PFNGLATTACHSHADERPROC );
        BACKUP( glDetachShader,  glDetachObjectARB,    PFNGLDETACHSHADERPROC );
        BACKUP( glShaderSource,  glShaderSourceARB,    PFNGLSHADERSOURCEPROC );
        BACKUP( glCompileShader, glCompileShaderARB,   PFNGLCOMPILESHADERPROC );
        BACKUP( glGetShaderiv, glGetObjectParameterivARB, PFNGLGETSHADERIVPROC);
    }

    // misc functions
    if( checkExtension( "GL_EXT_blend_func_separate" ))
        BACKUP( glBlendFuncSeparate, glBlendFuncSeparateEXT, 
                PFNGLBLENDFUNCSEPARATEPROC );

}

GLFunctions::~GLFunctions()
{
}


// buffer object functions
void GLFunctions::genBuffers( GLsizei n, GLuint* buffers ) const
{
    EQASSERT( hasGenBuffers() );
    _table.glGenBuffers( n, buffers );
}
        
void GLFunctions::deleteBuffers( GLsizei n, const GLuint* buffers ) const
{
    EQASSERT( hasDeleteBuffers() );
    _table.glDeleteBuffers( n, buffers );
}
        
void GLFunctions::bindBuffer( GLenum target, GLuint buffer) const
{
    EQASSERT( hasBindBuffer() );
    _table.glBindBuffer( target, buffer );
}
        
void GLFunctions::bufferData( GLenum target, GLsizeiptr size, 
                              const GLvoid* data, GLenum usage) const
{
    EQASSERT( hasBufferData() );
    _table.glBufferData( target, size, data, usage );
}


// program object functions
GLuint GLFunctions::createProgram() const
{
    EQASSERT( hasCreateProgram() );
    return _table.glCreateProgram();
}

void GLFunctions::deleteProgram ( GLuint program ) const
{
    EQASSERT( hasDeleteProgram() );
    _table.glDeleteProgram( program );
}

void GLFunctions::linkProgram( GLuint program ) const
{
    EQASSERT( hasLinkProgram() );
    _table.glLinkProgram( program );
}

void GLFunctions::useProgram( GLuint program ) const
{
    EQASSERT( hasUseProgram() );
    _table.glUseProgram( program );
}

void GLFunctions::getProgramiv( GLuint program, GLenum pname, 
                                GLint* params ) const
{
    EQASSERT( hasGetProgramiv() );
    _table.glGetProgramiv( program, pname, params );
}


// shader object functions
GLuint GLFunctions::createShader( GLenum type ) const
{
    EQASSERT( hasCreateShader() );
    return _table.glCreateShader( type );
}

void GLFunctions::deleteShader( GLuint shader ) const
{
    EQASSERT( hasDeleteShader() );
    _table.glDeleteShader( shader );
}

void GLFunctions::attachShader( GLuint program, GLuint shader ) const
{
    EQASSERT( hasAttachShader() );
    _table.glAttachShader( program, shader );
}

void GLFunctions::detachShader( GLuint program, GLuint shader ) const
{
    EQASSERT( hasDetachShader() );
    _table.glDetachShader( program, shader );
}

void GLFunctions::shaderSource( GLuint shader, GLsizei count, 
                                const GLchar** string, 
                                const GLint* length ) const
{
    EQASSERT( hasShaderSource() );
    _table.glShaderSource( shader, count, string, length );
}

void GLFunctions::compileShader( GLuint shader ) const
{
    EQASSERT( hasCompileShader() );
    _table.glCompileShader( shader );
}

void GLFunctions::getShaderiv( GLuint shader, GLenum pname, 
                               GLint* params ) const
{
    EQASSERT( hasGetShaderiv() );
    _table.glGetShaderiv( shader, pname, params );
}

// misc functions
void GLFunctions::blendFuncSeparate( GLenum srcRGB,   GLenum dstRGB, 
                                       GLenum srcAlpha, GLenum dstAlpha ) const
{
    EQASSERT( hasBlendFuncSeparate() );
    _table.glBlendFuncSeparate( srcRGB, dstRGB, srcAlpha, dstAlpha );
}


// helper function to check for specific OpenGL extensions by name
bool GLFunctions::checkExtension( const char* extensionName )
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
