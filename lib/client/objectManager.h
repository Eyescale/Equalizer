
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_OBJECTMANAGER_H
#define EQ_OBJECTMANAGER_H

#include <eq/base/base.h>             // EQ_EXPORT definition
#include <eq/base/debug.h>            // EQASSERT definition
#include <eq/base/hash.h>             // member
#include <eq/base/nonCopyable.h>      // base class
#include <eq/client/windowSystem.h>   // OpenGL types

namespace eq
{
    class GLFunctions;

    /**
     * A facility class to managed OpenGL objects across shared contexts
     *
     * See also:
     * http://www.equalizergraphics.com/documents/design/objectManager.html
     * 
     * The semantics for the functions is:
     * get - lookup existing object,
     * new - allocate new object,
     * obtain - get or new,
     * release - decrease reference count,
     * delete - forcibly delete.
     */
    template< typename T >
    class EQ_EXPORT ObjectManager : public eqBase::NonCopyable
    {
    public:
        enum
        {
            FAILED = 0xffffffffu //<! return value for failed operations.
        };

        ObjectManager( const GLFunctions* glFunctions )
            : _glFunctions( glFunctions ) 
            { EQASSERT( glFunctions ); }

        virtual ~ObjectManager();

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

        bool   supportsBuffers() const;
        GLuint getBuffer( const T& key );
        GLuint newBuffer( const T& key );
        GLuint obtainBuffer( const T& key );
        void   releaseBuffer( const T& key );
        void   releaseBuffer( const GLuint id );
        void   deleteBuffer( const T& key );
        void   deleteBuffer( const GLuint id );

    private:
        const GLFunctions* _glFunctions;

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

        ObjectIDHash  _buffersID;
        ObjectKeyHash _buffersKey;
    };
}

#endif // EQ_OBJECTMANAGER_H

