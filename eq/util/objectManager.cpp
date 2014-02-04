
/* Copyright (c) 2007-2014, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#include "objectManager.h"

#include "accum.h"
#include "bitmapFont.h"
#include "frameBufferObject.h"
#include "pixelBufferObject.h"
#include "texture.h"

#include <eq/client/gl.h>
#include <lunchbox/hash.h>
#include <lunchbox/referenced.h>
#include <lunchbox/uploader.h>
#include <string.h>

//#define EQ_OM_TRACE_ALLOCATIONS

namespace eq
{
namespace util
{
namespace
{
struct Object
{
    unsigned   id;
    unsigned   num;
};

typedef stde::hash_map< const void*, Object >     ObjectHash;
typedef stde::hash_map< const void*, Texture* >   TextureHash;
typedef stde::hash_map< const void*, FrameBufferObject* > FBOHash;
typedef stde::hash_map< const void*, PixelBufferObject* > PBOHash;
typedef stde::hash_map< const void*, util::BitmapFont* > FontHash;
typedef stde::hash_map< const void*, Accum* > AccumHash;
typedef stde::hash_map< const void*, lunchbox::Uploader* > UploaderHash;
#   ifdef EQ_OM_TRACE_ALLOCATIONS
typedef stde::hash_map< const void*, std::string > UploaderAllocs;
#   endif
}

namespace detail
{
class ObjectManager : public lunchbox::Referenced
{
public:
    ObjectManager( const GLEWContext* gl )
    {
        if( gl )
            memcpy( &glewContext, gl, sizeof( GLEWContext ));
        else
            lunchbox::setZero( &glewContext, sizeof( GLEWContext ));
    }

    virtual ~ObjectManager()
    {
        // Do not delete GL objects, we may no longer have a GL context.
        if( !lists.empty( ))
            LBWARN << lists.size()
                   << " lists allocated in ObjectManager destructor"
                   << std::endl;
        LBASSERT( lists.empty( ));
        lists.clear();

        if( !textures.empty( ))
            LBWARN << textures.size()
                   << " textures allocated in ObjectManager destructor"
                   << std::endl;
        LBASSERT( textures.empty( ));
        textures.clear();

        if( !buffers.empty( ))
            LBWARN << buffers.size()
                   << " buffers allocated in ObjectManager destructor"
                   << std::endl;
        LBASSERT( buffers.empty( ));
        buffers.clear();

        if( !programs.empty( ))
            LBWARN << programs.size()
                   << " programs allocated in ObjectManager destructor"
                   << std::endl;
        LBASSERT( programs.empty( ));
        programs.clear();

        if( !shaders.empty( ))
            LBWARN << shaders.size()
                   << " shaders allocated in ObjectManager destructor"
                   << std::endl;
        LBASSERT( shaders.empty( ));
        shaders.clear();

        if( !eqTextures.empty( ))
            LBWARN << eqTextures.size()
                   << " eq::Texture allocated in ObjectManager destructor"
                   << std::endl;
        LBASSERT( eqTextures.empty( ));
        eqTextures.clear();

        if( !eqFonts.empty( ))
            LBWARN << eqFonts.size()
                   << " eq::BitmapFont allocated in ObjectManager destructor"
                   << std::endl;
        LBASSERT( eqFonts.empty( ));
        eqFonts.clear();

        if( !eqFrameBufferObjects.empty( ))
            LBWARN << eqFrameBufferObjects.size()
                   << " eq::FrameBufferObject's allocated in ObjectManager "
                   << "destructor" << std::endl;
        LBASSERT( eqFrameBufferObjects.empty( ));
        eqFrameBufferObjects.clear();

        if( !eqUploaders.empty( ))
            LBWARN << eqUploaders.size()
                   << " uploader allocated in ObjectManager destructor"
                   << std::endl;

#ifdef EQ_OM_TRACE_ALLOCATIONS
        LBASSERTINFO( eqUploaders.empty(), eqUploaderAllocs.begin()->second );
#else
        LBASSERTINFO( eqUploaders.empty(), (void*)eqUploaders.begin()->second );
#endif
        eqUploaders.clear();
    }


    GLEWContext glewContext;
    ObjectHash lists;
    ObjectHash textures;
    ObjectHash buffers;
    ObjectHash programs;
    ObjectHash shaders;
    ObjectHash uploaderDatas;
    AccumHash  accums;
    TextureHash eqTextures;
    FBOHash eqFrameBufferObjects;
    PBOHash eqPixelBufferObjects;
    FontHash eqFonts;
    UploaderHash eqUploaders;
#   ifdef EQ_OM_TRACE_ALLOCATIONS
    UploaderAllocs eqUploaderAllocs;
#   endif
};
}

ObjectManager::ObjectManager( const GLEWContext* const glewContext )
    : impl_( new detail::ObjectManager( glewContext ))
{
}

ObjectManager::ObjectManager( const ObjectManager& shared )
        : impl_( shared.impl_ )
{
    LBASSERT( impl_ );
    LBASSERT( glewGetContext( ));
}

ObjectManager::~ObjectManager()
{
    impl_ = 0;
}

ObjectManager& ObjectManager::operator = ( const ObjectManager& rhs )
{
    if( this != &rhs )
        impl_ = rhs.impl_;
    LBASSERT( glewGetContext( ));
    return *this;
}

void ObjectManager::clear()
{
    impl_ = new detail::ObjectManager( 0 );
}

bool ObjectManager::isShared() const
{
    return impl_->getRefCount() > 1;
}

const GLEWContext* ObjectManager::glewGetContext() const
{
    return &impl_->glewContext;
}

void ObjectManager::deleteAll()
{
    for( ObjectHash::const_iterator i = impl_->lists.begin();
         i != impl_->lists.end(); ++i )
    {
        const Object& object = i->second;
        LBVERB << "Delete list " << object.id << std::endl;
        glDeleteLists( object.id, object.num );
    }
    impl_->lists.clear();

    for( ObjectHash::const_iterator i = impl_->textures.begin();
         i != impl_->textures.end(); ++i )
    {
        const Object& object = i->second;
        LBVERB << "Delete texture " << object.id << std::endl;
        glDeleteTextures( 1, &object.id );
    }
    impl_->textures.clear();

    for( ObjectHash::const_iterator i = impl_->buffers.begin();
         i != impl_->buffers.end(); ++i )
    {
        const Object& object = i->second;
        LBVERB << "Delete buffer " << object.id << std::endl;
        glDeleteBuffers( 1, &object.id );
    }
    impl_->buffers.clear();

    for( ObjectHash::const_iterator i = impl_->programs.begin();
         i != impl_->programs.end(); ++i )
    {
        const Object& object = i->second;
        LBVERB << "Delete program " << object.id << std::endl;
        glDeleteProgram( object.id );
    }
    impl_->programs.clear();

    for( ObjectHash::const_iterator i = impl_->shaders.begin();
         i != impl_->shaders.end(); ++i )
    {
        const Object& object = i->second;
        LBVERB << "Delete shader " << object.id << std::endl;
        glDeleteShader( object.id );
    }
    impl_->shaders.clear();

    for( TextureHash::const_iterator i = impl_->eqTextures.begin();
         i != impl_->eqTextures.end(); ++i )
    {
        Texture* texture = i->second;
        LBVERB << "Delete eq::Texture " << i->first << " @" << (void*)texture
               << std::endl;
        texture->flush();
        delete texture;
    }
    impl_->eqTextures.clear();

    for( FontHash::const_iterator i = impl_->eqFonts.begin();
         i != impl_->eqFonts.end(); ++i )
    {
        util::BitmapFont* font = i->second;
        LBVERB << "Delete eq::Font " << i->first << " @" << (void*)font
               << std::endl;
        font->exit();
        delete font;
    }
    impl_->eqFonts.clear();

    for( FBOHash::const_iterator i =
             impl_->eqFrameBufferObjects.begin();
         i != impl_->eqFrameBufferObjects.end(); ++i )
    {
        FrameBufferObject* frameBufferObject = i->second;
        LBVERB << "Delete eq::FrameBufferObject " << i->first << " @"
               << (void*)frameBufferObject << std::endl;
        frameBufferObject->exit();
        delete frameBufferObject;
    }
    impl_->eqFrameBufferObjects.clear();

    for( UploaderHash::const_iterator i = impl_->eqUploaders.begin();
         i != impl_->eqUploaders.end(); ++i )
    {
        lunchbox::Uploader* uploader = i->second;
        LBVERB << "Delete uploader " << i->first << " @" << (void*)uploader
               << std::endl;
        uploader->clear();
        delete uploader;
    }
    impl_->eqUploaders.clear();
}

// display list functions

GLuint ObjectManager::getList( const void* key ) const
{
    ObjectHash::const_iterator i = impl_->lists.find( key );
    if( i == impl_->lists.end( ))
        return INVALID;

    const Object& object = i->second;
    return object.id;
}

GLuint ObjectManager::newList( const void* key, const GLsizei num )
{
    if( impl_->lists.find( key ) != impl_->lists.end( ))
    {
        LBWARN << "Requested new list for existing key" << std::endl;
        return INVALID;
    }

    const GLuint id = glGenLists( num );
    if( !id )
    {
        LBWARN << "glGenLists failed: " << glGetError() << std::endl;
        return INVALID;
    }

    Object& object   = impl_->lists[ key ];
    object.id        = id;
    object.num       = num;

    return id;
}

GLuint ObjectManager::obtainList( const void* key, const GLsizei num )
{
    const GLuint id = getList( key );
    if( id != INVALID )
        return id;
    return newList( key, num );
}

void ObjectManager::deleteList( const void* key )
{
    ObjectHash::iterator i = impl_->lists.find( key );
    if( i == impl_->lists.end( ))
        return;

    const Object& object = i->second;
    glDeleteLists( object.id, object.num );
    impl_->lists.erase( i );
}

// texture object functions

GLuint ObjectManager::getTexture( const void* key ) const
{
    ObjectHash::const_iterator i = impl_->textures.find( key );
    if( i == impl_->textures.end( ))
        return INVALID;

    const Object& object = i->second;
    return object.id;
}

GLuint ObjectManager::newTexture( const void* key )
{
    if( impl_->textures.find( key ) != impl_->textures.end( ))
    {
        LBWARN << "Requested new texture for existing key" << std::endl;
        return INVALID;
    }

    GLuint id = INVALID;
    glGenTextures( 1, &id );
    if( id == INVALID )
    {
        LBWARN << "glGenTextures failed: " << glGetError() << std::endl;
        return INVALID;
    }

    Object& object   = impl_->textures[ key ];
    object.id        = id;
    return id;
}

GLuint ObjectManager::obtainTexture( const void* key )
{
    const GLuint id = getTexture( key );
    if( id != INVALID )
        return id;
    return newTexture( key );
}

void ObjectManager::deleteTexture( const void* key )
{
    ObjectHash::iterator i = impl_->textures.find( key );
    if( i == impl_->textures.end( ))
        return;

    const Object& object = i->second;
    glDeleteTextures( 1, &object.id );
    impl_->textures.erase( i );
}

// buffer object functions

bool ObjectManager::supportsBuffers() const
{
    return ( GLEW_VERSION_1_5 );
}

GLuint ObjectManager::getBuffer( const void* key ) const
{
    ObjectHash::const_iterator i = impl_->buffers.find( key );
    if( i == impl_->buffers.end() )
        return INVALID;

    const Object& object = i->second;
    return object.id;
}

GLuint ObjectManager::newBuffer( const void* key )
{
    if( !GLEW_VERSION_1_5 )
    {
        LBWARN << "glGenBuffers not available" << std::endl;
        return INVALID;
    }

    if( impl_->buffers.find( key ) != impl_->buffers.end() )
    {
        LBWARN << "Requested new buffer for existing key" << std::endl;
        return INVALID;
    }

    GLuint id = INVALID;
    glGenBuffers( 1, &id );

    if( id == INVALID )
    {
        LBWARN << "glGenBuffers failed: " << glGetError() << std::endl;
        return INVALID;
    }

    Object& object     = impl_->buffers[ key ];
    object.id          = id;
    return id;
}

GLuint ObjectManager::obtainBuffer( const void* key )
{
    const GLuint id = getBuffer( key );
    if( id != INVALID )
        return id;
    return newBuffer( key );
}

void ObjectManager::deleteBuffer( const void* key )
{
    ObjectHash::iterator i = impl_->buffers.find( key );
    if( i == impl_->buffers.end() )
        return;

    const Object& object = i->second;
    glDeleteBuffers( 1, &object.id );
    impl_->buffers.erase( i );
}

// program object functions

bool ObjectManager::supportsPrograms() const
{
    return ( GLEW_VERSION_2_0 );
}

GLuint ObjectManager::getProgram( const void* key ) const
{
    ObjectHash::const_iterator i = impl_->programs.find( key );
    if( i == impl_->programs.end() )
        return INVALID;

    const Object& object = i->second;
    return object.id;
}

GLuint ObjectManager::newProgram( const void* key )
{
    if( !GLEW_VERSION_2_0 )
    {
        LBWARN << "glCreateProgram not available" << std::endl;
        return INVALID;
    }

    if( impl_->programs.find( key ) != impl_->programs.end() )
    {
        LBWARN << "Requested new program for existing key" << std::endl;
        return INVALID;
    }

    const GLuint id = glCreateProgram();
    if( !id )
    {
        LBWARN << "glCreateProgram failed: " << glGetError() << std::endl;
        return INVALID;
    }

    Object& object     = impl_->programs[ key ];
    object.id          = id;
    return id;
}

GLuint ObjectManager::obtainProgram( const void* key )
{
    const GLuint id = getProgram( key );
    if( id != INVALID )
        return id;
    return newProgram( key );
}

void ObjectManager::deleteProgram( const void* key )
{
    ObjectHash::iterator i = impl_->programs.find( key );
    if( i == impl_->programs.end() )
        return;

    const Object& object = i->second;
    glDeleteProgram( object.id );
    impl_->programs.erase( i );
}

// shader object functions

bool ObjectManager::supportsShaders() const
{
    return ( GLEW_VERSION_2_0 );
}

GLuint ObjectManager::getShader( const void* key ) const
{
    ObjectHash::const_iterator i = impl_->shaders.find( key );
    if( i == impl_->shaders.end() )
        return INVALID;

    const Object& object = i->second;
    return object.id;
}

GLuint ObjectManager::newShader( const void* key, const GLenum type )
{
    if( !GLEW_VERSION_2_0 )
    {
        LBWARN << "glCreateShader not available" << std::endl;
        return INVALID;
    }

    if( impl_->shaders.find( key ) != impl_->shaders.end() )
    {
        LBWARN << "Requested new shader for existing key" << std::endl;
        return INVALID;
    }

    const GLuint id = glCreateShader( type );
    if( !id )
    {
        LBWARN << "glCreateShader failed: " << glGetError() << std::endl;
        return INVALID;
    }


    Object& object     = impl_->shaders[ key ];
    object.id          = id;
    return id;
}

GLuint ObjectManager::obtainShader( const void* key, const GLenum type )
{
    const GLuint id = getShader( key );
    if( id != INVALID )
        return id;
    return newShader( key, type );
}

void ObjectManager::deleteShader( const void* key )
{
    ObjectHash::iterator i = impl_->shaders.find( key );
    if( i == impl_->shaders.end() )
        return;

    const Object& object = i->second;
    glDeleteShader( object.id );
    impl_->shaders.erase( i );
}

Accum* ObjectManager::getEqAccum( const void* key ) const
{
    AccumHash::const_iterator i = impl_->accums.find( key );
    if( i == impl_->accums.end( ))
        return 0;

    return i->second;
}

Accum* ObjectManager::newEqAccum( const void* key )
{
    if( impl_->accums.find( key ) != impl_->accums.end( ))
    {
        LBWARN << "Requested new Accumulation for existing key" << std::endl;
        return 0;
    }

    Accum* accum = new Accum( &impl_->glewContext );
    impl_->accums[ key ] = accum;
    return accum;
}

Accum* ObjectManager::obtainEqAccum( const void* key )
{
    Accum* accum = getEqAccum( key );
    if( accum )
        return accum;
    return newEqAccum( key );
}

void ObjectManager::deleteEqAccum( const void* key )
{
    AccumHash::iterator i = impl_->accums.find( key );
    if( i == impl_->accums.end( ))
        return;

    Accum* accum = i->second;
    impl_->accums.erase( i );

    accum->exit();
    delete accum;
}

// eq::CompressorData object functions
lunchbox::Uploader* ObjectManager::getEqUploader( const void* key ) const
{
    UploaderHash::const_iterator i = impl_->eqUploaders.find( key );
    if( i == impl_->eqUploaders.end( ))
        return 0;

    return i->second;
}

lunchbox::Uploader* ObjectManager::newEqUploader( const void* key )
{
    if( impl_->eqUploaders.find( key ) != impl_->eqUploaders.end( ))
    {
        LBWARN << "Requested new compressor for existing key" << std::endl;
        return 0;
    }

    lunchbox::Uploader* compressor = new lunchbox::Uploader;
    impl_->eqUploaders[ key ] = compressor;
#ifdef EQ_OM_TRACE_ALLOCATIONS
    std::ostringstream out;
    out << lunchbox::backtrace;
    impl_->eqUploaderAllocs[ key ] = out.str();
#endif

    return compressor;
}

lunchbox::Uploader* ObjectManager::obtainEqUploader( const void* key )
{
    lunchbox::Uploader* compressor = getEqUploader( key );
    if( compressor )
        return compressor;
    return newEqUploader( key );
}

void ObjectManager::deleteEqUploader( const void* key )
{
    UploaderHash::iterator i = impl_->eqUploaders.find( key );
    if( i == impl_->eqUploaders.end( ))
        return;

    lunchbox::Uploader* uploader = i->second;
    impl_->eqUploaders.erase( i );
#ifdef EQ_OM_TRACE_ALLOCATIONS
    impl_->eqUploaderAllocs.erase( key );
#endif
    uploader->clear();
    delete uploader;
}

// eq::Texture object functions
bool ObjectManager::supportsEqTexture() const
{
    return (GLEW_ARB_texture_rectangle);
}

Texture* ObjectManager::getEqTexture( const void* key ) const
{
    TextureHash::const_iterator i = impl_->eqTextures.find( key );
    if( i == impl_->eqTextures.end( ))
        return 0;

    return i->second;
}

Texture* ObjectManager::newEqTexture( const void* key, const GLenum target )
{
    if( impl_->eqTextures.find( key ) != impl_->eqTextures.end( ))
    {
        LBWARN << "Requested new eqTexture for existing key" << std::endl;
        return 0;
    }

    Texture* texture = new Texture( target, &impl_->glewContext );
    impl_->eqTextures[ key ] = texture;
    return texture;
}

Texture* ObjectManager::obtainEqTexture( const void* key, const GLenum target )
{
    Texture* texture = getEqTexture( key );
    if( texture )
        return texture;
    return newEqTexture( key, target );
}

void   ObjectManager::deleteEqTexture( const void* key )
{
    TextureHash::iterator i = impl_->eqTextures.find( key );
    if( i == impl_->eqTextures.end( ))
        return;

    Texture* texture = i->second;
    impl_->eqTextures.erase( i );

    texture->flush();
    delete texture;
}

// eq::util::BitmapFont object functions
util::BitmapFont* ObjectManager::getEqBitmapFont( const void* key ) const
{
    FontHash::const_iterator i = impl_->eqFonts.find( key );
    if( i == impl_->eqFonts.end( ))
        return 0;

    return i->second;
}

util::BitmapFont* ObjectManager::newEqBitmapFont( const void* key )
{
    if( impl_->eqFonts.find( key ) != impl_->eqFonts.end( ))
    {
        LBWARN << "Requested new eqFont for existing key" << std::endl;
        return 0;
    }

    util::BitmapFont* font = new util::BitmapFont( *this, key );
    impl_->eqFonts[ key ] = font;
    return font;
}

util::BitmapFont* ObjectManager::obtainEqBitmapFont( const void* key )
{
    util::BitmapFont* font = getEqBitmapFont( key );
    if( font )
        return font;
    return newEqBitmapFont( key );
}

void ObjectManager::deleteEqBitmapFont( const void* key )
{
    FontHash::iterator i = impl_->eqFonts.find( key );
    if( i == impl_->eqFonts.end( ))
        return;

    util::BitmapFont* font = i->second;
    impl_->eqFonts.erase( i );

    font->exit();
    delete font;
}

// eq::FrameBufferObject object functions
bool ObjectManager::supportsEqFrameBufferObject() const
{
    return (GLEW_EXT_framebuffer_object);
}

FrameBufferObject* ObjectManager::getEqFrameBufferObject( const void* key )
    const
{
    FBOHash::const_iterator i = impl_->eqFrameBufferObjects.find(key);
    if( i == impl_->eqFrameBufferObjects.end( ))
        return 0;

    return i->second;
}

FrameBufferObject* ObjectManager::newEqFrameBufferObject( const void* key )
{
    if( impl_->eqFrameBufferObjects.find( key ) !=
        impl_->eqFrameBufferObjects.end( ))
    {
        LBWARN << "Requested new eqFrameBufferObject for existing key"
               << std::endl;
        return 0;
    }

    FrameBufferObject* frameBufferObject =
                                    new FrameBufferObject( &impl_->glewContext );
    impl_->eqFrameBufferObjects[ key ] = frameBufferObject;
    return frameBufferObject;
}

FrameBufferObject* ObjectManager::obtainEqFrameBufferObject( const void* key )
{
    FrameBufferObject* frameBufferObject = getEqFrameBufferObject( key );
    if( frameBufferObject )
        return frameBufferObject;
    return newEqFrameBufferObject( key );
}

void ObjectManager::deleteEqFrameBufferObject( const void* key )
{
    FBOHash::iterator i = impl_->eqFrameBufferObjects.find(key);
    if( i == impl_->eqFrameBufferObjects.end( ))
        return;

    FrameBufferObject* frameBufferObject = i->second;
    impl_->eqFrameBufferObjects.erase( i );

    frameBufferObject->exit();
    delete frameBufferObject;
}

// eq::PixelBufferObject object functions
bool ObjectManager::supportsEqPixelBufferObject() const
{
    return (GLEW_ARB_pixel_buffer_object);
}

PixelBufferObject* ObjectManager::getEqPixelBufferObject( const void* key )
    const
{
    PBOHash::const_iterator i = impl_->eqPixelBufferObjects.find(key);
    if( i == impl_->eqPixelBufferObjects.end( ))
        return 0;

    return i->second;
}

PixelBufferObject* ObjectManager::newEqPixelBufferObject( const void* key,
                                                         const bool threadSafe )
{
    if( impl_->eqPixelBufferObjects.find( key ) !=
        impl_->eqPixelBufferObjects.end( ))
    {
        LBWARN << "Requested new eqPixelBufferObject for existing key"
               << std::endl;
        return 0;
    }

    PixelBufferObject* pixelBufferObject =
        new PixelBufferObject( &impl_->glewContext, threadSafe );
    impl_->eqPixelBufferObjects[ key ] = pixelBufferObject;
    return pixelBufferObject;
}

PixelBufferObject* ObjectManager::obtainEqPixelBufferObject( const void* key,
                                                         const bool threadSafe )
{
    PixelBufferObject* pixelBufferObject = getEqPixelBufferObject( key );
    if( pixelBufferObject )
    {
        if( pixelBufferObject->isThreadSafe() != threadSafe )
        {
            LBERROR << "Wrong sharing option requested!" << std::endl;
            return 0;
        }
        return pixelBufferObject;
    }
    return newEqPixelBufferObject( key, threadSafe );
}

void ObjectManager::deleteEqPixelBufferObject( const void* key )
{
    PBOHash::iterator i = impl_->eqPixelBufferObjects.find(key);
    if( i == impl_->eqPixelBufferObjects.end( ))
        return;

    PixelBufferObject* pixelBufferObject = i->second;
    impl_->eqPixelBufferObjects.erase( i );

    pixelBufferObject->destroy();
    delete pixelBufferObject;
}

}
}
