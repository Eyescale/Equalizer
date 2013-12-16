
/* Copyright (c) 2009-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2007, Tobias Wolf <twolf@access.unizh.ch>
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

#ifndef EQPLY_VERTEXBUFFERSTATE_H
#define EQPLY_VERTEXBUFFERSTATE_H

#include "channel.h"

#include <ply/vertexBufferState.h>
#include <eq/eq.h>

namespace eqPly
{
/*  State for Equalizer usage, uses Eq's Object Manager.  */
class VertexBufferState : public ply::VertexBufferState
{
public:
    VertexBufferState( eq::util::ObjectManager& objectManager )
        : ply::VertexBufferState( objectManager.glewGetContext( ))
        , _objectManager( objectManager )
        , _channel( 0 )
    {}

    virtual ~VertexBufferState() {};

    virtual GLuint getDisplayList( const void* key )
        { return _objectManager.getList( key ); }

    virtual GLuint newDisplayList( const void* key )
        { return _objectManager.newList( key ); }

    virtual GLuint getTexture( const void* key )
        { return _objectManager.getTexture( key ); }

    virtual GLuint newTexture( const void* key )
        { return _objectManager.newTexture( key ); }

    virtual GLuint getBufferObject( const void* key )
        { return _objectManager.getBuffer( key ); }

    virtual GLuint newBufferObject( const void* key )
        { return _objectManager.newBuffer( key ); }

    virtual GLuint getProgram( const void* key )
        { return _objectManager.getProgram( key ); }

    virtual GLuint newProgram( const void* key )
        { return _objectManager.newProgram( key ); }

    virtual GLuint getShader( const void* key )
        { return _objectManager.getShader( key ); }

    virtual GLuint newShader( const void* key, GLenum type )
        { return _objectManager.newShader( key, type ); }

    virtual void deleteAll() { _objectManager.deleteAll(); }
    bool isShared() const { return _objectManager.isShared(); }

    void setChannel( Channel* channel ) { _channel = channel; }

    virtual bool stopRendering( ) const
        { return _channel ? _channel->stopRendering() : false; }

    virtual void declareRegion( const ply::Vector4f& region )
        { if( _channel ) _channel->declareRegion( eq::Viewport( region )); }

private:
    eq::util::ObjectManager& _objectManager;
    Channel* _channel;
};
} // namespace eqPly

#endif // EQ_MESH_VERTEXBUFFERSTATE_H
