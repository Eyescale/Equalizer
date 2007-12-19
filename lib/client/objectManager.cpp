
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
    if( !_listsID.empty( ))
        EQWARN << _listsID.size() 
               << " lists still allocated in ObjectManager destructor" << endl;
    _listsID.clear();
    _listsKey.clear();

    if( !_texturesID.empty( ))
        EQWARN << _texturesID.size() 
               << " textures still allocated in ObjectManager destructor" 
               << endl;
    _texturesID.clear();
    _texturesKey.clear();

    if( !_buffersID.empty( ))
        EQWARN << _buffersID.size() 
               << " buffers still allocated in ObjectManager destructor" 
               << endl;
    _buffersID.clear();
    _buffersKey.clear();

    if( !_programsID.empty( ))
        EQWARN << _programsID.size() 
               << " programs still allocated in ObjectManager destructor" 
               << endl;
    _programsID.clear();
    _programsKey.clear();

    if( !_shadersID.empty( ))
        EQWARN << _shadersID.size() 
               << " shaders still allocated in ObjectManager destructor" 
               << endl;
    _shadersID.clear();
    _shadersKey.clear();
}

template< typename T >
void ObjectManager<T>::deleteAll()
{
   for( typename ObjectIDHash::const_iterator i = _listsID.begin(); 
         i != _listsID.end(); ++i )
    {
        const Object& object = i->second;
        EQVERB << "Delete list " << object.key << " id " << object.id
               << " ref " << object.refCount << endl;
        glDeleteLists( object.id, 1 ); 
    }
    _listsID.clear();
    _listsKey.clear();

    for( typename ObjectIDHash::const_iterator i = _texturesID.begin(); 
         i != _texturesID.end(); ++i )
    {
        const Object& object = i->second;
        EQVERB << "Delete texture " << object.key << " id " << object.id
               << " ref " << object.refCount << endl;
        glDeleteTextures( 1, &object.id ); 
    }
    _texturesID.clear();
    _texturesKey.clear();

    for( typename ObjectIDHash::const_iterator i = _buffersID.begin(); 
         i != _buffersID.end(); ++i )
    {
        const Object& object = i->second;
        EQVERB << "Delete buffer " << object.key << " id " << object.id
               << " ref " << object.refCount << endl;
        glDeleteBuffers( 1, &object.id ); 
    }
    _buffersID.clear();
    _buffersKey.clear();

    for( typename ObjectIDHash::const_iterator i = _programsID.begin(); 
         i != _programsID.end(); ++i )
    {
        const Object& object = i->second;
        EQVERB << "Delete program " << object.key << " id " << object.id
               << " ref " << object.refCount << endl;
        glDeleteProgram( object.id ); 
    }
    _programsID.clear();
    _programsKey.clear();

    for( typename ObjectIDHash::const_iterator i = _shadersID.begin(); 
         i != _shadersID.end(); ++i )
    {
        const Object& object = i->second;
        EQVERB << "Delete shader " << object.key << " id " << object.id
               << " ref " << object.refCount << endl;
        glDeleteShader( object.id ); 
    }
    _shadersID.clear();
    _shadersKey.clear();
}

// display list functions

template< typename T >
GLuint ObjectManager<T>::getList( const T& key )
{
    if( _listsKey.find( key ) == _listsKey.end( ))
        return FAILED;

    Object* object = _listsKey[ key ];
    ++object->refCount;
    return object->id;
}

template< typename T >
GLuint ObjectManager<T>::newList( const T& key )
{
    if( _listsKey.find( key ) != _listsKey.end( ))
    {
        EQWARN << "Requested new list for existing key" << endl;
        return FAILED;
    }

    const GLuint id = glGenLists( 1 );
    if( !id )
    {
        EQWARN << "glGenLists failed: " << glGetError() << endl;
        return FAILED;
    }
    
    Object& object   = _listsID[ id ];
    object.id        = id;
    object.key       = key;
    object.refCount  = 1;
    _listsKey[ key ] = &object;

    return id;
}

template< typename T >
GLuint ObjectManager<T>::obtainList( const T& key )
{
    const GLuint id = getList( key );
    if( id != FAILED )
        return id;
    return newList( key );
}

template< typename T >
void   ObjectManager<T>::releaseList( const T& key )
{
    if( _listsKey.find( key ) == _listsKey.end( ))
        return;

    Object* object = _listsKey[ key ];
    --object->refCount;
    if( object->refCount )
        return;

    glDeleteLists( object->id, 1 );
    _listsKey.erase( key );
    _listsID.erase( object->id );
}

template< typename T >
void   ObjectManager<T>::releaseList( const GLuint id )
{
    if( _listsID.find( id ) == _listsID.end( ))
        return;

    Object& object = _listsID[ id ];
    --object.refCount;
    if( object.refCount )
        return;

    glDeleteLists( id, 1 );
    _listsKey.erase( object.key );
    _listsID.erase( id );
}

template< typename T >
void   ObjectManager<T>::deleteList( const T& key )
{
    if( _listsKey.find( key ) == _listsKey.end( ))
        return;

    Object* object = _listsKey[ key ];
    glDeleteLists( object->id, 1 );
    _listsKey.erase( key );
    _listsID.erase( object->id );
}

template< typename T >
void   ObjectManager<T>::deleteList( const GLuint id )
{
    if( _listsID.find( id ) == _listsID.end( ))
        return;

    Object& object = _listsID[ id ];
    glDeleteLists( id, 1 );
    _listsKey.erase( object.key );
    _listsID.erase( id );
}

// texture object functions

template< typename T >
GLuint ObjectManager<T>::getTexture( const T& key )
{
    if( _texturesKey.find( key ) == _texturesKey.end( ))
        return FAILED;

    Object* object = _texturesKey[ key ];
    ++object->refCount;
    return object->id;
}

template< typename T >
GLuint ObjectManager<T>::newTexture( const T& key )
{
    if( _texturesKey.find( key ) != _texturesKey.end( ))
    {
        EQWARN << "Requested new texture for existing key" << endl;
        return FAILED;
    }

    GLuint id = FAILED;
    glGenTextures( 1, &id );
    if( id == FAILED )
    {
        EQWARN << "glGenTextures failed: " << glGetError() << endl;
        return FAILED;
    }
    
    Object& object   = _texturesID[ id ];
    object.id        = id;
    object.key       = key;
    object.refCount  = 1;
    _texturesKey[ key ] = &object;

    return id;
}

template< typename T >
GLuint ObjectManager<T>::obtainTexture( const T& key )
{
    const GLuint id = getTexture( key );
    if( id != FAILED )
        return id;
    return newTexture( key );
}

template< typename T >
void   ObjectManager<T>::releaseTexture( const T& key )
{
    if( _texturesKey.find( key ) == _texturesKey.end( ))
        return;

    Object* object = _texturesKey[ key ];
    --object->refCount;
    if( object->refCount )
        return;

    glDeleteTextures( 1, &object->id );
    _texturesKey.erase( key );
    _texturesID.erase( object->id );
}

template< typename T >
void   ObjectManager<T>::releaseTexture( const GLuint id )
{
    if( _texturesID.find( id ) == _texturesID.end( ))
        return;

    Object& object = _texturesID[ id ];
    --object.refCount;
    if( object.refCount )
        return;

    glDeleteTextures( 1, &id );
    _texturesKey.erase( object.key );
    _texturesID.erase( id );
}

template< typename T >
void   ObjectManager<T>::deleteTexture( const T& key )
{
    if( _texturesKey.find( key ) == _texturesKey.end( ))
        return;

    Object* object = _texturesKey[ key ];
    glDeleteTextures( 1, &object->id );
    _texturesKey.erase( key );
    _texturesID.erase( object->id );
}

template< typename T >
void   ObjectManager<T>::deleteTexture( const GLuint id )
{
    if( _texturesID.find( id ) == _texturesID.end( ))
        return;

    Object& object = _texturesID[ id ];
    glDeleteTextures( 1, &id );
    _texturesKey.erase( object.key );
    _texturesID.erase( id );
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
    if( _buffersKey.find( key ) == _buffersKey.end() )
        return FAILED;

    Object* object = _buffersKey[ key ];
    ++object->refCount;
    return object->id;
}

template< typename T >
GLuint ObjectManager<T>::newBuffer( const T& key )
{
    if( !GLEW_VERSION_1_5 )
    {
        EQWARN << "glGenBuffers not available" << endl;
        return FAILED;
    }

    if( _buffersKey.find( key ) != _buffersKey.end() )
    {
        EQWARN << "Requested new buffer for existing key" << endl;
        return FAILED;
    }

    GLuint id = FAILED;
    glGenBuffers( 1, &id );

    if( id == FAILED )
    {
        EQWARN << "glGenBuffers failed: " << glGetError() << endl;
        return FAILED;
    }
    
    Object& object     = _buffersID[ id ];
    object.id          = id;
    object.key         = key;
    object.refCount    = 1;
    _buffersKey[ key ] = &object;

    return id;
}

template< typename T >
GLuint ObjectManager<T>::obtainBuffer( const T& key )
{
    const GLuint id = getBuffer( key );
    if( id != FAILED )
        return id;
    return newBuffer( key );
}

template< typename T >
void ObjectManager<T>::releaseBuffer( const T& key )
{
    if( _buffersKey.find( key ) == _buffersKey.end() )
        return;

    Object* object = _buffersKey[ key ];
    --object->refCount;
    if( object->refCount )
        return;

    glDeleteBuffers( 1, &object->id );
    _buffersKey.erase( key );
    _buffersID.erase( object->id );
}

template< typename T >
void ObjectManager<T>::releaseBuffer( const GLuint id )
{
    if( _buffersID.find( id ) == _buffersID.end() )
        return;

    Object& object = _buffersID[ id ];
    --object.refCount;
    if( object.refCount )
        return;

    glDeleteBuffers( 1, &id );
    _buffersKey.erase( object.key );
    _buffersID.erase( id );
}

template< typename T >
void ObjectManager<T>::deleteBuffer( const T& key )
{
    if( _buffersKey.find( key ) == _buffersKey.end() )
        return;

    Object* object = _buffersKey[ key ];
    glDeleteBuffers( 1, &object->id );
    _buffersKey.erase( key );
    _buffersID.erase( object->id );
}

template< typename T >
void ObjectManager<T>::deleteBuffer( const GLuint id )
{
    if( _buffersID.find( id ) == _buffersID.end() )
        return;

    Object& object = _buffersID[ id ];
    glDeleteBuffers( 1, &id );
    _buffersKey.erase( object.key );
    _buffersID.erase( id );
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
    if( _programsKey.find( key ) == _programsKey.end() )
        return FAILED;

    Object* object = _programsKey[ key ];
    ++object->refCount;
    return object->id;
}

template< typename T >
GLuint ObjectManager<T>::newProgram( const T& key )
{
    if( !GLEW_VERSION_2_0 )
    {
        EQWARN << "glCreateProgram not available" << endl;
        return FAILED;
    }

    if( _programsKey.find( key ) != _programsKey.end() )
    {
        EQWARN << "Requested new program for existing key" << endl;
        return FAILED;
    }

    const GLuint id = glCreateProgram();
    if( !id )
    {
        EQWARN << "glCreateProgram failed: " << glGetError() << endl;
        return FAILED;
    }
    
    Object& object     = _programsID[ id ];
    object.id          = id;
    object.key         = key;
    object.refCount    = 1;
    _programsKey[ key ] = &object;

    return id;
}

template< typename T >
GLuint ObjectManager<T>::obtainProgram( const T& key )
{
    const GLuint id = getProgram( key );
    if( id != FAILED )
        return id;
    return newProgram( key );
}

template< typename T >
void ObjectManager<T>::releaseProgram( const T& key )
{
    if( _programsKey.find( key ) == _programsKey.end() )
        return;

    Object* object = _programsKey[ key ];
    --object->refCount;
    if( object->refCount )
        return;

    glDeleteProgram( object->id );
    _programsKey.erase( key );
    _programsID.erase( object->id );
}

template< typename T >
void ObjectManager<T>::releaseProgram( const GLuint id )
{
    if( _programsID.find( id ) == _programsID.end() )
        return;

    Object& object = _programsID[ id ];
    --object.refCount;
    if( object.refCount )
        return;

    glDeleteProgram( id );
    _programsKey.erase( object.key );
    _programsID.erase( id );
}

template< typename T >
void ObjectManager<T>::deleteProgram( const T& key )
{
    if( _programsKey.find( key ) == _programsKey.end() )
        return;

    Object* object = _programsKey[ key ];
    glDeleteProgram( object->id );
    _programsKey.erase( key );
    _programsID.erase( object->id );
}

template< typename T >
void ObjectManager<T>::deleteProgram( const GLuint id )
{
    if( _programsID.find( id ) == _programsID.end() )
        return;

    Object& object = _programsID[ id ];
    glDeleteProgram( id );
    _programsKey.erase( object.key );
    _programsID.erase( id );
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
    if( _shadersKey.find( key ) == _shadersKey.end() )
        return FAILED;

    Object* object = _shadersKey[ key ];
    ++object->refCount;
    return object->id;
}

template< typename T >
GLuint ObjectManager<T>::newShader( const T& key, const GLenum type )
{
    if( !GLEW_VERSION_2_0 )
    {
        EQWARN << "glCreateShader not available" << endl;
        return FAILED;
    }

    if( _shadersKey.find( key ) != _shadersKey.end() )
    {
        EQWARN << "Requested new shader for existing key" << endl;
        return FAILED;
    }

    const GLuint id = glCreateShader( type );
    if( !id )
    {
        EQWARN << "glCreateShader failed: " << glGetError() << endl;
        return FAILED;
    }

    
    Object& object     = _shadersID[ id ];
    object.id          = id;
    object.key         = key;
    object.refCount    = 1;
    _shadersKey[ key ] = &object;

    return id;
}

template< typename T >
GLuint ObjectManager<T>::obtainShader( const T& key, const GLenum type )
{
    const GLuint id = getShader( key );
    if( id != FAILED )
        return id;
    return newShader( key, type );
}

template< typename T >
void ObjectManager<T>::releaseShader( const T& key )
{
    if( _shadersKey.find( key ) == _shadersKey.end() )
        return;

    Object* object = _shadersKey[ key ];
    --object->refCount;
    if( object->refCount )
        return;

    glDeleteShader( object->id );
    _shadersKey.erase( key );
    _shadersID.erase( object->id );
}

template< typename T >
void ObjectManager<T>::releaseShader( const GLuint id )
{
    if( _shadersID.find( id ) == _shadersID.end() )
        return;

    Object& object = _shadersID[ id ];
    --object.refCount;
    if( object.refCount )
        return;

    glDeleteShader( id );
    _shadersKey.erase( object.key );
    _shadersID.erase( id );
}

template< typename T >
void ObjectManager<T>::deleteShader( const T& key )
{
    if( _shadersKey.find( key ) == _shadersKey.end() )
        return;

    Object* object = _shadersKey[ key ];
    glDeleteShader( object->id );
    _shadersKey.erase( key );
    _shadersID.erase( object->id );
}

template< typename T >
void ObjectManager<T>::deleteShader( const GLuint id )
{
    if( _shadersID.find( id ) == _shadersID.end() )
        return;

    Object& object = _shadersID[ id ];
    glDeleteShader( id );
    _shadersKey.erase( object.key );
    _shadersID.erase( id );
}

// instantiate desired key types
//   Instantiation has to be explicit to have all instantiations in the client
//   library, due to the way the heap is handled with Win32 dll's.
template class ObjectManager< const void* >;
