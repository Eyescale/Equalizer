/*  
    vertexBufferState.h
    Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com>
    Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
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
  
    
    Header file of the VertexBufferState class.
*/


#ifndef MESH_VERTEXBUFFERSTATE_H
#define MESH_VERTEXBUFFERSTATE_H


#include "typedefs.h"
#include <map>

#ifdef EQUALIZER
#   include <eq/eq.h>
#endif // EQUALIZER


namespace mesh 
{
    /*  The abstract base class for kd-tree rendering state.  */
    class VertexBufferState
    {
    public:
        enum
        {
            INVALID = 0 //<! return value for failed operations.
        };

        virtual bool useColors() const { return _useColors; }
        virtual void setColors( const bool colors ) { _useColors = colors; }
        virtual RenderMode getRenderMode() const { return _renderMode; }
        virtual void setRenderMode( const RenderMode mode ) 
        { 
            if( _renderMode == mode )
                return;

            _renderMode = mode;

            // Check if VBO funcs available, else fall back to display lists
            if( _renderMode == RENDER_MODE_BUFFER_OBJECT && !GLEW_VERSION_1_5 )
            {
                MESHINFO << "VBO not available, using display lists"
                         << std::endl;
                _renderMode = RENDER_MODE_DISPLAY_LIST;
            }
        }

        virtual GLuint getDisplayList( const void* key ) = 0;
        virtual GLuint newDisplayList( const void* key ) = 0;
        virtual GLuint getBufferObject( const void* key ) = 0;
        virtual GLuint newBufferObject( const void* key ) = 0;
        
        virtual void deleteAll() = 0;

        GLEWContext* glewGetContext() { return _glewContext; }
        
    protected:
        VertexBufferState( GLEWContext* glewContext ) 
            : _glewContext( glewContext ), _useColors( false ), 
              _renderMode( RENDER_MODE_DISPLAY_LIST ) 
        {
            MESHASSERT( glewContext );
        } 
        
        virtual ~VertexBufferState() {}
        
        GLEWContext*  _glewContext;
        bool          _useColors;
        RenderMode    _renderMode;
        
    private:
    };
    
    
    /*  Simple state for stand-alone single-pipe usage.  */
    class VertexBufferStateSimple : public VertexBufferState 
    {
    public:
        VertexBufferStateSimple( GLEWContext* glewContext )
            : VertexBufferState( glewContext ) {}
        
        virtual GLuint getDisplayList( const void* key )
        {
            if( _displayLists.find( key ) == _displayLists.end() )
                return INVALID;
            return _displayLists[key];
        }
        
        virtual GLuint newDisplayList( const void* key )
        {
            _displayLists[key] = glGenLists( 1 );
            return _displayLists[key];
        }
        
        virtual GLuint getBufferObject( const void* key )
        {
            if( _bufferObjects.find( key ) == _bufferObjects.end() )
                return INVALID;
            return _bufferObjects[key];
        }
        
        virtual GLuint newBufferObject( const void* key )
        {
            if( !GLEW_VERSION_1_5 )
                return INVALID;
            glGenBuffers( 1, &_bufferObjects[key] );
            return _bufferObjects[key];
        }
        
        virtual void deleteAll() { /* NOP, TBD */ }

    private:
        std::map< const void*, GLuint >  _displayLists;
        std::map< const void*, GLuint >  _bufferObjects;
    };
} // namespace mesh

#ifdef EQUALIZER
namespace eqPly
{
    /*  State for Equalizer usage, uses Eq's Object Manager.  */
    class VertexBufferState : public mesh::VertexBufferState 
    {
    public:
        VertexBufferState( eq::Window::ObjectManager* objectManager ) 
                : mesh::VertexBufferState( objectManager->glewGetContext( ))
                , _objectManager( objectManager )
            {} 
        
        virtual GLuint getDisplayList( const void* key )
            { return _objectManager->getList( key ); }
        
        virtual GLuint newDisplayList( const void* key )
            { return _objectManager->newList( key ); }
        
        virtual GLuint getTexture( const void* key )
            { return _objectManager->getTexture( key ); }
        
        virtual GLuint newTexture( const void* key )
            { return _objectManager->newTexture( key ); }
        
        virtual GLuint getBufferObject( const void* key )
            { return _objectManager->getBuffer( key ); }
        
        virtual GLuint newBufferObject( const void* key )
            { return _objectManager->newBuffer( key ); }
        
        virtual GLuint getProgram( const void* key )
            { return _objectManager->getProgram( key ); }
        
        virtual GLuint newProgram( const void* key )
            { return _objectManager->newProgram( key ); }
        
        virtual GLuint getShader( const void* key )
            { return _objectManager->getShader( key ); }
        
        virtual GLuint newShader( const void* key, GLenum type )
            { return _objectManager->newShader( key, type ); }

        virtual void deleteAll() { _objectManager->deleteAll(); }
        
    private:
        eq::Window::ObjectManager* _objectManager;
    };
} // namespace eqPly
#endif // EQUALIZER
    
    

#endif // MESH_VERTEXBUFFERSTATE_H
