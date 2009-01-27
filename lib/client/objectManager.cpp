
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "objectManager.h"
#include <string.h>

using namespace eq;
using namespace std;
using namespace stde;

// instantiate desired key types -- see end of file

template< typename T >
ObjectManager<T>::~ObjectManager()
{
    // Do not delete GL objects, we may have no GL context.
    if( !_lists.empty( ))
        EQWARN << _lists.size() 
               << " lists still allocated in ObjectManager destructor" << endl;
    _lists.clear();

    if( !_textures.empty( ))
        EQWARN << _textures.size() 
               << " textures still allocated in ObjectManager destructor" 
               << endl;
    _textures.clear();

    if( !_buffers.empty( ))
        EQWARN << _buffers.size() 
               << " buffers still allocated in ObjectManager destructor" 
               << endl;
    _buffers.clear();

    if( !_programs.empty( ))
        EQWARN << _programs.size() 
               << " programs still allocated in ObjectManager destructor" 
               << endl;
    _programs.clear();

    if( !_shaders.empty( ))
        EQWARN << _shaders.size() 
               << " shaders still allocated in ObjectManager destructor" 
               << endl;
    _shaders.clear();
}

template< typename T >
void ObjectManager<T>::deleteAll()
{
   for( typename ObjectKeyHash::const_iterator i = _lists.begin(); 
         i != _lists.end(); ++i )
    {
        const Object& object = i->second;
        EQVERB << "Delete list " << object.id << endl;
        glDeleteLists( object.id, object.num ); 
    }
    _lists.clear();

    for( typename ObjectKeyHash::const_iterator i = _textures.begin(); 
         i != _textures.end(); ++i )
    {
        const Object& object = i->second;
        EQVERB << "Delete texture " << object.id << endl;
        glDeleteTextures( 1, &object.id ); 
    }
    _textures.clear();

    for( typename ObjectKeyHash::const_iterator i = _buffers.begin(); 
         i != _buffers.end(); ++i )
    {
        const Object& object = i->second;
        EQVERB << "Delete buffer " << object.id << endl;
        glDeleteBuffers( 1, &object.id ); 
    }
    _buffers.clear();

    for( typename ObjectKeyHash::const_iterator i = _programs.begin(); 
         i != _programs.end(); ++i )
    {
        const Object& object = i->second;
        EQVERB << "Delete program " << object.id << endl;
        glDeleteProgram( object.id ); 
    }
    _programs.clear();

    for( typename ObjectKeyHash::const_iterator i = _shaders.begin(); 
         i != _shaders.end(); ++i )
    {
        const Object& object = i->second;
        EQVERB << "Delete shader " << object.id << endl;
        glDeleteShader( object.id ); 
    }
    _shaders.clear();
}

// display list functions

template< typename T >
GLuint ObjectManager<T>::getList( const T& key )
{
    if( _lists.find( key ) == _lists.end( ))
        return INVALID;

    const Object& object = _lists[ key ];
    return object.id;
}

template< typename T >
GLuint ObjectManager<T>::newList( const T& key, const GLsizei num )
{
    if( _lists.find( key ) != _lists.end( ))
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
    
    Object& object   = _lists[ key ];
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
    if( _lists.find( key ) == _lists.end( ))
        return;

    const Object& object = _lists[ key ];
    glDeleteLists( object.id, object.num );
    _lists.erase( key );
}

// texture object functions

template< typename T >
GLuint ObjectManager<T>::getTexture( const T& key )
{
    if( _textures.find( key ) == _textures.end( ))
        return INVALID;

    const Object& object = _textures[ key ];
    return object.id;
}

template< typename T >
GLuint ObjectManager<T>::newTexture( const T& key )
{
    if( _textures.find( key ) != _textures.end( ))
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
    
    Object& object   = _textures[ key ];
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
    if( _textures.find( key ) == _textures.end( ))
        return;

    const Object& object = _textures[ key ];
    glDeleteTextures( 1, &object.id );
    _textures.erase( key );
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
    if( _buffers.find( key ) == _buffers.end() )
        return INVALID;

    const Object& object = _buffers[ key ];
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

    if( _buffers.find( key ) != _buffers.end() )
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
    
    Object& object     = _buffers[ key ];
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
    if( _buffers.find( key ) == _buffers.end() )
        return;

    const Object& object = _buffers[ key ];
    glDeleteBuffers( 1, &object.id );
    _buffers.erase( key );
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
    if( _programs.find( key ) == _programs.end() )
        return INVALID;

    const Object& object = _programs[ key ];
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

    if( _programs.find( key ) != _programs.end() )
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
    
    Object& object     = _programs[ key ];
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
    if( _programs.find( key ) == _programs.end() )
        return;

    const Object& object = _programs[ key ];
    glDeleteProgram( object.id );
    _programs.erase( key );
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
    if( _shaders.find( key ) == _shaders.end() )
        return INVALID;

    const Object& object = _shaders[ key ];
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

    if( _shaders.find( key ) != _shaders.end() )
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

    
    Object& object     = _shaders[ key ];
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
    if( _shaders.find( key ) == _shaders.end() )
        return;

    const Object& object = _shaders[ key ];
    glDeleteShader( object.id );
    _shaders.erase( key );
}

// instantiate desired key types
//   Instantiation has to be explicit to have all instantiations in the client
//   library, due to the way the heap is handled with Win32 dll's.
template class ObjectManager< const void* >;
