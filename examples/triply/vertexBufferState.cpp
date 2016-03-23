
/* Copyright (c) 2011-2016, Stefan Eilemann <eile@eyescale.ch>
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
 */


#include "vertexBufferState.h"

namespace triply
{
VertexBufferState::VertexBufferState( const GLEWContext* glewContext )
        : _glewContext( glewContext )
        , _renderMode( RENDER_MODE_DISPLAY_LIST )
        , _useColors( false )
        , _useFrustumCulling( true )
{
    _range[0] = 0.f;
    _range[1] = 1.f;
    resetRegion();
    PLYLIBASSERT( glewContext );
}

void VertexBufferState::setRenderMode( const RenderMode mode )
{
    if( _renderMode == mode )
        return;

    _renderMode = mode;

    // Check if VBO funcs available, else fall back to display lists
    if( _renderMode == RENDER_MODE_BUFFER_OBJECT && !GLEW_VERSION_1_5 )
    {
        PLYLIBINFO << "VBO not available, using display lists" << std::endl;
        _renderMode = RENDER_MODE_DISPLAY_LIST;
    }
}

void VertexBufferState::resetRegion()
{
    _region[0] = std::numeric_limits< float >::max();
    _region[1] = std::numeric_limits< float >::max();
    _region[2] = -std::numeric_limits< float >::max();
    _region[3] = -std::numeric_limits< float >::max();
}

void VertexBufferState::updateRegion( const BoundingBox& box )
{
    const Vertex corners[8] = { Vertex( box[0][0], box[0][1], box[0][2] ),
                                Vertex( box[1][0], box[0][1], box[0][2] ),
                                Vertex( box[0][0], box[1][1], box[0][2] ),
                                Vertex( box[1][0], box[1][1], box[0][2] ),
                                Vertex( box[0][0], box[0][1], box[1][2] ),
                                Vertex( box[1][0], box[0][1], box[1][2] ),
                                Vertex( box[0][0], box[1][1], box[1][2] ),
                                Vertex( box[1][0], box[1][1], box[1][2] ) };

    Vector4f region(  std::numeric_limits< float >::max(),
                      std::numeric_limits< float >::max(),
                     -std::numeric_limits< float >::max(),
                     -std::numeric_limits< float >::max( ));

    for( size_t i = 0; i < 8; ++i )
    {
        const Vertex corner = _pmvMatrix * corners[i];
        region[0] = std::min( corner[0], region[0] );
        region[1] = std::min( corner[1], region[1] );
        region[2] = std::max( corner[0], region[2] );
        region[3] = std::max( corner[1], region[3] );
    }

    // transform region of interest from [ -1 -1 1 1 ] to normalized viewport
    const Vector4f normalized( region[0] * .5f + .5f,
                               region[1] * .5f + .5f,
                               ( region[2] - region[0] ) * .5f,
                               ( region[3] - region[1] ) * .5f );

    declareRegion( normalized );
    _region[0] = std::min( _region[0], normalized[0] );
    _region[1] = std::min( _region[1], normalized[1] );
    _region[2] = std::max( _region[2], normalized[2] );
    _region[3] = std::max( _region[3], normalized[3] );
}

Vector4f VertexBufferState::getRegion() const
{
    if( _region[0] > _region[2] || _region[1] > _region[3] )
        return Vector4f::ZERO;

    return _region;
}

GLuint VertexBufferStateSimple::getDisplayList( const void* key )
{
    if( _displayLists.find( key ) == _displayLists.end() )
        return INVALID;
    return _displayLists[key];
}

GLuint VertexBufferStateSimple::newDisplayList( const void* key )
{
    _displayLists[key] = glGenLists( 1 );
    return _displayLists[key];
}

GLuint VertexBufferStateSimple::getBufferObject( const void* key )
{
    if( _bufferObjects.find( key ) == _bufferObjects.end() )
        return INVALID;
    return _bufferObjects[key];
}

GLuint VertexBufferStateSimple::newBufferObject( const void* key )
{
    if( !GLEW_VERSION_1_5 )
        return INVALID;
    glGenBuffers( 1, &_bufferObjects[key] );
    return _bufferObjects[key];
}

void VertexBufferStateSimple::deleteAll()
{
    for( GLMapCIter i = _displayLists.begin(); i != _displayLists.end(); ++i )
        glDeleteLists( i->second, 1 );

    for( GLMapCIter i = _bufferObjects.begin(); i != _bufferObjects.end(); ++i )
        glDeleteBuffers( 1, &(i->second) );

    _displayLists.clear();
    _bufferObjects.clear();
}

}
