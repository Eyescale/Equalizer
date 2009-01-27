
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
    /**
     * A facility class to managed OpenGL objects across shared contexts
     *
     * See also:
     * http://www.equalizergraphics.com/documents/design/objectManager.html
     * 
     * The semantics for each of the functions is:
     *
     * get - lookup existing object,
     * new - allocate new object,
     * obtain - get or new,
     * delete - delete.
     */
    template< typename T >
    class EQ_EXPORT ObjectManager : public base::NonCopyable
    {
    public:
        enum
        {
            INVALID = 0 //<! return value for failed operations.
        };

        ObjectManager( GLEWContext* glewContext )
            : _glewContext( glewContext ) 
            { EQASSERT( glewContext ); }

        virtual ~ObjectManager();

        void deleteAll();

        GLuint getList( const T& key );
        GLuint newList( const T& key, const GLsizei num = 1 );
        GLuint obtainList( const T& key, const GLsizei num = 1 );
        void   deleteList( const T& key );

        GLuint getTexture( const T& key );
        GLuint newTexture( const T& key );
        GLuint obtainTexture( const T& key );
        void   deleteTexture( const T& key );

        bool   supportsBuffers() const;
        GLuint getBuffer( const T& key );
        GLuint newBuffer( const T& key );
        GLuint obtainBuffer( const T& key );
        void   deleteBuffer( const T& key );

        bool   supportsPrograms() const;
        GLuint getProgram( const T& key );
        GLuint newProgram( const T& key );
        GLuint obtainProgram( const T& key );
        void   deleteProgram( const T& key );

        bool   supportsShaders() const;
        GLuint getShader( const T& key );
        GLuint newShader( const T& key, const GLenum type );
        GLuint obtainShader( const T& key, const GLenum type );
        void   deleteShader( const T& key );

        GLEWContext* glewGetContext() const { return _glewContext; }

    private:
        GLEWContext* _glewContext;

        struct Object
        {
            GLuint   id;
            GLuint   num;
        };

        typedef stde::hash_map< T, Object >     ObjectKeyHash;

        ObjectKeyHash _lists;
        ObjectKeyHash _textures;
        ObjectKeyHash _buffers;
        ObjectKeyHash _programs;
        ObjectKeyHash _shaders;
    };
}

#endif // EQ_OBJECTMANAGER_H

