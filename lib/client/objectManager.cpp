
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

// instantiate desired key types
//   Instantiation has to be explicit to have all instantiations in the client
//   library, due to the way the heap is handled with Win32 dll's.
template class ObjectManager< const void* >;
