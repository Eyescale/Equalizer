
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "objectManager.h"
#include <string.h>

using namespace eq;
using namespace std;
using namespace stde;

// instantiate desired key types -- see end of file

template< typename T >
ObjectManager<T>::ObjectManager()
        : _buffersSupported( false ),
          _glGenBuffersARB( 0 ),
          _glDeleteBuffersARB( 0 )
{
    // Do not test for GL extensions here, we may have no GL context.
}

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

#ifdef GL_ARB_vertex_buffer_object
    if( !_buffersID.empty() )
        EQWARN << _buffersID.size() 
               << " buffers still allocated in ObjectManager destructor" 
               << endl;
    _buffersID.clear();
    _buffersKey.clear();
#endif // GL_ARB_vertex_buffer_object
}

// helper function to check for specific OpenGL extensions
bool checkExtension( char* extensionName )
{
    // get the list of supported extensions
    const char* extensionList = 
        reinterpret_cast<const char*>( glGetString( GL_EXTENSIONS ) );

    if( !extensionName || !extensionList )
        return false;

    while( *extensionList )
    {
        // find the length of the first extension substring
        size_t firstExtensionLength = strcspn( extensionList, " " );

        if( strlen( extensionName ) == firstExtensionLength &&
            strncmp( extensionName, extensionList, firstExtensionLength ) == 0 )
        {
            return true;
		}

        // move to the next substring
        extensionList += firstExtensionLength + 1;
    }

    return false;
}

template< typename T >
void ObjectManager<T>::init()
{
    // test for OpenGL extension support
#ifdef GL_ARB_vertex_buffer_object
    if( checkExtension( "GL_ARB_vertex_buffer_object" ) )
	{
#   ifdef WIN32
	    _glGenBuffersARB = reinterpret_cast<PFNGLGENBUFFERSPROC>
            ( wglGetProcAddress( "glGenBuffersARB" ) );
	    _glDeleteBuffersARB = reinterpret_cast<PFNGLDELETEBUFFERSPROC>
		    ( wglGetProcAddress( "glDeleteBuffersARB" ) );
#   else
        _glGenBuffersARB = &glGenBuffersARB;
        _glDeleteBuffersARB = &glDeleteBuffersARB;
#   endif // WIN32

        // make sure we have valid pointers for all functions
        if( _glGenBuffersARB && _glDeleteBuffersARB )
        {
            EQINFO << "Succeeded acquiring pointers for glGenBuffersARB and "
                   << "glDeleteBuffersARB, enabling buffer object support"
                   << endl;
            _buffersSupported = true;
        } else {
            EQWARN << "Failed to acquire pointers for glGenBuffersARB and "
                   << "glDeleteBuffersARB, disabling buffer object support"
                   << endl;
            _buffersSupported = false;
        }
    }
#endif // GL_ARB_vertex_buffer_object
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

#ifdef GL_ARB_vertex_buffer_object
    for( typename ObjectIDHash::const_iterator i = _buffersID.begin(); 
         i != _buffersID.end(); ++i )
    {
        const Object& object = i->second;
        EQVERB << "Delete buffer " << object.key << " id " << object.id
               << " ref " << object.refCount << endl;
        if( _buffersSupported )
            _glDeleteBuffersARB( 1, &object.id ); 
    }
    _buffersID.clear();
    _buffersKey.clear();
#endif // GL_ARB_vertex_buffer_object
}

// display list functions

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

// texture object functions

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

// buffer object functions
#ifdef GL_ARB_vertex_buffer_object

template< typename T >
GLuint ObjectManager<T>::getBuffer( const T& key )
{
    if( !_buffersSupported )
        return 0;

    if( _buffersKey.find( key ) == _buffersKey.end() )
        return 0;

    Object* object = _buffersKey[ key ];
    ++object->refCount;
    return object->id;
}

template< typename T >
GLuint ObjectManager<T>::newBuffer( const T& key )
{
    if( !_buffersSupported )
        return 0;

    if( _buffersKey.find( key ) != _buffersKey.end() )
    {
        EQWARN << "Requested new buffer for existing key" << endl;
        return 0;
    }

    GLuint id = 0;
    _glGenBuffersARB( 1, &id );
    if( !id )
    {
        EQWARN << "glGenBuffers failed: " << glGetError() << endl;
        return 0;
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
    if( !_buffersSupported )
        return 0;

    const GLuint id = getBuffer( key );
    if( id )
        return id;
    return newBuffer( key );
}

template< typename T >
void ObjectManager<T>::releaseBuffer( const T& key )
{
    if( !_buffersSupported )
        return;

    if( _buffersKey.find( key ) == _buffersKey.end() )
        return;

    Object* object = _buffersKey[ key ];
    --object->refCount;
    if( object->refCount )
        return;

    _glDeleteBuffersARB( 1, &object->id );
    _buffersKey.erase( key );
    _buffersID.erase( object->id );
}

template< typename T >
void ObjectManager<T>::releaseBuffer( const GLuint id )
{
    if( !_buffersSupported )
        return;

    if( _buffersID.find( id ) == _buffersID.end() )
        return;

    Object& object = _buffersID[ id ];
    --object.refCount;
    if( object.refCount )
        return;

    _glDeleteBuffersARB( 1, &id );
    _buffersKey.erase( object.key );
    _buffersID.erase( id );
}

template< typename T >
void ObjectManager<T>::deleteBuffer( const T& key )
{
    if( !_buffersSupported )
        return;

    if( _buffersKey.find( key ) == _buffersKey.end() )
        return;

    Object* object = _buffersKey[ key ];
    _glDeleteBuffersARB( 1, &object->id );
    _buffersKey.erase( key );
    _buffersID.erase( object->id );
}

template< typename T >
void ObjectManager<T>::deleteBuffer( const GLuint id )
{
    if( !_buffersSupported )
        return;

    if( _buffersID.find( id ) == _buffersID.end() )
        return;

    Object& object = _buffersID[ id ];
    _glDeleteBuffersARB( 1, &id );
    _buffersKey.erase( object.key );
    _buffersID.erase( id );
}

#endif // GL_ARB_vertex_buffer_object

// instantiate desired key types
//   Instantiation has to be explicit to have all instantiations in the client
//   library, due to the way the heap is handled with Win32 dll's.
template class ObjectManager< const void* >;
