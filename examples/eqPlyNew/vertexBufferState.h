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


namespace mesh 
{
    
    
    /*  The abstract base class for kd-tree rendering state.  */
    class VertexBufferState
    {
    public:
        virtual ~VertexBufferState() {}
        virtual bool hasColors() = 0;
        virtual void setColors( bool colors ) = 0;
        virtual RenderMode getRenderMode() = 0;
        virtual void setRenderMode( RenderMode mode ) = 0;
        virtual Culler* getCuller() = 0;
        virtual void setCuller( Culler& culler ) = 0;
        virtual Range getRange() = 0;
        virtual void setRange( float start, float end ) = 0;
        virtual GLuint getDisplayList( Index i ) = 0;
        virtual GLuint newDisplayList( Index i ) = 0;
        virtual GLuint* getBufferObjects( Index i ) = 0;
        virtual GLuint* newBufferObjects( Index i ) = 0;
    private:
    };
    
    
    /*  Simple state for stand-alone usage.  */
    class VertexBufferStateSimple : public VertexBufferState 
    {
    public:
        VertexBufferStateSimple() : _culler( 0 ), _hasColors( false ), 
                                    _renderMode( IMMEDIATE_MODE ) 
        {
            setRange( 0.0f, 1.0f );
        }
        
        virtual bool hasColors() { return _hasColors; }
        virtual void setColors( bool colors ) { _hasColors = colors; }
        virtual RenderMode getRenderMode() { return _renderMode; }
        virtual void setRenderMode( RenderMode mode ) { _renderMode = mode; }
        virtual Culler* getCuller() { return _culler; }
        virtual void setCuller( Culler& culler ) { _culler = &culler; }
        virtual Range getRange() { return _range; }
        
        virtual void setRange( float start, float end )
        {
            _range[0] = start;
            _range[1] = end;
        }
        
        virtual GLuint getDisplayList( Index i )
        {
            if( _displayLists.find( i ) == _displayLists.end() )
                return 0;
            else
                return _displayLists[i];
        }
        
        virtual GLuint newDisplayList( Index i )
        {
            _displayLists[i] = glGenLists( 1 );
            return _displayLists[i];
        }
        
        virtual GLuint* getBufferObjects( Index i )
        {
            if( _bufferObjects.find( i ) == _bufferObjects.end() )
                return 0;
            else
                return &_bufferObjects[i][0];
        }
        
        virtual GLuint* newBufferObjects( Index i )
        {
            GLuint* buffers = &_bufferObjects[i][0];
            glGenBuffers( 4, buffers );
            return buffers;
        }
        
    private:
        Culler*                                         _culler;
        Range                                           _range;
        bool                                            _hasColors;
        RenderMode                                      _renderMode;
        std::map< Index, GLuint >                       _displayLists;
        std::map< Index, ArrayWrapper< GLuint, 4 > >    _bufferObjects;
    };
    
    
}


#endif // MESH_VERTEXBUFFERSTATE_H
