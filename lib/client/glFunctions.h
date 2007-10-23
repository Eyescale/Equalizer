
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_GLFUNCTIONS_H
#define EQ_GLFUNCTIONS_H

#include <eq/base/base.h>             // EQ_EXPORT definition
#include <eq/base/debug.h>            // EQASSERT definition
#include <eq/base/nonCopyable.h>      // base class
#include <eq/client/windowSystem.h>   // OpenGL types

namespace eq
{
    class GLFunctionTable;

    /** Abstracts the access to OpenGL extension functions. */
    class EQ_EXPORT GLFunctions : public eqBase::NonCopyable
    {
    public:
        /** 
         * Create a new OpenGL function table.
         * 
         * The OpenGL context for which this function table is used has to be 
         * current.
         */
        GLFunctions( const WindowSystem windowSystem );
        ~GLFunctions();
        
        // buffer object functions
        bool hasGenBuffers() const
            { return ( _table.glGenBuffers != 0 ); }
        void genBuffers( GLsizei n, GLuint* buffers ) const;
        
        bool hasDeleteBuffers() const
            { return ( _table.glDeleteBuffers != 0 ); }
        void deleteBuffers( GLsizei n, const GLuint* buffers ) const;
        
        bool hasBindBuffer() const
           { return ( _table.glBindBuffer != 0 ); }
        void bindBuffer( GLenum target, GLuint buffer) const;
        
        bool hasBufferData() const
            { return ( _table.glBufferData != 0 ); }
        void bufferData( GLenum target, GLsizeiptr size, const GLvoid* data, 
                         GLenum usage) const;
        
        // program object functions
        bool hasCreateProgram() const
            { return ( _table.glCreateProgram != 0 ); }
        GLuint createProgram() const;
        
        bool hasDeleteProgram() const
            { return ( _table.glDeleteProgram != 0 ); }
        void deleteProgram ( GLuint program ) const;
        
        bool hasLinkProgram() const
            { return ( _table.glLinkProgram != 0 ); }
        void linkProgram( GLuint program ) const;
        
        bool hasUseProgram() const
            { return ( _table.glUseProgram != 0 ); }
        void useProgram( GLuint program ) const;
        
        bool hasGetProgramiv() const
            { return ( _table.glGetProgramiv != 0 ); }
        void getProgramiv( GLuint program, GLenum pname, GLint* params ) const;
        
        // shader object functions
        bool hasCreateShader() const
            { return ( _table.glCreateShader != 0 ); }
        GLuint createShader( GLenum type ) const;
        
        bool hasDeleteShader() const
            { return ( _table.glDeleteShader != 0 ); }
        void deleteShader( GLuint shader ) const;
        
        bool hasAttachShader() const
            { return ( _table.glAttachShader != 0 ); }
        void attachShader( GLuint program, GLuint shader ) const;
        
        bool hasDetachShader() const
            { return ( _table.glDetachShader != 0 ); }
        void detachShader( GLuint program, GLuint shader ) const;
        
        bool hasShaderSource() const
            { return ( _table.glShaderSource != 0 ); }
        void shaderSource( GLuint shader, GLsizei count, const GLchar** string, 
                           const GLint* length ) const;
        
        bool hasCompileShader() const
            { return ( _table.glCompileShader != 0 ); }
        void compileShader( GLuint shader ) const;
        
        bool hasGetShaderiv() const
            { return ( _table.glGetShaderiv != 0 ); }
        void getShaderiv( GLuint shader, GLenum pname, GLint* params ) const;

        // misc functions
        bool hasBlendFuncSeparate() const
            { return ( _table.glBlendFuncSeparate != 0 ); }
        void blendFuncSeparate( GLenum srcRGB,   GLenum dstRGB, 
                                GLenum srcAlpha, GLenum dstAlpha ) const;

        // utility functions
        static bool checkExtension( const char* extensionName );
        
    private:
        class GLFunctionTable
        {
        public:
            // buffer object functions
            PFNGLGENBUFFERSPROC     glGenBuffers;
            PFNGLDELETEBUFFERSPROC  glDeleteBuffers;
            PFNGLBINDBUFFERPROC     glBindBuffer;
            PFNGLBUFFERDATAPROC     glBufferData;
            
            // program object functions
            PFNGLCREATEPROGRAMPROC  glCreateProgram;
            PFNGLDELETEPROGRAMPROC  glDeleteProgram;
            PFNGLLINKPROGRAMPROC    glLinkProgram;
            PFNGLUSEPROGRAMPROC     glUseProgram;
            PFNGLGETPROGRAMIVPROC   glGetProgramiv;
            
            // shader object functions
            PFNGLCREATESHADERPROC   glCreateShader;
            PFNGLDELETESHADERPROC   glDeleteShader;
            PFNGLATTACHSHADERPROC   glAttachShader;
            PFNGLDETACHSHADERPROC   glDetachShader;
            PFNGLSHADERSOURCEPROC   glShaderSource;
            PFNGLCOMPILESHADERPROC  glCompileShader;
            PFNGLGETSHADERIVPROC    glGetShaderiv;

            // misc functions
            PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate;
        } _table;
    };
}

#endif // EQ_GLFUNCTIONS_H

