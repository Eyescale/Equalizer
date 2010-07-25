
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

#ifndef EQUTIL_OBJECTMANAGER_H
#define EQUTIL_OBJECTMANAGER_H

#include <eq/client/os.h>             // OpenGL/GLEW types

#include <eq/base/base.h>             // EQ_EXPORT definition
#include <eq/base/debug.h>            // EQASSERT definition
#include <eq/base/hash.h>             // member
#include <eq/base/nonCopyable.h>      // base class
#include <eq/base/referenced.h>       // base class

namespace eq
{
namespace util
{
    template< typename T > class BitmapFont;
    class Accum;
    class CompressorDataGPU;
    class FrameBufferObject;
    class Texture;

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
    template< typename T > class ObjectManager : public base::NonCopyable
    {
    public:
        enum
        {
            INVALID = 0 //<! return value for failed operations.
        };

        /** Construct a new object manager. */
        EQ_EXPORT ObjectManager( GLEWContext* const glewContext );

        /** Construct a new object manager sharing data with another manager. */
        EQ_EXPORT ObjectManager( GLEWContext* const glewContext,
                                 ObjectManager* shared );

        EQ_EXPORT virtual ~ObjectManager();

        /** @return true if more than one OM is using the same data. */
        bool isShared() const { return _data->getRefCount() > 1; }

        EQ_EXPORT void deleteAll();

        EQ_EXPORT GLuint getList( const T& key ) const;
        EQ_EXPORT GLuint newList( const T& key, const GLsizei num = 1 );
        EQ_EXPORT GLuint obtainList( const T& key, const GLsizei num = 1 );
        EQ_EXPORT void   deleteList( const T& key );

        EQ_EXPORT GLuint getTexture( const T& key ) const;
        EQ_EXPORT GLuint newTexture( const T& key );
        EQ_EXPORT GLuint obtainTexture( const T& key );
        EQ_EXPORT void   deleteTexture( const T& key );

        EQ_EXPORT bool   supportsBuffers() const;
        EQ_EXPORT GLuint getBuffer( const T& key ) const;
        EQ_EXPORT GLuint newBuffer( const T& key );
        EQ_EXPORT GLuint obtainBuffer( const T& key );
        EQ_EXPORT void   deleteBuffer( const T& key );

        EQ_EXPORT bool   supportsPrograms() const;
        EQ_EXPORT GLuint getProgram( const T& key ) const;
        EQ_EXPORT GLuint newProgram( const T& key );
        EQ_EXPORT GLuint obtainProgram( const T& key );
        EQ_EXPORT void   deleteProgram( const T& key );

        EQ_EXPORT bool   supportsShaders() const;
        EQ_EXPORT GLuint getShader( const T& key ) const;
        EQ_EXPORT GLuint newShader( const T& key, const GLenum type );
        EQ_EXPORT GLuint obtainShader( const T& key, const GLenum type );
        EQ_EXPORT void   deleteShader( const T& key );

        EQ_EXPORT Accum* getEqAccum( const T& key ) const;
        EQ_EXPORT Accum* newEqAccum( const T& key );
        EQ_EXPORT Accum* obtainEqAccum( const T& key );
        EQ_EXPORT void   deleteEqAccum( const T& key );

        EQ_EXPORT CompressorDataGPU* getEqUploader( const T& key ) const;
        EQ_EXPORT CompressorDataGPU* newEqUploader( const T& key );
        EQ_EXPORT CompressorDataGPU* obtainEqUploader( const T& key );
        EQ_EXPORT void   deleteEqUploader( const T& key );

        EQ_EXPORT bool     supportsEqTexture() const;
        EQ_EXPORT Texture* getEqTexture( const T& key ) const;
        EQ_EXPORT Texture* newEqTexture( const T& key );
        EQ_EXPORT Texture* obtainEqTexture( const T& key );
        EQ_EXPORT void     deleteEqTexture( const T& key );

        EQ_EXPORT bool               supportsEqFrameBufferObject() const;
        EQ_EXPORT FrameBufferObject* getEqFrameBufferObject(const T& key) const;
        EQ_EXPORT FrameBufferObject* newEqFrameBufferObject( const T& key );
        EQ_EXPORT FrameBufferObject* obtainEqFrameBufferObject( const T& key );
        EQ_EXPORT void               deleteEqFrameBufferObject( const T& key );

        EQ_EXPORT util::BitmapFont< T >* getEqBitmapFont( const T& key ) const;
        EQ_EXPORT util::BitmapFont< T >* newEqBitmapFont( const T& key );
        EQ_EXPORT util::BitmapFont< T >* obtainEqBitmapFont( const T& key );
        EQ_EXPORT void                   deleteEqBitmapFont( const T& key );

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
        typedef stde::hash_map< T, FrameBufferObject* > FBOHash;
        typedef stde::hash_map< T, util::BitmapFont< T >* > FontHash;
        typedef stde::hash_map< T, Accum* > AccumHash;
        typedef stde::hash_map< T, CompressorDataGPU* > UploaderHash;

        struct SharedData : public base::Referenced
        {
            virtual ~SharedData();

            ObjectHash lists;
            ObjectHash textures;
            ObjectHash buffers;
            ObjectHash programs;
            ObjectHash shaders;
            ObjectHash uploaderDatas;
            AccumHash accums;
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

