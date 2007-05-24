
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "objectManager.h"

using namespace eq;
using namespace std;
using namespace stde;

// instantiate desired key types -- see tail of fail

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
}

template< typename T >
GLuint ObjectManager<T>::getList( const T& key )
{
    if( _listsKey.find( key ) == _listsKey.end( ))
        return 0;

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
        return 0;
    }

    const GLuint id = glGenLists( 1 );
    if( !id )
    {
        EQWARN << "glGenLists failed: " << glGetError() << endl;
        return 0;
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
    if( id )
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

template< typename T >
GLuint ObjectManager<T>::getTexture( const T& key )
{
    if( _texturesKey.find( key ) == _texturesKey.end( ))
        return 0;

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
        return 0;
    }

    GLuint id = 0;
    glGenTextures( 1, &id );
    if( !id )
    {
        EQWARN << "glGenTextures failed: " << glGetError() << endl;
        return 0;
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
    if( id )
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

// instantiate desired key types
//   Instantiation has to be explicit to have all instantiations in the client
//   library, due to the way the heap is handled with Win32 dll's.
template class ObjectManager< const void* >;
