
/* Copyright (c) 2007-2013, Stefan Eilemann <eile@equalizergraphics.com>
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
#include <lunchbox/uploader.h>
#include <string.h>

namespace eq
{
namespace util
{
ObjectManager::ObjectManager( const GLEWContext* const glewContext )
        : _data( new SharedData( glewContext ))
{
}

ObjectManager::ObjectManager( ObjectManager& shared )
        : _data( shared._data )
{
    LBASSERT( _data );
}

ObjectManager::~ObjectManager()
{
    _data = 0;
}

ObjectManager& ObjectManager::operator = ( ObjectManager& rhs )
{
    _data = rhs._data;
    return *this;
}

void ObjectManager::clear()
{
    _data = new SharedData( 0 );
}

ObjectManager::SharedData::SharedData( const GLEWContext* gl )
        : glewContext( new GLEWContext )
{
    if( gl )
        memcpy( glewContext, gl, sizeof( GLEWContext ));
    else
        lunchbox::setZero( glewContext, sizeof( GLEWContext ));
}

ObjectManager::SharedData::~SharedData()
{
    // Do not delete GL objects, we may no longer have a GL context.
    if( !lists.empty( ))
        LBWARN << lists.size()
               << " lists still allocated in ObjectManager destructor"
               << std::endl;
    LBASSERT( lists.empty( ));
    lists.clear();

    if( !textures.empty( ))
        LBWARN << textures.size()
               << " textures still allocated in ObjectManager destructor"
               << std::endl;
    LBASSERT( textures.empty( ));
    textures.clear();

    if( !buffers.empty( ))
        LBWARN << buffers.size()
               << " buffers still allocated in ObjectManager destructor"
               << std::endl;
    LBASSERT( buffers.empty( ));
    buffers.clear();

    if( !programs.empty( ))
        LBWARN << programs.size()
               << " programs still allocated in ObjectManager destructor"
               << std::endl;
    LBASSERT( programs.empty( ));
    programs.clear();

    if( !shaders.empty( ))
        LBWARN << shaders.size()
               << " shaders still allocated in ObjectManager destructor"
               << std::endl;
    LBASSERT( shaders.empty( ));
    shaders.clear();

    if( !eqTextures.empty( ))
        LBWARN << eqTextures.size()
               << " eq::Texture still allocated in ObjectManager destructor"
               << std::endl;
    LBASSERT( eqTextures.empty( ));
    eqTextures.clear();

    if( !eqFonts.empty( ))
        LBWARN << eqFonts.size()
               << " eq::BitmapFont still allocated in ObjectManager destructor"
               << std::endl;
    LBASSERT( eqFonts.empty( ));
    eqFonts.clear();

    if( !eqFrameBufferObjects.empty( ))
        LBWARN << eqFrameBufferObjects.size()
               << " eq::FrameBufferObject's still allocated in ObjectManager "
               << "destructor" << std::endl;
    LBASSERT( eqFrameBufferObjects.empty( ));
    eqFrameBufferObjects.clear();

    if( !eqUploaders.empty( ))
        LBWARN << eqUploaders.size()
               << " uploader still allocated in ObjectManager destructor"
               << std::endl;
#ifdef EQ_OM_TRACE_ALLOCATIONS
    LBASSERTINFO( eqUploaders.empty(), eqUploaderAllocs.begin()->second );
#else
    LBASSERTINFO( eqUploaders.empty(), (void*)eqUploaders.begin()->second );
#endif
    eqUploaders.clear();
    delete glewContext;
}

void ObjectManager::deleteAll()
{
    for( ObjectHash::const_iterator i = _data->lists.begin();
         i != _data->lists.end(); ++i )
    {
        const Object& object = i->second;
        LBVERB << "Delete list " << object.id << std::endl;
        glDeleteLists( object.id, object.num );
    }
    _data->lists.clear();

    for( ObjectHash::const_iterator i = _data->textures.begin();
         i != _data->textures.end(); ++i )
    {
        const Object& object = i->second;
        LBVERB << "Delete texture " << object.id << std::endl;
        glDeleteTextures( 1, &object.id );
    }
    _data->textures.clear();

    for( ObjectHash::const_iterator i = _data->buffers.begin();
         i != _data->buffers.end(); ++i )
    {
        const Object& object = i->second;
        LBVERB << "Delete buffer " << object.id << std::endl;
        glDeleteBuffers( 1, &object.id );
    }
    _data->buffers.clear();

    for( ObjectHash::const_iterator i = _data->programs.begin();
         i != _data->programs.end(); ++i )
    {
        const Object& object = i->second;
        LBVERB << "Delete program " << object.id << std::endl;
        glDeleteProgram( object.id );
    }
    _data->programs.clear();

    for( ObjectHash::const_iterator i = _data->shaders.begin();
         i != _data->shaders.end(); ++i )
    {
        const Object& object = i->second;
        LBVERB << "Delete shader " << object.id << std::endl;
        glDeleteShader( object.id );
    }
    _data->shaders.clear();

    for( TextureHash::const_iterator i = _data->eqTextures.begin();
         i != _data->eqTextures.end(); ++i )
    {
        Texture* texture = i->second;
        LBVERB << "Delete eq::Texture " << i->first << " @" << (void*)texture
               << std::endl;
        texture->flush();
        delete texture;
    }
    _data->eqTextures.clear();

    for( FontHash::const_iterator i = _data->eqFonts.begin();
         i != _data->eqFonts.end(); ++i )
    {
        util::BitmapFont* font = i->second;
        LBVERB << "Delete eq::Font " << i->first << " @" << (void*)font
               << std::endl;
        font->exit();
        delete font;
    }
    _data->eqFonts.clear();

    for( FBOHash::const_iterator i =
             _data->eqFrameBufferObjects.begin();
         i != _data->eqFrameBufferObjects.end(); ++i )
    {
        FrameBufferObject* frameBufferObject = i->second;
        LBVERB << "Delete eq::FrameBufferObject " << i->first << " @"
               << (void*)frameBufferObject << std::endl;
        frameBufferObject->exit();
        delete frameBufferObject;
    }
    _data->eqFrameBufferObjects.clear();

    for( UploaderHash::const_iterator i = _data->eqUploaders.begin();
         i != _data->eqUploaders.end(); ++i )
    {
        lunchbox::Uploader* uploader = i->second;
        LBVERB << "Delete uploader " << i->first << " @" << (void*)uploader
               << std::endl;
        uploader->clear();
        delete uploader;
    }
    _data->eqUploaders.clear();
}

// display list functions

GLuint ObjectManager::getList( const void* key ) const
{
    ObjectHash::const_iterator i = _data->lists.find( key );
    if( i == _data->lists.end( ))
        return INVALID;

    const Object& object = i->second;
    return object.id;
}

GLuint ObjectManager::newList( const void* key, const GLsizei num )
{
    if( _data->lists.find( key ) != _data->lists.end( ))
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

    Object& object   = _data->lists[ key ];
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
    ObjectHash::iterator i = _data->lists.find( key );
    if( i == _data->lists.end( ))
        return;

    const Object& object = i->second;
    glDeleteLists( object.id, object.num );
    _data->lists.erase( i );
}

// texture object functions

GLuint ObjectManager::getTexture( const void* key ) const
{
    ObjectHash::const_iterator i = _data->textures.find( key );
    if( i == _data->textures.end( ))
        return INVALID;

    const Object& object = i->second;
    return object.id;
}

GLuint ObjectManager::newTexture( const void* key )
{
    if( _data->textures.find( key ) != _data->textures.end( ))
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

    Object& object   = _data->textures[ key ];
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
    ObjectHash::iterator i = _data->textures.find( key );
    if( i == _data->textures.end( ))
        return;

    const Object& object = i->second;
    glDeleteTextures( 1, &object.id );
    _data->textures.erase( i );
}

// buffer object functions

bool ObjectManager::supportsBuffers() const
{
    return ( GLEW_VERSION_1_5 );
}

GLuint ObjectManager::getBuffer( const void* key ) const
{
    ObjectHash::const_iterator i = _data->buffers.find( key );
    if( i == _data->buffers.end() )
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

    if( _data->buffers.find( key ) != _data->buffers.end() )
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

    Object& object     = _data->buffers[ key ];
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
    ObjectHash::iterator i = _data->buffers.find( key );
    if( i == _data->buffers.end() )
        return;

    const Object& object = i->second;
    glDeleteBuffers( 1, &object.id );
    _data->buffers.erase( i );
}

// program object functions

bool ObjectManager::supportsPrograms() const
{
    return ( GLEW_VERSION_2_0 );
}

GLuint ObjectManager::getProgram( const void* key ) const
{
    ObjectHash::const_iterator i = _data->programs.find( key );
    if( i == _data->programs.end() )
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

    if( _data->programs.find( key ) != _data->programs.end() )
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

    Object& object     = _data->programs[ key ];
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
    ObjectHash::iterator i = _data->programs.find( key );
    if( i == _data->programs.end() )
        return;

    const Object& object = i->second;
    glDeleteProgram( object.id );
    _data->programs.erase( i );
}

// shader object functions

bool ObjectManager::supportsShaders() const
{
    return ( GLEW_VERSION_2_0 );
}

GLuint ObjectManager::getShader( const void* key ) const
{
    ObjectHash::const_iterator i = _data->shaders.find( key );
    if( i == _data->shaders.end() )
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

    if( _data->shaders.find( key ) != _data->shaders.end() )
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


    Object& object     = _data->shaders[ key ];
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
    ObjectHash::iterator i = _data->shaders.find( key );
    if( i == _data->shaders.end() )
        return;

    const Object& object = i->second;
    glDeleteShader( object.id );
    _data->shaders.erase( i );
}

Accum* ObjectManager::getEqAccum( const void* key ) const
{
    AccumHash::const_iterator i = _data->accums.find( key );
    if( i == _data->accums.end( ))
        return 0;

    return i->second;
}

Accum* ObjectManager::newEqAccum( const void* key )
{
    if( _data->accums.find( key ) != _data->accums.end( ))
    {
        LBWARN << "Requested new Accumulation for existing key" << std::endl;
        return 0;
    }

    Accum* accum = new Accum( _data->glewContext );
    _data->accums[ key ] = accum;
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
    AccumHash::iterator i = _data->accums.find( key );
    if( i == _data->accums.end( ))
        return;

    Accum* accum = i->second;
    _data->accums.erase( i );

    accum->exit();
    delete accum;
}

// eq::CompressorData object functions
lunchbox::Uploader* ObjectManager::getEqUploader( const void* key ) const
{
    UploaderHash::const_iterator i = _data->eqUploaders.find( key );
    if( i == _data->eqUploaders.end( ))
        return 0;

    return i->second;
}

lunchbox::Uploader* ObjectManager::newEqUploader( const void* key )
{
    if( _data->eqUploaders.find( key ) != _data->eqUploaders.end( ))
    {
        LBWARN << "Requested new compressor for existing key" << std::endl;
        return 0;
    }

    lunchbox::Uploader* compressor = new lunchbox::Uploader;
    _data->eqUploaders[ key ] = compressor;
#ifdef EQ_OM_TRACE_ALLOCATIONS
    std::ostringstream out;
    out << lunchbox::backtrace;
    _data->eqUploaderAllocs[ key ] = out.str();
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
    UploaderHash::iterator i = _data->eqUploaders.find( key );
    if( i == _data->eqUploaders.end( ))
        return;

    lunchbox::Uploader* uploader = i->second;
    _data->eqUploaders.erase( i );
#ifdef EQ_OM_TRACE_ALLOCATIONS
    _data->eqUploaderAllocs.erase( key );
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
    TextureHash::const_iterator i = _data->eqTextures.find( key );
    if( i == _data->eqTextures.end( ))
        return 0;

    return i->second;
}

Texture* ObjectManager::newEqTexture( const void* key, const GLenum target )
{
    if( _data->eqTextures.find( key ) != _data->eqTextures.end( ))
    {
        LBWARN << "Requested new eqTexture for existing key" << std::endl;
        return 0;
    }

    Texture* texture = new Texture( target, _data->glewContext );
    _data->eqTextures[ key ] = texture;
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
    TextureHash::iterator i = _data->eqTextures.find( key );
    if( i == _data->eqTextures.end( ))
        return;

    Texture* texture = i->second;
    _data->eqTextures.erase( i );

    texture->flush();
    delete texture;
}

// eq::util::BitmapFont object functions
util::BitmapFont* ObjectManager::getEqBitmapFont( const void* key ) const
{
    FontHash::const_iterator i = _data->eqFonts.find( key );
    if( i == _data->eqFonts.end( ))
        return 0;

    return i->second;
}

util::BitmapFont* ObjectManager::newEqBitmapFont( const void* key )
{
    if( _data->eqFonts.find( key ) != _data->eqFonts.end( ))
    {
        LBWARN << "Requested new eqFont for existing key" << std::endl;
        return 0;
    }

    util::BitmapFont* font = new util::BitmapFont( *this, key );
    _data->eqFonts[ key ] = font;
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
    FontHash::iterator i = _data->eqFonts.find( key );
    if( i == _data->eqFonts.end( ))
        return;

    util::BitmapFont* font = i->second;
    _data->eqFonts.erase( i );

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
    FBOHash::const_iterator i = _data->eqFrameBufferObjects.find(key);
    if( i == _data->eqFrameBufferObjects.end( ))
        return 0;

    return i->second;
}

FrameBufferObject* ObjectManager::newEqFrameBufferObject( const void* key )
{
    if( _data->eqFrameBufferObjects.find( key ) !=
        _data->eqFrameBufferObjects.end( ))
    {
        LBWARN << "Requested new eqFrameBufferObject for existing key"
               << std::endl;
        return 0;
    }

    FrameBufferObject* frameBufferObject =
                                    new FrameBufferObject( _data->glewContext );
    _data->eqFrameBufferObjects[ key ] = frameBufferObject;
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
    FBOHash::iterator i = _data->eqFrameBufferObjects.find(key);
    if( i == _data->eqFrameBufferObjects.end( ))
        return;

    FrameBufferObject* frameBufferObject = i->second;
    _data->eqFrameBufferObjects.erase( i );

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
    PBOHash::const_iterator i = _data->eqPixelBufferObjects.find(key);
    if( i == _data->eqPixelBufferObjects.end( ))
        return 0;

    return i->second;
}

PixelBufferObject* ObjectManager::newEqPixelBufferObject( const void* key,
                                                         const bool threadSafe )
{
    if( _data->eqPixelBufferObjects.find( key ) !=
        _data->eqPixelBufferObjects.end( ))
    {
        LBWARN << "Requested new eqPixelBufferObject for existing key"
               << std::endl;
        return 0;
    }

    PixelBufferObject* pixelBufferObject =
                        new PixelBufferObject( _data->glewContext, threadSafe );
    _data->eqPixelBufferObjects[ key ] = pixelBufferObject;
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
    PBOHash::iterator i = _data->eqPixelBufferObjects.find(key);
    if( i == _data->eqPixelBufferObjects.end( ))
        return;

    PixelBufferObject* pixelBufferObject = i->second;
    _data->eqPixelBufferObjects.erase( i );

    pixelBufferObject->destroy();
    delete pixelBufferObject;
}

}
}
