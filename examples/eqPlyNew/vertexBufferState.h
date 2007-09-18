/*  
    vertexBufferState.h
    Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
    All rights reserved.  
    
    Header file of the VertexBufferState class.
*/


#ifndef MESH_VERTEXBUFFERSTATE_H
#define MESH_VERTEXBUFFERSTATE_H


#include "typedefs.h"
#include <map>

#ifdef EQUALIZER
#   include <eq/client/objectManager.h>
#endif // EQUALIZER


namespace mesh 
{
    
    
    /*  The abstract base class for kd-tree rendering state.  */
    class VertexBufferState
    {
    public:
        virtual bool useColors() const { return _useColors; }
        virtual void setColors( const bool colors ) { _useColors = colors; }
        virtual RenderMode getRenderMode() const { return _renderMode; }
        virtual void setRenderMode( const RenderMode mode ) 
        { 
            _renderMode = mode;
        }
        
        virtual GLuint getDisplayList( const void* key ) = 0;
        virtual GLuint newDisplayList( const void* key ) = 0;
#ifdef GL_ARB_vertex_buffer_object
        virtual GLuint getBufferObject( const void* key ) = 0;
        virtual GLuint newBufferObject( const void* key ) = 0;
#endif // GL_ARB_vertex_buffer_object
        
        virtual const GLFunctions* getGLFunctions() const 
        { 
            return _glFunctions; 
        }
        
    protected:
        VertexBufferState( const GLFunctions* glFunctions ) 
            : _glFunctions( glFunctions ), _useColors( false ), 
              _renderMode( DISPLAY_LIST_MODE ) 
        {
            MESHASSERT( glFunctions );
        } 
        
        virtual ~VertexBufferState() {}
        
        const GLFunctions*  _glFunctions;
        bool                _useColors;
        RenderMode          _renderMode;
        
    private:
    };
    
    
    /*  Simple state for stand-alone usage.  */
    class VertexBufferStateSimple : public VertexBufferState 
    {
    public:
        VertexBufferStateSimple( const GLFunctions* glFunctions )
            : VertexBufferState( glFunctions ) {}
        
        virtual GLuint getDisplayList( const void* key )
        {
            if( _displayLists.find( key ) == _displayLists.end() )
                return 0;
            else
                return _displayLists[key];
        }
        
        virtual GLuint newDisplayList( const void* key )
        {
            _displayLists[key] = glGenLists( 1 );
            return _displayLists[key];
        }
        
#ifdef GL_ARB_vertex_buffer_object
        virtual GLuint getBufferObject( const void* key )
        {
            if( _bufferObjects.find( key ) == _bufferObjects.end() )
                return 0;
            else
                return _bufferObjects[key];
        }
        
        virtual GLuint newBufferObject( const void* key )
        {
            _glFunctions->genBuffers( 1, &_bufferObjects[key] );
            return _bufferObjects[key];
        }
#endif // GL_ARB_vertex_buffer_object
        
    private:
        std::map< const void*, GLuint >  _displayLists;
#ifdef GL_ARB_vertex_buffer_object
        std::map< const void*, GLuint >  _bufferObjects;
#endif // GL_ARB_vertex_buffer_object
    };
    
    
#ifdef EQUALIZER
    /*  State for Equalizer usage, uses EQ Object Manager.  */
    class VertexBufferStateOM : public VertexBufferState 
    {
    public:
        VertexBufferStateOM( const eq::GLFunctions* glFunctions, 
                             eq::ObjectManager< const void* >& om ) 
            : VertexBufferState( glFunctions ), _objectManager( om ) {} 
        
        virtual GLuint getDisplayList( const void* key )
        {
            GLuint i = _objectManager.getList( key );
            return ( i == eq::ObjectManager< const void* >::FAILED ? 0 : i );
        }
        
        virtual GLuint newDisplayList( const void* key )
        {
            GLuint i = _objectManager.newList( key );
            return ( i == eq::ObjectManager< const void* >::FAILED ? 0 : i );
        }
        
#ifdef GL_ARB_vertex_buffer_object
        virtual GLuint getBufferObject( const void* key )
        {
            GLuint i = _objectManager.getBuffer( key );
            return ( i == eq::ObjectManager< const void* >::FAILED ? 0 : i );
        }
        
        virtual GLuint newBufferObject( const void* key )
        {
            GLuint i = _objectManager.newBuffer( key );
            return ( i == eq::ObjectManager< const void* >::FAILED ? 0 : i );
        }
#endif // GL_ARB_vertex_buffer_object
        
    private:
        eq::ObjectManager< const void* >&   _objectManager;
    };
#endif // EQUALIZER
    
    
}


#endif // MESH_VERTEXBUFFERSTATE_H
