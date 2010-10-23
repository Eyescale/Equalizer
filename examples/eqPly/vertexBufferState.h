
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
  
    
    Header file of the VertexBufferState class.
*/


#ifndef MESH_VERTEXBUFFERSTATE_H
#define MESH_VERTEXBUFFERSTATE_H


#include "typedefs.h"
#include <map>

#ifdef EQUALIZER
#   include <eq/eq.h>
#   include "channel.h"
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
        virtual bool stopRendering() const { return false; }
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

        const GLEWContext* glewGetContext() const { return _glewContext; }
        
    protected:
        VertexBufferState( const GLEWContext* glewContext ) 
            : _glewContext( glewContext ), _useColors( false ), 
              _renderMode( RENDER_MODE_DISPLAY_LIST ) 
        {
            MESHASSERT( glewContext );
        } 
        
        virtual ~VertexBufferState() {}
        
        const GLEWContext* const _glewContext;
        bool          _useColors;
        RenderMode    _renderMode;
        
    private:
    };
    
    
    /*  Simple state for stand-alone single-pipe usage.  */
    class VertexBufferStateSimple : public VertexBufferState 
    {
    public:
        VertexBufferStateSimple( const GLEWContext* glewContext )
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
        bool isShared() const { return _objectManager->isShared(); }
        
        void setChannel( const Channel* channel )
             { _channel = channel; }

        virtual bool stopRendering( ) const
            { EQASSERT( _channel ); return _channel->stopRendering(); }

    private:
        eq::Window::ObjectManager* _objectManager;
        const Channel* _channel;
    };
} // namespace eqPly
#endif // EQUALIZER
    
    

#endif // MESH_VERTEXBUFFERSTATE_H
