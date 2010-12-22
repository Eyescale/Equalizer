
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c)      2010, Cedric Stalder <cedric.stalder@gmail.com>
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
#include "gpuCompressor.h"
#include "texture.h"

#include <string.h>

// instantiate desired key types -- see objectManager.cpp

namespace eq
{
namespace util
{
template< class T >
ObjectManager< T >::ObjectManager( const GLEWContext* const glewContext )
        : _glewContext( glewContext )
        , _data( new SharedData )
{
    EQASSERT( glewContext );
}

template< class T >
ObjectManager< T >::ObjectManager( const GLEWContext* const glewContext, 
                                   ObjectManager* shared )
        : _glewContext( glewContext )
        , _data( shared->_data )
{
    EQASSERT( shared );
    EQASSERT( glewContext );
}

template< class T >
ObjectManager<T>::~ObjectManager()
{
    _data = 0;
}

template< class T >
ObjectManager<T>::SharedData::~SharedData()
{
    // Do not delete GL objects, we may no longer have a GL context.
    if( !lists.empty( ))
        EQWARN << lists.size() 
               << " lists still allocated in ObjectManager destructor" << std::endl;
    lists.clear();

    if( !textures.empty( ))
        EQWARN << textures.size() 
               << " textures still allocated in ObjectManager destructor" 
               << std::endl;
    textures.clear();

    if( !buffers.empty( ))
        EQWARN << buffers.size() 
               << " buffers still allocated in ObjectManager destructor" 
               << std::endl;
    buffers.clear();

    if( !programs.empty( ))
        EQWARN << programs.size() 
               << " programs still allocated in ObjectManager destructor" 
               << std::endl;
    programs.clear();

    if( !shaders.empty( ))
        EQWARN << shaders.size() 
               << " shaders still allocated in ObjectManager destructor" 
               << std::endl;
    shaders.clear();

    if( !eqTextures.empty( ))
        EQWARN << eqTextures.size() 
               << " eq::Texture still allocated in ObjectManager destructor" 
               << std::endl;
    eqTextures.clear();

    if( !eqFonts.empty( ))
        EQWARN << eqFonts.size() 
               << " eq::BitmapFont still allocated in ObjectManager destructor" 
               << std::endl;
    eqFonts.clear();

    if( !eqFrameBufferObjects.empty( ))
        EQWARN << eqFrameBufferObjects.size() 
               << " eq::FrameBufferObject's still allocated in ObjectManager "
               << "destructor" << std::endl;
    eqFrameBufferObjects.clear();

    if( !eqUploaders.empty( ))
        EQWARN << eqUploaders.size() 
               << " eq::GPUCompressor's still allocated in ObjectManager "
               << "destructor" << std::endl;
    eqUploaders.clear();
}

template< class T >
void ObjectManager<T>::deleteAll()
{
    for( typename ObjectHash::const_iterator i = _data->lists.begin(); 
         i != _data->lists.end(); ++i )
    {
        const Object& object = i->second;
        EQVERB << "Delete list " << object.id << std::endl;
        glDeleteLists( object.id, object.num ); 
    }
    _data->lists.clear();

    for( typename ObjectHash::const_iterator i = _data->textures.begin(); 
         i != _data->textures.end(); ++i )
    {
        const Object& object = i->second;
        EQVERB << "Delete texture " << object.id << std::endl;
        glDeleteTextures( 1, &object.id ); 
    }
    _data->textures.clear();

    for( typename ObjectHash::const_iterator i = _data->buffers.begin(); 
         i != _data->buffers.end(); ++i )
    {
        const Object& object = i->second;
        EQVERB << "Delete buffer " << object.id << std::endl;
        glDeleteBuffers( 1, &object.id ); 
    }
    _data->buffers.clear();

    for( typename ObjectHash::const_iterator i = _data->programs.begin(); 
         i != _data->programs.end(); ++i )
    {
        const Object& object = i->second;
        EQVERB << "Delete program " << object.id << std::endl;
        glDeleteProgram( object.id ); 
    }
    _data->programs.clear();

    for( typename ObjectHash::const_iterator i = _data->shaders.begin(); 
         i != _data->shaders.end(); ++i )
    {
        const Object& object = i->second;
        EQVERB << "Delete shader " << object.id << std::endl;
        glDeleteShader( object.id ); 
    }
    _data->shaders.clear();

    for( typename TextureHash::const_iterator i = _data->eqTextures.begin(); 
         i != _data->eqTextures.end(); ++i )
    {
        Texture* texture = i->second;
        EQVERB << "Delete eq::Texture " << i->first << " @" << (void*)texture
               << std::endl;
        texture->flush();
        delete texture;
    }
    _data->eqTextures.clear();

    for( typename FontHash::const_iterator i = _data->eqFonts.begin(); 
         i != _data->eqFonts.end(); ++i )
    {
        util::BitmapFont< T >* font = i->second;
        EQVERB << "Delete eq::Font " << i->first << " @" << (void*)font
               << std::endl;
        font->exit();
        delete font;
    }
    _data->eqFonts.clear();

    for( typename FBOHash::const_iterator i =
             _data->eqFrameBufferObjects.begin();
         i != _data->eqFrameBufferObjects.end(); ++i )
    {
        FrameBufferObject* frameBufferObject = i->second;
        EQVERB << "Delete eq::FrameBufferObject " << i->first << " @" 
               << (void*)frameBufferObject << std::endl;
        frameBufferObject->exit();
        delete frameBufferObject;
    }
    _data->eqFrameBufferObjects.clear();

    for( typename UploaderHash::const_iterator i =
             _data->eqUploaders.begin();
         i != _data->eqUploaders.end(); ++i )
    {
        GPUCompressor* uploader = i->second;
        EQVERB << "Delete eq::GPUCompressor " << i->first << " @" 
               << (void*)uploader << std::endl;
        delete uploader;
    }
    _data->eqUploaders.clear();
}

// display list functions

template< class T >
GLuint ObjectManager<T>::getList( const T& key ) const
{
    typename ObjectHash::const_iterator i = _data->lists.find( key );
    if( i == _data->lists.end( ))
        return INVALID;

    const Object& object = i->second;
    return object.id;
}

template< class T >
GLuint ObjectManager<T>::newList( const T& key, const GLsizei num )
{
    if( _data->lists.find( key ) != _data->lists.end( ))
    {
        EQWARN << "Requested new list for existing key" << std::endl;
        return INVALID;
    }

    const GLuint id = glGenLists( num );
    if( !id )
    {
        EQWARN << "glGenLists failed: " << glGetError() << std::endl;
        return INVALID;
    }
    
    Object& object   = _data->lists[ key ];
    object.id        = id;
    object.num       = num;

    return id;
}

template< class T >
GLuint ObjectManager<T>::obtainList( const T& key, const GLsizei num )
{
    const GLuint id = getList( key );
    if( id != INVALID )
        return id;
    return newList( key, num );
}

template< class T >
void   ObjectManager<T>::deleteList( const T& key )
{
    typename ObjectHash::iterator i = _data->lists.find( key );
    if( i == _data->lists.end( ))
        return;

    const Object& object = i->second;
    glDeleteLists( object.id, object.num );
    _data->lists.erase( i );
}

// texture object functions

template< class T >
GLuint ObjectManager<T>::getTexture( const T& key ) const
{
    typename ObjectHash::const_iterator i = _data->textures.find( key );
    if( i == _data->textures.end( ))
        return INVALID;

    const Object& object = i->second;
    return object.id;
}

template< class T >
GLuint ObjectManager<T>::newTexture( const T& key )
{
    if( _data->textures.find( key ) != _data->textures.end( ))
    {
        EQWARN << "Requested new texture for existing key" << std::endl;
        return INVALID;
    }

    GLuint id = INVALID;
    glGenTextures( 1, &id );
    if( id == INVALID )
    {
        EQWARN << "glGenTextures failed: " << glGetError() << std::endl;
        return INVALID;
    }
    
    Object& object   = _data->textures[ key ];
    object.id        = id;
    return id;
}

template< class T >
GLuint ObjectManager<T>::obtainTexture( const T& key )
{
    const GLuint id = getTexture( key );
    if( id != INVALID )
        return id;
    return newTexture( key );
}

template< class T >
void   ObjectManager<T>::deleteTexture( const T& key )
{
    typename ObjectHash::iterator i = _data->textures.find( key );
    if( i == _data->textures.end( ))
        return;

    const Object& object = i->second;
    glDeleteTextures( 1, &object.id );
    _data->textures.erase( i );
}

// buffer object functions

template< class T >
bool ObjectManager<T>::supportsBuffers() const
{
    return ( GLEW_VERSION_1_5 );
}

template< class T >
GLuint ObjectManager<T>::getBuffer( const T& key ) const
{
    typename ObjectHash::const_iterator i = _data->buffers.find( key );
    if( i == _data->buffers.end() )
        return INVALID;

    const Object& object = i->second;
    return object.id;
}

template< class T >
GLuint ObjectManager<T>::newBuffer( const T& key )
{
    if( !GLEW_VERSION_1_5 )
    {
        EQWARN << "glGenBuffers not available" << std::endl;
        return INVALID;
    }

    if( _data->buffers.find( key ) != _data->buffers.end() )
    {
        EQWARN << "Requested new buffer for existing key" << std::endl;
        return INVALID;
    }

    GLuint id = INVALID;
    glGenBuffers( 1, &id );

    if( id == INVALID )
    {
        EQWARN << "glGenBuffers failed: " << glGetError() << std::endl;
        return INVALID;
    }
    
    Object& object     = _data->buffers[ key ];
    object.id          = id;
    return id;
}

template< class T >
GLuint ObjectManager<T>::obtainBuffer( const T& key )
{
    const GLuint id = getBuffer( key );
    if( id != INVALID )
        return id;
    return newBuffer( key );
}

template< class T >
void ObjectManager<T>::deleteBuffer( const T& key )
{
    typename ObjectHash::iterator i = _data->buffers.find( key );
    if( i == _data->buffers.end() )
        return;

    const Object& object = i->second;
    glDeleteBuffers( 1, &object.id );
    _data->buffers.erase( i );
}

// program object functions

template< class T >
bool ObjectManager<T>::supportsPrograms() const
{
    return ( GLEW_VERSION_2_0 );
}

template< class T >
GLuint ObjectManager<T>::getProgram( const T& key ) const
{
    typename ObjectHash::const_iterator i = _data->programs.find( key );
    if( i == _data->programs.end() )
        return INVALID;

    const Object& object = i->second;
    return object.id;
}

template< class T >
GLuint ObjectManager<T>::newProgram( const T& key )
{
    if( !GLEW_VERSION_2_0 )
    {
        EQWARN << "glCreateProgram not available" << std::endl;
        return INVALID;
    }

    if( _data->programs.find( key ) != _data->programs.end() )
    {
        EQWARN << "Requested new program for existing key" << std::endl;
        return INVALID;
    }

    const GLuint id = glCreateProgram();
    if( !id )
    {
        EQWARN << "glCreateProgram failed: " << glGetError() << std::endl;
        return INVALID;
    }
    
    Object& object     = _data->programs[ key ];
    object.id          = id;
    return id;
}

template< class T >
GLuint ObjectManager<T>::obtainProgram( const T& key )
{
    const GLuint id = getProgram( key );
    if( id != INVALID )
        return id;
    return newProgram( key );
}

template< class T >
void ObjectManager<T>::deleteProgram( const T& key )
{
    typename ObjectHash::iterator i = _data->programs.find( key );
    if( i == _data->programs.end() )
        return;

    const Object& object = i->second;
    glDeleteProgram( object.id );
    _data->programs.erase( i );
}

// shader object functions

template< class T >
bool ObjectManager<T>::supportsShaders() const
{
    return ( GLEW_VERSION_2_0 );
}

template< class T >
GLuint ObjectManager<T>::getShader( const T& key ) const
{
    typename ObjectHash::const_iterator i = _data->shaders.find( key );
    if( i == _data->shaders.end() )
        return INVALID;

    const Object& object = i->second;
    return object.id;
}

template< class T >
GLuint ObjectManager<T>::newShader( const T& key, const GLenum type )
{
    if( !GLEW_VERSION_2_0 )
    {
        EQWARN << "glCreateShader not available" << std::endl;
        return INVALID;
    }

    if( _data->shaders.find( key ) != _data->shaders.end() )
    {
        EQWARN << "Requested new shader for existing key" << std::endl;
        return INVALID;
    }

    const GLuint id = glCreateShader( type );
    if( !id )
    {
        EQWARN << "glCreateShader failed: " << glGetError() << std::endl;
        return INVALID;
    }

    
    Object& object     = _data->shaders[ key ];
    object.id          = id;
    return id;
}

template< class T >
GLuint ObjectManager<T>::obtainShader( const T& key, const GLenum type )
{
    const GLuint id = getShader( key );
    if( id != INVALID )
        return id;
    return newShader( key, type );
}

template< class T >
void ObjectManager<T>::deleteShader( const T& key )
{
    typename ObjectHash::iterator i = _data->shaders.find( key );
    if( i == _data->shaders.end() )
        return;

    const Object& object = i->second;
    glDeleteShader( object.id );
    _data->shaders.erase( i );
}

template< class T >
Accum* ObjectManager<T>::getEqAccum( const T& key ) const
{
    typename AccumHash::const_iterator i = _data->accums.find( key );
    if( i == _data->accums.end( ))
        return 0;

    return i->second;
}

template< class T >
Accum* ObjectManager<T>::newEqAccum( const T& key )
{
    if( _data->accums.find( key ) != _data->accums.end( ))
    {
        EQWARN << "Requested new Accumulation for existing key" << std::endl;
        return 0;
    }

    Accum* accum = new Accum( _glewContext );
    _data->accums[ key ] = accum;
    return accum;
}

template< class T >
Accum* ObjectManager<T>::obtainEqAccum( const T& key )
{
    Accum* accum = getEqAccum( key );
    if( accum )
        return accum;
    return newEqAccum( key );
}

template< class T >
void ObjectManager<T>::deleteEqAccum( const T& key )
{
    typename AccumHash::iterator i = _data->accums.find( key );
    if( i == _data->accums.end( ))
        return;

    Accum* accum = i->second;
    _data->accums.erase( i );

    accum->exit();
    delete accum;
}

// eq::CompressorData object functions
template< class T >
GPUCompressor* ObjectManager<T>::getEqUploader( const T& key ) const
{
    typename UploaderHash::const_iterator i = _data->eqUploaders.find( key );
    if( i == _data->eqUploaders.end( ))
        return 0;

    return i->second;
}

template< class T >
GPUCompressor* ObjectManager<T>::newEqUploader( const T& key )
{
    if( _data->eqUploaders.find( key ) != _data->eqUploaders.end( ))
    {
        EQWARN << "Requested new Accumulation for existing key" << std::endl;
        return 0;
    }

    GPUCompressor* compressorData = new GPUCompressor( _glewContext );
    _data->eqUploaders[ key ] = compressorData;
    return compressorData;
}

template< class T >
GPUCompressor* ObjectManager<T>::obtainEqUploader( const T& key )
{
    GPUCompressor* compressorData = getEqUploader( key );
    if( compressorData )
        return compressorData;
    return newEqUploader( key );
}

template< class T >
void ObjectManager<T>::deleteEqUploader( const T& key )
{
    typename UploaderHash::iterator i = _data->eqUploaders.find( key );
    if( i == _data->eqUploaders.end( ))
        return;

    GPUCompressor* compressorData = i->second;
    _data->eqUploaders.erase( i );

    delete compressorData;
}

// eq::Texture object functions
template< class T >
bool ObjectManager<T>::supportsEqTexture() const
{
    return (GLEW_ARB_texture_rectangle);
}

template< class T >
Texture* ObjectManager<T>::getEqTexture( const T& key ) const
{
    typename TextureHash::const_iterator i = _data->eqTextures.find( key );
    if( i == _data->eqTextures.end( ))
        return 0;

    return i->second;
}

template< class T >
Texture* ObjectManager<T>::newEqTexture( const T& key, const GLenum target )
{
    if( _data->eqTextures.find( key ) != _data->eqTextures.end( ))
    {
        EQWARN << "Requested new eqTexture for existing key" << std::endl;
        return 0;
    }

    Texture* texture = new Texture( target, _glewContext );
    _data->eqTextures[ key ] = texture;
    return texture;
}

template< class T >
Texture* ObjectManager<T>::obtainEqTexture( const T& key, const GLenum target )
{
    Texture* texture = getEqTexture( key );
    if( texture )
        return texture;
    return newEqTexture( key, target );
}

template< class T >
void   ObjectManager<T>::deleteEqTexture( const T& key )
{
    typename TextureHash::iterator i = _data->eqTextures.find( key );
    if( i == _data->eqTextures.end( ))
        return;

    Texture* texture = i->second;
    _data->eqTextures.erase( i );

    texture->flush();
    delete texture;
}

// eq::util::BitmapFont object functions
template< class T >
util::BitmapFont< T >* ObjectManager<T>::getEqBitmapFont( const T& key ) const
{
    typename FontHash::const_iterator i = _data->eqFonts.find( key );
    if( i == _data->eqFonts.end( ))
        return 0;

    return i->second;
}

template< class T >
util::BitmapFont< T >* ObjectManager<T>::newEqBitmapFont( const T& key )
{
    if( _data->eqFonts.find( key ) != _data->eqFonts.end( ))
    {
        EQWARN << "Requested new eqFont for existing key" << std::endl;
        return 0;
    }

    util::BitmapFont< T >* font = new util::BitmapFont<T>( *this, key );
    _data->eqFonts[ key ] = font;
    return font;
}

template< class T >
util::BitmapFont< T >* ObjectManager<T>::obtainEqBitmapFont( const T& key )
{
    util::BitmapFont< T >* font = getEqBitmapFont( key );
    if( font )
        return font;
    return newEqBitmapFont( key );
}

template< class T >
void ObjectManager<T>::deleteEqBitmapFont( const T& key )
{
    typename FontHash::iterator i = _data->eqFonts.find( key );
    if( i == _data->eqFonts.end( ))
        return;

    util::BitmapFont< T >* font = i->second;
    _data->eqFonts.erase( i );

    font->exit();
    delete font;
}

// eq::FrameBufferObject object functions
template< class T >
bool ObjectManager<T>::supportsEqFrameBufferObject() const
{
    return (GLEW_EXT_framebuffer_object);
}

template< class T >
FrameBufferObject* ObjectManager<T>::getEqFrameBufferObject( const T& key )
    const
{
    typename FBOHash::const_iterator i = _data->eqFrameBufferObjects.find(key);
    if( i == _data->eqFrameBufferObjects.end( ))
        return 0;

    return i->second;
}

template< class T >
FrameBufferObject* ObjectManager<T>::newEqFrameBufferObject( const T& key )
{
    if( _data->eqFrameBufferObjects.find( key ) !=
        _data->eqFrameBufferObjects.end( ))
    {
        EQWARN << "Requested new eqFrameBufferObject for existing key"
               << std::endl;
        return 0;
    }

    FrameBufferObject* frameBufferObject = 
                                        new FrameBufferObject( _glewContext );
    _data->eqFrameBufferObjects[ key ] = frameBufferObject;
    return frameBufferObject;
}

template< class T >
FrameBufferObject* ObjectManager<T>::obtainEqFrameBufferObject( const T& key )
{
    FrameBufferObject* frameBufferObject = getEqFrameBufferObject( key );
    if( frameBufferObject )
        return frameBufferObject;
    return newEqFrameBufferObject( key );
}

template< class T >
void ObjectManager<T>::deleteEqFrameBufferObject( const T& key )
{
    typename FBOHash::iterator i = _data->eqFrameBufferObjects.find(key);
    if( i == _data->eqFrameBufferObjects.end( ))
        return;

    FrameBufferObject* frameBufferObject = i->second;
    _data->eqFrameBufferObjects.erase( i );

    frameBufferObject->exit();
    delete frameBufferObject;
}

}
}
