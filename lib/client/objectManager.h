
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_OBJECTMANAGER_H
#define EQ_OBJECTMANAGER_H

#include <eq/base/base.h>             // EQ_EXPORT definition
#include <eq/base/hash.h>             // member
#include <eq/base/nonCopyable.h>      // base class
#include <eq/client/windowSystem.h>   // OpenGL types

namespace eq
{
    template< typename T >
    class EQ_EXPORT ObjectManager : public eqBase::NonCopyable
    {
    public:
        ObjectManager();
        virtual ~ObjectManager();

        void init();
        void deleteAll();

        GLuint getList( const T& key );
        GLuint newList( const T& key );
        GLuint obtainList( const T& key );
        void   releaseList( const T& key );
        void   releaseList( const GLuint id );
        void   deleteList( const T& key );
        void   deleteList( const GLuint id );

        GLuint getTexture( const T& key );
        GLuint newTexture( const T& key );
        GLuint obtainTexture( const T& key );
        void   releaseTexture( const T& key );
        void   releaseTexture( const GLuint id );
        void   deleteTexture( const T& key );
        void   deleteTexture( const GLuint id );

#ifdef GL_ARB_vertex_buffer_object
        GLuint getBuffer( const T& key );
        GLuint newBuffer( const T& key );
        GLuint obtainBuffer( const T& key );
        void   releaseBuffer( const T& key );
        void   releaseBuffer( const GLuint id );
        void   deleteBuffer( const T& key );
        void   deleteBuffer( const GLuint id );
#endif

    private:
        struct Object
        {
            GLuint   id;
            T        key;
            uint32_t refCount;
        };

        typedef stde::hash_map< GLuint, Object > ObjectIDHash;
        typedef stde::hash_map< T, Object* >     ObjectKeyHash;

        ObjectIDHash  _listsID;
        ObjectKeyHash _listsKey;

        ObjectIDHash  _texturesID;
        ObjectKeyHash _texturesKey;

#ifdef GL_ARB_vertex_buffer_object
        ObjectIDHash  _buffersID;
        ObjectKeyHash _buffersKey;
        bool          _buffersSupported;

#   ifdef WIN32
        PFNGLGENBUFFERSPROC    _glGenBuffersARB;
        PFNGLDELETEBUFFERSPROC _glDeleteBuffersARB;
#   else
        void ( * _glGenBuffersARB )( GLsizei, GLuint* );
        void ( * _glDeleteBuffersARB )( GLsizei, const GLuint* );
#   endif // WIN32

#endif // GL_ARB_vertex_buffer_object
    };
}

#endif // EQ_OBJECTMANAGER_H

