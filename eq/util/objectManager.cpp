
/* Copyright (c) 2007-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include <eq/gl.h>
#include <lunchbox/hash.h>
#include <lunchbox/os.h>
#include <lunchbox/referenced.h>
#include <pression/uploader.h>
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
    unsigned id;
    unsigned num;
};

typedef stde::hash_map< const void*, Object >     ObjectHash;
typedef stde::hash_map< const void*, Texture* >   TextureHash;
typedef stde::hash_map< const void*, FrameBufferObject* > FBOHash;
typedef stde::hash_map< const void*, PixelBufferObject* > PBOHash;
typedef stde::hash_map< const void*, util::BitmapFont* > FontHash;
typedef stde::hash_map< const void*, Accum* > AccumHash;
typedef stde::hash_map< const void*, pression::Uploader* > UploaderHash;
#ifdef EQ_OM_TRACE_ALLOCATIONS
typedef stde::hash_map< const void*, std::string > UploaderAllocs;
#endif
}

namespace detail
{
class ObjectManager : public lunchbox::Referenced
{
public:
    explicit ObjectManager( const GLEWContext* gl )
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
    ObjectHash vertexArrays;
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
#ifdef EQ_OM_TRACE_ALLOCATIONS
    UploaderAllocs eqUploaderAllocs;
#endif
};
}

ObjectManager::ObjectManager( const GLEWContext* const glewContext )
    : _impl( new detail::ObjectManager( glewContext ))
{
}

ObjectManager::ObjectManager( const ObjectManager& shared )
        : _impl( shared._impl )
{
    LBASSERT( _impl );
    LBASSERT( glewGetContext( ));
}

ObjectManager::~ObjectManager()
{
}

ObjectManager& ObjectManager::operator = ( const ObjectManager& rhs )
{
    if( this != &rhs )
        _impl = rhs._impl;
    LBASSERT( glewGetContext( ));
    return *this;
}

void ObjectManager::clear()
{
    _impl = new detail::ObjectManager( 0 );
}

bool ObjectManager::isShared() const
{
    return _impl->getRefCount() > 1;
}

const GLEWContext* ObjectManager::glewGetContext() const
{
    return &_impl->glewContext;
}

void ObjectManager::deleteAll()
{
    for( ObjectHash::const_iterator i = _impl->lists.begin();
         i != _impl->lists.end(); ++i )
    {
        const Object& object = i->second;
        LBVERB << "Delete list " << object.id << std::endl;
        EQ_GL_CALL( glDeleteLists( object.id, object.num ));
    }
    _impl->lists.clear();

    for( ObjectHash::const_iterator i = _impl->textures.begin();
         i != _impl->textures.end(); ++i )
    {
        const Object& object = i->second;
        LBVERB << "Delete texture " << object.id << std::endl;
        EQ_GL_CALL( glDeleteTextures( 1, &object.id ));
    }
    _impl->textures.clear();

    for( ObjectHash::const_iterator i = _impl->buffers.begin();
         i != _impl->buffers.end(); ++i )
    {
        const Object& object = i->second;
        LBVERB << "Delete buffer " << object.id << std::endl;
        EQ_GL_CALL( glDeleteBuffers( 1, &object.id ));
    }
    _impl->buffers.clear();

    for( ObjectHash::const_iterator i = _impl->programs.begin();
         i != _impl->programs.end(); ++i )
    {
        const Object& object = i->second;
        LBVERB << "Delete program " << object.id << std::endl;
        EQ_GL_CALL( glDeleteProgram( object.id ));
    }
    _impl->programs.clear();

    for( ObjectHash::const_iterator i = _impl->shaders.begin();
         i != _impl->shaders.end(); ++i )
    {
        const Object& object = i->second;
        LBVERB << "Delete shader " << object.id << std::endl;
        EQ_GL_CALL( glDeleteShader( object.id ));
    }
    _impl->shaders.clear();

    for( TextureHash::const_iterator i = _impl->eqTextures.begin();
         i != _impl->eqTextures.end(); ++i )
    {
        Texture* texture = i->second;
        LBVERB << "Delete eq::Texture " << i->first << " @" << (void*)texture
               << std::endl;
        texture->flush();
        delete texture;
    }
    _impl->eqTextures.clear();

    for( FontHash::const_iterator i = _impl->eqFonts.begin();
         i != _impl->eqFonts.end(); ++i )
    {
        util::BitmapFont* font = i->second;
        LBVERB << "Delete eq::Font " << i->first << " @" << (void*)font
               << std::endl;
        font->exit();
        delete font;
    }
    _impl->eqFonts.clear();

    for( FBOHash::const_iterator i =
             _impl->eqFrameBufferObjects.begin();
         i != _impl->eqFrameBufferObjects.end(); ++i )
    {
        FrameBufferObject* frameBufferObject = i->second;
        LBVERB << "Delete eq::FrameBufferObject " << i->first << " @"
               << (void*)frameBufferObject << std::endl;
        frameBufferObject->exit();
        delete frameBufferObject;
    }
    _impl->eqFrameBufferObjects.clear();

    for( UploaderHash::const_iterator i = _impl->eqUploaders.begin();
         i != _impl->eqUploaders.end(); ++i )
    {
        pression::Uploader* uploader = i->second;
        LBVERB << "Delete uploader " << i->first << " @" << (void*)uploader
               << std::endl;
        uploader->clear();
        delete uploader;
    }
    _impl->eqUploaders.clear();
}

// display list functions

GLuint ObjectManager::getList( const void* key ) const
{
    ObjectHash::const_iterator i = _impl->lists.find( key );
    if( i == _impl->lists.end( ))
        return INVALID;

    const Object& object = i->second;
    return object.id;
}

GLuint ObjectManager::newList( const void* key, const GLsizei num )
{
    if( _impl->lists.find( key ) != _impl->lists.end( ))
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

    Object& object   = _impl->lists[ key ];
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
    ObjectHash::iterator i = _impl->lists.find( key );
    if( i == _impl->lists.end( ))
        return;

    const Object& object = i->second;
    EQ_GL_CALL( glDeleteLists( object.id, object.num ));
    _impl->lists.erase( i );
}

// vertex array functions

GLuint ObjectManager::getVertexArray( const void* key ) const
{
    ObjectHash::const_iterator i = _impl->vertexArrays.find( key );
    if( i == _impl->vertexArrays.end( ))
        return INVALID;

    const Object& object = i->second;
    return object.id;
}

GLuint ObjectManager::newVertexArray( const void* key )
{
    if( _impl->vertexArrays.find( key ) != _impl->vertexArrays.end( ))
    {
        LBWARN << "Requested new vertex array for existing key" << std::endl;
        return INVALID;
    }

    GLuint id = INVALID;
    EQ_GL_CALL( glGenVertexArrays( 1, &id ));
    if( !id )
    {
        LBWARN << "glGenVertexArrays failed: " << glGetError() << std::endl;
        return INVALID;
    }

    Object& object   = _impl->vertexArrays[ key ];
    object.id        = id;
    return id;
}

GLuint ObjectManager::obtainVertexArray( const void* key )
{
    const GLuint id = getVertexArray( key );
    if( id != INVALID )
        return id;
    return newVertexArray( key );
}

void ObjectManager::deleteVertexArray( const void* key )
{
    ObjectHash::iterator i = _impl->vertexArrays.find( key );
    if( i == _impl->vertexArrays.end( ))
        return;

    const Object& object = i->second;
    EQ_GL_CALL( glDeleteVertexArrays( 1, &object.id ));
    _impl->vertexArrays.erase( i );
}

// texture object functions

GLuint ObjectManager::getTexture( const void* key ) const
{
    ObjectHash::const_iterator i = _impl->textures.find( key );
    if( i == _impl->textures.end( ))
        return INVALID;

    const Object& object = i->second;
    return object.id;
}

GLuint ObjectManager::newTexture( const void* key )
{
    if( _impl->textures.find( key ) != _impl->textures.end( ))
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

    Object& object   = _impl->textures[ key ];
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
    ObjectHash::iterator i = _impl->textures.find( key );
    if( i == _impl->textures.end( ))
        return;

    const Object& object = i->second;
    EQ_GL_CALL( glDeleteTextures( 1, &object.id ));
    _impl->textures.erase( i );
}

// buffer object functions

bool ObjectManager::supportsBuffers() const
{
    return ( GLEW_VERSION_1_5 );
}

GLuint ObjectManager::getBuffer( const void* key ) const
{
    ObjectHash::const_iterator i = _impl->buffers.find( key );
    if( i == _impl->buffers.end() )
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

    if( _impl->buffers.find( key ) != _impl->buffers.end() )
    {
        LBWARN << "Requested new buffer for existing key " << key << std::endl;
        return INVALID;
    }

    GLuint id = INVALID;
    glGenBuffers( 1, &id );
    if( id == INVALID )
    {
        LBWARN << "glGenBuffers failed: " << glGetError() << std::endl;
        return INVALID;
    }

    Object& object     = _impl->buffers[ key ];
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
    ObjectHash::iterator i = _impl->buffers.find( key );
    if( i == _impl->buffers.end() )
        return;

    const Object& object = i->second;
    EQ_GL_CALL( glDeleteBuffers( 1, &object.id ));
    _impl->buffers.erase( i );
}

// program object functions

bool ObjectManager::supportsPrograms() const
{
    return ( GLEW_VERSION_2_0 );
}

GLuint ObjectManager::getProgram( const void* key ) const
{
    ObjectHash::const_iterator i = _impl->programs.find( key );
    if( i == _impl->programs.end() )
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

    if( _impl->programs.find( key ) != _impl->programs.end() )
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

    Object& object     = _impl->programs[ key ];
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
    ObjectHash::iterator i = _impl->programs.find( key );
    if( i == _impl->programs.end() )
        return;

    const Object& object = i->second;
    EQ_GL_CALL( glDeleteProgram( object.id ));
    _impl->programs.erase( i );
}

// shader object functions

bool ObjectManager::supportsShaders() const
{
    return ( GLEW_VERSION_2_0 );
}

GLuint ObjectManager::getShader( const void* key ) const
{
    ObjectHash::const_iterator i = _impl->shaders.find( key );
    if( i == _impl->shaders.end() )
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

    if( _impl->shaders.find( key ) != _impl->shaders.end() )
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

    Object& object     = _impl->shaders[ key ];
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
    ObjectHash::iterator i = _impl->shaders.find( key );
    if( i == _impl->shaders.end() )
        return;

    const Object& object = i->second;
    EQ_GL_CALL( glDeleteShader( object.id ));
    _impl->shaders.erase( i );
}

Accum* ObjectManager::getEqAccum( const void* key ) const
{
    AccumHash::const_iterator i = _impl->accums.find( key );
    if( i == _impl->accums.end( ))
        return 0;

    return i->second;
}

Accum* ObjectManager::newEqAccum( const void* key )
{
    if( _impl->accums.find( key ) != _impl->accums.end( ))
    {
        LBWARN << "Requested new Accumulation for existing key" << std::endl;
        return 0;
    }

    Accum* accum = new Accum( &_impl->glewContext );
    _impl->accums[ key ] = accum;
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
    AccumHash::iterator i = _impl->accums.find( key );
    if( i == _impl->accums.end( ))
        return;

    Accum* accum = i->second;
    _impl->accums.erase( i );

    accum->exit();
    delete accum;
}

// eq::CompressorData object functions
pression::Uploader* ObjectManager::getEqUploader( const void* key ) const
{
    UploaderHash::const_iterator i = _impl->eqUploaders.find( key );
    if( i == _impl->eqUploaders.end( ))
        return 0;

    return i->second;
}

pression::Uploader* ObjectManager::newEqUploader( const void* key )
{
    if( _impl->eqUploaders.find( key ) != _impl->eqUploaders.end( ))
    {
        LBWARN << "Requested new compressor for existing key" << std::endl;
        return 0;
    }

    pression::Uploader* compressor = new pression::Uploader;
    _impl->eqUploaders[ key ] = compressor;
#ifdef EQ_OM_TRACE_ALLOCATIONS
    std::ostringstream out;
    out << lunchbox::backtrace;
    _impl->eqUploaderAllocs[ key ] = out.str();
#endif

    return compressor;
}

pression::Uploader* ObjectManager::obtainEqUploader( const void* key )
{
    pression::Uploader* compressor = getEqUploader( key );
    if( compressor )
        return compressor;
    return newEqUploader( key );
}

void ObjectManager::deleteEqUploader( const void* key )
{
    UploaderHash::iterator i = _impl->eqUploaders.find( key );
    if( i == _impl->eqUploaders.end( ))
        return;

    pression::Uploader* uploader = i->second;
    _impl->eqUploaders.erase( i );
#ifdef EQ_OM_TRACE_ALLOCATIONS
    _impl->eqUploaderAllocs.erase( key );
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
    TextureHash::const_iterator i = _impl->eqTextures.find( key );
    if( i == _impl->eqTextures.end( ))
        return 0;

    return i->second;
}

Texture* ObjectManager::newEqTexture( const void* key, const GLenum target )
{
    if( _impl->eqTextures.find( key ) != _impl->eqTextures.end( ))
    {
        LBWARN << "Requested new eqTexture for existing key" << std::endl;
        return 0;
    }

    Texture* texture = new Texture( target, &_impl->glewContext );
    _impl->eqTextures[ key ] = texture;
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
    TextureHash::iterator i = _impl->eqTextures.find( key );
    if( i == _impl->eqTextures.end( ))
        return;

    Texture* texture = i->second;
    _impl->eqTextures.erase( i );

    texture->flush();
    delete texture;
}

// eq::util::BitmapFont object functions
util::BitmapFont* ObjectManager::getEqBitmapFont( const void* key ) const
{
    FontHash::const_iterator i = _impl->eqFonts.find( key );
    if( i == _impl->eqFonts.end( ))
        return 0;

    return i->second;
}

util::BitmapFont* ObjectManager::newEqBitmapFont( const void* key )
{
    if( _impl->eqFonts.find( key ) != _impl->eqFonts.end( ))
    {
        LBWARN << "Requested new eqFont for existing key" << std::endl;
        return 0;
    }

    util::BitmapFont* font = new util::BitmapFont( *this, key );
    _impl->eqFonts[ key ] = font;
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
    FontHash::iterator i = _impl->eqFonts.find( key );
    if( i == _impl->eqFonts.end( ))
        return;

    util::BitmapFont* font = i->second;
    _impl->eqFonts.erase( i );

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
    FBOHash::const_iterator i = _impl->eqFrameBufferObjects.find(key);
    if( i == _impl->eqFrameBufferObjects.end( ))
        return 0;

    return i->second;
}

FrameBufferObject* ObjectManager::newEqFrameBufferObject( const void* key )
{
    if( _impl->eqFrameBufferObjects.find( key ) !=
        _impl->eqFrameBufferObjects.end( ))
    {
        LBWARN << "Requested new eqFrameBufferObject for existing key"
               << std::endl;
        return 0;
    }

    FrameBufferObject* frameBufferObject =
                                    new FrameBufferObject( &_impl->glewContext );
    _impl->eqFrameBufferObjects[ key ] = frameBufferObject;
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
    FBOHash::iterator i = _impl->eqFrameBufferObjects.find(key);
    if( i == _impl->eqFrameBufferObjects.end( ))
        return;

    FrameBufferObject* frameBufferObject = i->second;
    _impl->eqFrameBufferObjects.erase( i );

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
    PBOHash::const_iterator i = _impl->eqPixelBufferObjects.find(key);
    if( i == _impl->eqPixelBufferObjects.end( ))
        return 0;

    return i->second;
}

PixelBufferObject* ObjectManager::newEqPixelBufferObject( const void* key,
                                                         const bool threadSafe )
{
    if( _impl->eqPixelBufferObjects.find( key ) !=
        _impl->eqPixelBufferObjects.end( ))
    {
        LBWARN << "Requested new eqPixelBufferObject for existing key"
               << std::endl;
        return 0;
    }

    PixelBufferObject* pixelBufferObject =
        new PixelBufferObject( &_impl->glewContext, threadSafe );
    _impl->eqPixelBufferObjects[ key ] = pixelBufferObject;
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
    PBOHash::iterator i = _impl->eqPixelBufferObjects.find(key);
    if( i == _impl->eqPixelBufferObjects.end( ))
        return;

    PixelBufferObject* pixelBufferObject = i->second;
    _impl->eqPixelBufferObjects.erase( i );

    pixelBufferObject->destroy();
    delete pixelBufferObject;
}

}
}
