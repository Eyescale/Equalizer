
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#include "frameBufferObject.h"
#include "texture.h"

#include <string.h>

using namespace eq;
using namespace std;
using namespace stde;

// instantiate desired key types -- see end of file

template< typename T >
ObjectManager< T >::ObjectManager( GLEWContext* const glewContext )
        : _glewContext( glewContext )
        , _data( new SharedData )
{
    EQASSERT( glewContext );
}

template< typename T >
ObjectManager< T >::ObjectManager( GLEWContext* const glewContext, 
                              ObjectManager* shared )
        : _glewContext( glewContext )
        , _data( shared->_data )
{
    EQASSERT( glewContext );
}

template< typename T >
ObjectManager<T>::~ObjectManager()
{
    _data = 0;
}

template< typename T >
ObjectManager<T>::SharedData::~SharedData()
{
    // Do not delete GL objects, we may no longer have a GL context.
    if( !lists.empty( ))
        EQWARN << lists.size() 
               << " lists still allocated in ObjectManager destructor" << endl;
    lists.clear();

    if( !textures.empty( ))
        EQWARN << textures.size() 
               << " textures still allocated in ObjectManager destructor" 
               << endl;
    textures.clear();

    if( !buffers.empty( ))
        EQWARN << buffers.size() 
               << " buffers still allocated in ObjectManager destructor" 
               << endl;
    buffers.clear();

    if( !programs.empty( ))
        EQWARN << programs.size() 
               << " programs still allocated in ObjectManager destructor" 
               << endl;
    programs.clear();

    if( !shaders.empty( ))
        EQWARN << shaders.size() 
               << " shaders still allocated in ObjectManager destructor" 
               << endl;
    shaders.clear();

    if( !eqTextures.empty( ))
        EQWARN << eqTextures.size() 
               << " eq::Texture's still allocated in ObjectManager destructor" 
               << endl;
    eqTextures.clear();

    if( !eqFrameBufferObjects.empty( ))
        EQWARN << eqFrameBufferObjects.size() 
               << " eq::FrameBufferObject's still allocated in ObjectManager "
               << "destructor" << endl;
    eqFrameBufferObjects.clear();
}

template< typename T >
void ObjectManager<T>::deleteAll()
{
   for( typename ObjectHash::const_iterator i = _data->lists.begin(); 
         i != _data->lists.end(); ++i )
    {
        const Object& object = i->second;
        EQVERB << "Delete list " << object.id << endl;
        glDeleteLists( object.id, object.num ); 
    }
    _data->lists.clear();

    for( typename ObjectHash::const_iterator i = _data->textures.begin(); 
         i != _data->textures.end(); ++i )
    {
        const Object& object = i->second;
        EQVERB << "Delete texture " << object.id << endl;
        glDeleteTextures( 1, &object.id ); 
    }
    _data->textures.clear();

    for( typename ObjectHash::const_iterator i = _data->buffers.begin(); 
         i != _data->buffers.end(); ++i )
    {
        const Object& object = i->second;
        EQVERB << "Delete buffer " << object.id << endl;
        glDeleteBuffers( 1, &object.id ); 
    }
    _data->buffers.clear();

    for( typename ObjectHash::const_iterator i = _data->programs.begin(); 
         i != _data->programs.end(); ++i )
    {
        const Object& object = i->second;
        EQVERB << "Delete program " << object.id << endl;
        glDeleteProgram( object.id ); 
    }
    _data->programs.clear();

    for( typename ObjectHash::const_iterator i = _data->shaders.begin(); 
         i != _data->shaders.end(); ++i )
    {
        const Object& object = i->second;
        EQVERB << "Delete shader " << object.id << endl;
        glDeleteShader( object.id ); 
    }
    _data->shaders.clear();

    for( typename TextureHash::const_iterator i = _data->eqTextures.begin(); 
         i != _data->eqTextures.end(); ++i )
    {
        Texture* texture = i->second;
        EQVERB << "Delete eq::Texture " << i->first << " @" << (void*)texture
               << endl;
        texture->flush();
        delete texture;
    }
    _data->eqTextures.clear();

    for( typename FrameBufferObjectHash::const_iterator i = 
             _data->eqFrameBufferObjects.begin();
         i != _data->eqFrameBufferObjects.end(); ++i )
    {
        FrameBufferObject* frameBufferObject = i->second;
        EQVERB << "Delete eq::FrameBufferObject " << i->first << " @" 
               << (void*)frameBufferObject << endl;
        frameBufferObject->exit();
        delete frameBufferObject;
    }
    _data->eqFrameBufferObjects.clear();
}

// display list functions

template< typename T >
GLuint ObjectManager<T>::getList( const T& key )
{
    if( _data->lists.find( key ) == _data->lists.end( ))
        return INVALID;

    const Object& object = _data->lists[ key ];
    return object.id;
}

template< typename T >
GLuint ObjectManager<T>::newList( const T& key, const GLsizei num )
{
    if( _data->lists.find( key ) != _data->lists.end( ))
    {
        EQWARN << "Requested new list for existing key" << endl;
        return INVALID;
    }

    const GLuint id = glGenLists( num );
    if( !id )
    {
        EQWARN << "glGenLists failed: " << glGetError() << endl;
        return INVALID;
    }
    
    Object& object   = _data->lists[ key ];
    object.id        = id;
    object.num       = num;

    return id;
}

template< typename T >
GLuint ObjectManager<T>::obtainList( const T& key, const GLsizei num )
{
    const GLuint id = getList( key );
    if( id != INVALID )
        return id;
    return newList( key, num );
}

template< typename T >
void   ObjectManager<T>::deleteList( const T& key )
{
    if( _data->lists.find( key ) == _data->lists.end( ))
        return;

    const Object& object = _data->lists[ key ];
    glDeleteLists( object.id, object.num );
    _data->lists.erase( key );
}

// texture object functions

template< typename T >
GLuint ObjectManager<T>::getTexture( const T& key )
{
    if( _data->textures.find( key ) == _data->textures.end( ))
        return INVALID;

    const Object& object = _data->textures[ key ];
    return object.id;
}

template< typename T >
GLuint ObjectManager<T>::newTexture( const T& key )
{
    if( _data->textures.find( key ) != _data->textures.end( ))
    {
        EQWARN << "Requested new texture for existing key" << endl;
        return INVALID;
    }

    GLuint id = INVALID;
    glGenTextures( 1, &id );
    if( id == INVALID )
    {
        EQWARN << "glGenTextures failed: " << glGetError() << endl;
        return INVALID;
    }
    
    Object& object   = _data->textures[ key ];
    object.id        = id;
    return id;
}

template< typename T >
GLuint ObjectManager<T>::obtainTexture( const T& key )
{
    const GLuint id = getTexture( key );
    if( id != INVALID )
        return id;
    return newTexture( key );
}

template< typename T >
void   ObjectManager<T>::deleteTexture( const T& key )
{
    if( _data->textures.find( key ) == _data->textures.end( ))
        return;

    const Object& object = _data->textures[ key ];
    glDeleteTextures( 1, &object.id );
    _data->textures.erase( key );
}

// buffer object functions

template< typename T >
bool ObjectManager<T>::supportsBuffers() const
{
    return ( GLEW_VERSION_1_5 );
}

template< typename T >
GLuint ObjectManager<T>::getBuffer( const T& key )
{
    if( _data->buffers.find( key ) == _data->buffers.end() )
        return INVALID;

    const Object& object = _data->buffers[ key ];
    return object.id;
}

template< typename T >
GLuint ObjectManager<T>::newBuffer( const T& key )
{
    if( !GLEW_VERSION_1_5 )
    {
        EQWARN << "glGenBuffers not available" << endl;
        return INVALID;
    }

    if( _data->buffers.find( key ) != _data->buffers.end() )
    {
        EQWARN << "Requested new buffer for existing key" << endl;
        return INVALID;
    }

    GLuint id = INVALID;
    glGenBuffers( 1, &id );

    if( id == INVALID )
    {
        EQWARN << "glGenBuffers failed: " << glGetError() << endl;
        return INVALID;
    }
    
    Object& object     = _data->buffers[ key ];
    object.id          = id;
    return id;
}

template< typename T >
GLuint ObjectManager<T>::obtainBuffer( const T& key )
{
    const GLuint id = getBuffer( key );
    if( id != INVALID )
        return id;
    return newBuffer( key );
}

template< typename T >
void ObjectManager<T>::deleteBuffer( const T& key )
{
    if( _data->buffers.find( key ) == _data->buffers.end() )
        return;

    const Object& object = _data->buffers[ key ];
    glDeleteBuffers( 1, &object.id );
    _data->buffers.erase( key );
}

// program object functions

template< typename T >
bool ObjectManager<T>::supportsPrograms() const
{
    return ( GLEW_VERSION_2_0 );
}

template< typename T >
GLuint ObjectManager<T>::getProgram( const T& key )
{
    if( _data->programs.find( key ) == _data->programs.end() )
        return INVALID;

    const Object& object = _data->programs[ key ];
    return object.id;
}

template< typename T >
GLuint ObjectManager<T>::newProgram( const T& key )
{
    if( !GLEW_VERSION_2_0 )
    {
        EQWARN << "glCreateProgram not available" << endl;
        return INVALID;
    }

    if( _data->programs.find( key ) != _data->programs.end() )
    {
        EQWARN << "Requested new program for existing key" << endl;
        return INVALID;
    }

    const GLuint id = glCreateProgram();
    if( !id )
    {
        EQWARN << "glCreateProgram failed: " << glGetError() << endl;
        return INVALID;
    }
    
    Object& object     = _data->programs[ key ];
    object.id          = id;
    return id;
}

template< typename T >
GLuint ObjectManager<T>::obtainProgram( const T& key )
{
    const GLuint id = getProgram( key );
    if( id != INVALID )
        return id;
    return newProgram( key );
}

template< typename T >
void ObjectManager<T>::deleteProgram( const T& key )
{
    if( _data->programs.find( key ) == _data->programs.end() )
        return;

    const Object& object = _data->programs[ key ];
    glDeleteProgram( object.id );
    _data->programs.erase( key );
}

// shader object functions

template< typename T >
bool ObjectManager<T>::supportsShaders() const
{
    return ( GLEW_VERSION_2_0 );
}

template< typename T >
GLuint ObjectManager<T>::getShader( const T& key )
{
    if( _data->shaders.find( key ) == _data->shaders.end() )
        return INVALID;

    const Object& object = _data->shaders[ key ];
    return object.id;
}

template< typename T >
GLuint ObjectManager<T>::newShader( const T& key, const GLenum type )
{
    if( !GLEW_VERSION_2_0 )
    {
        EQWARN << "glCreateShader not available" << endl;
        return INVALID;
    }

    if( _data->shaders.find( key ) != _data->shaders.end() )
    {
        EQWARN << "Requested new shader for existing key" << endl;
        return INVALID;
    }

    const GLuint id = glCreateShader( type );
    if( !id )
    {
        EQWARN << "glCreateShader failed: " << glGetError() << endl;
        return INVALID;
    }

    
    Object& object     = _data->shaders[ key ];
    object.id          = id;
    return id;
}

template< typename T >
GLuint ObjectManager<T>::obtainShader( const T& key, const GLenum type )
{
    const GLuint id = getShader( key );
    if( id != INVALID )
        return id;
    return newShader( key, type );
}

template< typename T >
void ObjectManager<T>::deleteShader( const T& key )
{
    if( _data->shaders.find( key ) == _data->shaders.end() )
        return;

    const Object& object = _data->shaders[ key ];
    glDeleteShader( object.id );
    _data->shaders.erase( key );
}

// eq::Texture object functions
template< typename T >
bool ObjectManager<T>::supportsEqTexture() const
{
    return (GLEW_ARB_texture_rectangle);
}

template< typename T >
Texture* ObjectManager<T>::getEqTexture( const T& key )
{
    if( _data->eqTextures.find( key ) == _data->eqTextures.end( ))
        return 0;

    return _data->eqTextures[ key ];
}

template< typename T >
Texture* ObjectManager<T>::newEqTexture( const T& key )
{
    if( _data->eqTextures.find( key ) != _data->eqTextures.end( ))
    {
        EQWARN << "Requested new eqTexture for existing key" << endl;
        return 0;
    }

    Texture* texture = new Texture( _glewContext );
    _data->eqTextures[ key ] = texture;
    return texture;
}

template< typename T >
Texture* ObjectManager<T>::obtainEqTexture( const T& key )
{
    Texture* texture = getEqTexture( key );
    if( texture )
        return texture;
    return newEqTexture( key );
}

template< typename T >
void   ObjectManager<T>::deleteEqTexture( const T& key )
{
    if( _data->eqTextures.find( key ) == _data->eqTextures.end( ))
        return;

    Texture* texture = _data->eqTextures[ key ];
    _data->eqTextures.erase( key );

    texture->flush();
    delete texture;
}

// eq::FrameBufferObject object functions
template< typename T >
bool ObjectManager<T>::supportsEqFrameBufferObject() const
{
    return (GLEW_EXT_framebuffer_object);
}

template< typename T >
FrameBufferObject* ObjectManager<T>::getEqFrameBufferObject( const T& key )
{
    if( _data->eqFrameBufferObjects.find( key ) == _data->eqFrameBufferObjects.end( ))
        return 0;

    return _data->eqFrameBufferObjects[ key ];
}

template< typename T >
FrameBufferObject* ObjectManager<T>::newEqFrameBufferObject( const T& key )
{
    if( _data->eqFrameBufferObjects.find( key ) != _data->eqFrameBufferObjects.end( ))
    {
        EQWARN << "Requested new eqFrameBufferObject for existing key" << endl;
        return 0;
    }

    FrameBufferObject* frameBufferObject = new FrameBufferObject( _glewContext );
    _data->eqFrameBufferObjects[ key ] = frameBufferObject;
    return frameBufferObject;
}

template< typename T >
FrameBufferObject* ObjectManager<T>::obtainEqFrameBufferObject( const T& key )
{
    FrameBufferObject* frameBufferObject = getEqFrameBufferObject( key );
    if( frameBufferObject )
        return frameBufferObject;
    return newEqFrameBufferObject( key );
}

template< typename T >
void ObjectManager<T>::deleteEqFrameBufferObject( const T& key )
{
    if( _data->eqFrameBufferObjects.find( key ) == _data->eqFrameBufferObjects.end( ))
        return;

    FrameBufferObject* frameBufferObject = _data->eqFrameBufferObjects[ key ];
    _data->eqFrameBufferObjects.erase( key );

    frameBufferObject->exit();
    delete frameBufferObject;
}

// instantiate desired key types
//   Instantiation has to be explicit to have all instantiations in the client
//   library, due to the way the heap is handled with Win32 dll's.
template class ObjectManager< const void* >;
