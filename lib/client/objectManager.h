
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef EQ_OBJECTMANAGER_H
#define EQ_OBJECTMANAGER_H

#include <eq/client/windowSystem.h>   // OpenGL types
#include <eq/base/base.h>             // EQ_EXPORT definition
#include <eq/base/debug.h>            // EQASSERT definition
#include <eq/base/hash.h>             // member
#include <eq/base/nonCopyable.h>      // base class
#include <eq/base/referenced.h>       // base class

namespace eq
{
    class FrameBufferObject;
    class Texture;

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
    class ObjectManager : public base::NonCopyable
    {
    public:
        enum
        {
            INVALID = 0 //<! return value for failed operations.
        };

        /** Construct a new object manager. */
        EQ_EXPORT ObjectManager( GLEWContext* const glewContext );

        /** Construct a new object manager sharing data with another manager. */
        EQ_EXPORT ObjectManager( GLEWContext* const glewContext, ObjectManager* shared );

        EQ_EXPORT virtual ~ObjectManager();

        /** @return the number of object managers currently sharing the data. */
        int getSharedUsage() const { return _data->getRefCount(); }

        EQ_EXPORT void deleteAll();

        EQ_EXPORT GLuint getList( const T& key );
        EQ_EXPORT GLuint newList( const T& key, const GLsizei num = 1 );
        EQ_EXPORT GLuint obtainList( const T& key, const GLsizei num = 1 );
        EQ_EXPORT void   deleteList( const T& key );

        EQ_EXPORT GLuint getTexture( const T& key );
        EQ_EXPORT GLuint newTexture( const T& key );
        EQ_EXPORT GLuint obtainTexture( const T& key );
        EQ_EXPORT void   deleteTexture( const T& key );

        EQ_EXPORT bool   supportsBuffers() const;
        EQ_EXPORT GLuint getBuffer( const T& key );
        EQ_EXPORT GLuint newBuffer( const T& key );
        EQ_EXPORT GLuint obtainBuffer( const T& key );
        EQ_EXPORT void   deleteBuffer( const T& key );

        EQ_EXPORT bool   supportsPrograms() const;
        EQ_EXPORT GLuint getProgram( const T& key );
        EQ_EXPORT GLuint newProgram( const T& key );
        EQ_EXPORT GLuint obtainProgram( const T& key );
        EQ_EXPORT void   deleteProgram( const T& key );

        EQ_EXPORT bool   supportsShaders() const;
        EQ_EXPORT GLuint getShader( const T& key );
        EQ_EXPORT GLuint newShader( const T& key, const GLenum type );
        EQ_EXPORT GLuint obtainShader( const T& key, const GLenum type );
        EQ_EXPORT void   deleteShader( const T& key );

        EQ_EXPORT bool     supportsEqTexture() const;
        EQ_EXPORT Texture* getEqTexture( const T& key );
        EQ_EXPORT Texture* newEqTexture( const T& key );
        EQ_EXPORT Texture* obtainEqTexture( const T& key );
        EQ_EXPORT void     deleteEqTexture( const T& key );

        EQ_EXPORT bool               supportsEqFrameBufferObject() const;
        EQ_EXPORT FrameBufferObject* getEqFrameBufferObject( const T& key );
        EQ_EXPORT FrameBufferObject* newEqFrameBufferObject( const T& key );
        EQ_EXPORT FrameBufferObject* obtainEqFrameBufferObject( const T& key );
        EQ_EXPORT void               deleteEqFrameBufferObject( const T& key );

        const GLEWContext* glewGetContext() const { return _glewContext; }
        GLEWContext* glewGetContext()             { return _glewContext; }

    private:
        GLEWContext* const _glewContext;

        struct Object
        {
            GLuint   id;
            GLuint   num;
        };

        typedef stde::hash_map< T, Object >     ObjectHash;
        typedef stde::hash_map< T, Texture* >   TextureHash;
        typedef stde::hash_map< T, FrameBufferObject* > FrameBufferObjectHash;

        struct SharedData : public base::Referenced
        {
            virtual ~SharedData();

            ObjectHash lists;
            ObjectHash textures;
            ObjectHash buffers;
            ObjectHash programs;
            ObjectHash shaders;
            TextureHash eqTextures;
            FrameBufferObjectHash eqFrameBufferObjects;

            union // placeholder for binary-compatible changes
            {
                char dummy[64];
            };
        };

        typedef base::RefPtr< SharedData > SharedDataPtr;
        SharedDataPtr _data;

        union // placeholder for binary-compatible changes
        {
            char dummy[16];
        };

    };
}

#endif // EQ_OBJECTMANAGER_H

