
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
        
        static bool checkExtension( const char* extensionName );
        
    private:
        class GLFunctionTable
        {
        public:
            PFNGLGENBUFFERSPROC     glGenBuffers;
            PFNGLDELETEBUFFERSPROC  glDeleteBuffers;
            PFNGLBINDBUFFERPROC     glBindBuffer;
            PFNGLBUFFERDATAPROC     glBufferData;
        } _table;
    };
}

#endif // EQ_OBJECTMANAGER_H

