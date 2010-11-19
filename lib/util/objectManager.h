
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQUTIL_OBJECTMANAGER_H
#define EQUTIL_OBJECTMANAGER_H

#include <eq/util/types.h>

#include <eq/client/os.h>             // OpenGL/GLEW types
#include <eq/base/base.h>             // EQ_CLIENT_DECL definition
#include <eq/base/debug.h>            // EQASSERT definition
#include <eq/base/hash.h>             // member
#include <eq/base/nonCopyable.h>      // base class
#include <eq/base/referenced.h>       // base class

namespace eq
{
namespace util
{
    /**
     * A facility class to manage OpenGL objects across shared contexts.
     *
     * The object manager implements object sharing in the same way as
     * OpenGL. During creation, a shared object manager may be given, causing
     * the two (or more) object managers to allocate objects from the same
     * namespace. The last object manager will delete all data allocated on the
     * host. OpenGL objects have to be explictly deleted using deleteAll() to
     * ensure an OpenGL context is still current during destruction.
     *
     * For each type of OpenGL object supported the following methods are
     * available: 
     * - supportsObject: Check if the necessary OpenGL version or extension
     *   is present
     * - getObject: Lookup an existing object, may return 0
     * - newObject: Allocate new object, returns false if key exists
     * - obtainObject: Lookup or allocate an object for the given key
     * - deleteObject: Delete the object of the given key and all associated
     *   OpenGL data
     *
     * @sa http://www.equalizergraphics.com/documents/design/objectManager.html
     */
    template< class T > class ObjectManager : public base::NonCopyable
    {
    public:
        enum
        {
            INVALID = 0 //<! return value for failed operations.
        };

        /** Construct a new object manager. */
        EQ_CLIENT_DECL ObjectManager( const GLEWContext* const glewContext );

        /** Construct a new object manager sharing data with another manager. */
        EQ_CLIENT_DECL ObjectManager( const GLEWContext* const glewContext,
                                 ObjectManager* shared );

        EQ_CLIENT_DECL virtual ~ObjectManager();

        /** @return true if more than one OM is using the same data. */
        bool isShared() const { return _data->getRefCount() > 1; }

        EQ_CLIENT_DECL void deleteAll();

        EQ_CLIENT_DECL GLuint getList( const T& key ) const;
        EQ_CLIENT_DECL GLuint newList( const T& key, const GLsizei num = 1 );
        EQ_CLIENT_DECL GLuint obtainList( const T& key, const GLsizei num = 1 );
        EQ_CLIENT_DECL void   deleteList( const T& key );

        EQ_CLIENT_DECL GLuint getTexture( const T& key ) const;
        EQ_CLIENT_DECL GLuint newTexture( const T& key );
        EQ_CLIENT_DECL GLuint obtainTexture( const T& key );
        EQ_CLIENT_DECL void   deleteTexture( const T& key );

        EQ_CLIENT_DECL bool   supportsBuffers() const;
        EQ_CLIENT_DECL GLuint getBuffer( const T& key ) const;
        EQ_CLIENT_DECL GLuint newBuffer( const T& key );
        EQ_CLIENT_DECL GLuint obtainBuffer( const T& key );
        EQ_CLIENT_DECL void   deleteBuffer( const T& key );

        EQ_CLIENT_DECL bool   supportsPrograms() const;
        EQ_CLIENT_DECL GLuint getProgram( const T& key ) const;
        EQ_CLIENT_DECL GLuint newProgram( const T& key );
        EQ_CLIENT_DECL GLuint obtainProgram( const T& key );
        EQ_CLIENT_DECL void   deleteProgram( const T& key );

        EQ_CLIENT_DECL bool   supportsShaders() const;
        EQ_CLIENT_DECL GLuint getShader( const T& key ) const;
        EQ_CLIENT_DECL GLuint newShader( const T& key, const GLenum type );
        EQ_CLIENT_DECL GLuint obtainShader( const T& key, const GLenum type );
        EQ_CLIENT_DECL void   deleteShader( const T& key );

        EQ_CLIENT_DECL Accum* getEqAccum( const T& key ) const;
        EQ_CLIENT_DECL Accum* newEqAccum( const T& key );
        EQ_CLIENT_DECL Accum* obtainEqAccum( const T& key );
        EQ_CLIENT_DECL void   deleteEqAccum( const T& key );

        EQ_CLIENT_DECL GPUCompressor* getEqUploader( const T& key ) const;
        EQ_CLIENT_DECL GPUCompressor* newEqUploader( const T& key );
        EQ_CLIENT_DECL GPUCompressor* obtainEqUploader( const T& key );
        EQ_CLIENT_DECL void   deleteEqUploader( const T& key );

        EQ_CLIENT_DECL bool     supportsEqTexture() const;
        EQ_CLIENT_DECL Texture* getEqTexture( const T& key ) const;
        EQ_CLIENT_DECL Texture* newEqTexture( const T& key, const GLenum target );
        EQ_CLIENT_DECL Texture* obtainEqTexture( const T& key, const GLenum target );
        EQ_CLIENT_DECL void     deleteEqTexture( const T& key );

        EQ_CLIENT_DECL bool               supportsEqFrameBufferObject() const;
        EQ_CLIENT_DECL FrameBufferObject* getEqFrameBufferObject(const T& key) const;
        EQ_CLIENT_DECL FrameBufferObject* newEqFrameBufferObject( const T& key );
        EQ_CLIENT_DECL FrameBufferObject* obtainEqFrameBufferObject( const T& key );
        EQ_CLIENT_DECL void               deleteEqFrameBufferObject( const T& key );

        EQ_CLIENT_DECL util::BitmapFont< T >* getEqBitmapFont( const T& key ) const;
        EQ_CLIENT_DECL util::BitmapFont< T >* newEqBitmapFont( const T& key );
        EQ_CLIENT_DECL util::BitmapFont< T >* obtainEqBitmapFont( const T& key );
        EQ_CLIENT_DECL void                   deleteEqBitmapFont( const T& key );

        const GLEWContext* glewGetContext() const { return _glewContext; }

    private:
        const GLEWContext* const _glewContext;

        struct Object
        {
            GLuint   id;
            GLuint   num;
        };

        typedef stde::hash_map< T, Object >     ObjectHash;
        typedef stde::hash_map< T, Texture* >   TextureHash;
        typedef stde::hash_map< T, FrameBufferObject* > FBOHash;
        typedef stde::hash_map< T, util::BitmapFont< T >* > FontHash;
        typedef stde::hash_map< T, Accum* > AccumHash;
        typedef stde::hash_map< T, GPUCompressor* > UploaderHash;

        struct SharedData : public base::Referenced
        {
            virtual ~SharedData();

            ObjectHash lists;
            ObjectHash textures;
            ObjectHash buffers;
            ObjectHash programs;
            ObjectHash shaders;
            ObjectHash uploaderDatas;
            AccumHash  accums;
            TextureHash eqTextures;
            FBOHash eqFrameBufferObjects;
            FontHash eqFonts;
            UploaderHash eqUploaders;

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
}

#endif // EQUTIL_OBJECTMANAGER_H

