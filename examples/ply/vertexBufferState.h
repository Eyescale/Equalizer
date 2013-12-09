
/* Copyright (c) 2009-2012, Stefan Eilemann <eile@equalizergraphics.com>
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


#ifndef PLYLIB_VERTEXBUFFERSTATE_H
#define PLYLIB_VERTEXBUFFERSTATE_H

#include "api.h"
#include "typedefs.h"
#include <map>

namespace ply
{
/*  The abstract base class for kd-tree rendering state.  */
class VertexBufferState
{
public:
    enum
    {
        INVALID = 0 //<! return value for failed operations.
    };

    PLYLIB_API virtual bool useColors() const { return _useColors; }
    PLYLIB_API virtual void setColors( const bool colors ) { _useColors = colors; }
    PLYLIB_API virtual bool stopRendering() const { return false; }
    PLYLIB_API virtual RenderMode getRenderMode() const { return _renderMode; }
    PLYLIB_API virtual void setRenderMode( const RenderMode mode );
    PLYLIB_API virtual bool useFrustumCulling() const { return _useFrustumCulling; }
    PLYLIB_API virtual void setFrustumCulling( const bool frustumCullingState )
        { _useFrustumCulling = frustumCullingState; }

    PLYLIB_API void setProjectionModelViewMatrix( const Matrix4f& pmv )
        { _pmvMatrix = pmv; }
    PLYLIB_API const Matrix4f& getProjectionModelViewMatrix() const
        { return _pmvMatrix; }

    PLYLIB_API void setRange( const Range& range ) { _range = range; }
    PLYLIB_API const Range& getRange() const { return _range; }

    PLYLIB_API void resetRegion();
    PLYLIB_API void updateRegion( const BoundingBox& box );
    PLYLIB_API virtual void declareRegion( const Vector4f& ) {}
    PLYLIB_API Vector4f getRegion() const;

    PLYLIB_API virtual GLuint getDisplayList( const void* key ) = 0;
    PLYLIB_API virtual GLuint newDisplayList( const void* key ) = 0;
    PLYLIB_API virtual GLuint getBufferObject( const void* key ) = 0;
    PLYLIB_API virtual GLuint newBufferObject( const void* key ) = 0;
    PLYLIB_API virtual void deleteAll() = 0;

    PLYLIB_API const GLEWContext* glewGetContext() const
        { return _glewContext; }

protected:
    PLYLIB_API VertexBufferState( const GLEWContext* glewContext );
    PLYLIB_API virtual ~VertexBufferState() {}

    Matrix4f      _pmvMatrix; //!< projection * modelView matrix
    Range         _range; //!< normalized [0,1] part of the model to draw
    const GLEWContext* const _glewContext;
    RenderMode    _renderMode;
    Vector4f      _region; //!< normalized x1 y1 x2 y2 region from cullDraw
    bool          _useColors;
    bool          _useFrustumCulling;

private:
};


/*  Simple state for stand-alone single-pipe usage.  */
class VertexBufferStateSimple : public VertexBufferState
{
private:
    typedef std::map< const void*, GLuint > GLMap;
    typedef GLMap::const_iterator GLMapCIter;

public:
    PLYLIB_API VertexBufferStateSimple( const GLEWContext* glewContext )
        : VertexBufferState( glewContext ) {}

    PLYLIB_API virtual GLuint getDisplayList( const void* key );
    PLYLIB_API virtual GLuint newDisplayList( const void* key );
    PLYLIB_API virtual GLuint getBufferObject( const void* key );
    PLYLIB_API virtual GLuint newBufferObject( const void* key );
    PLYLIB_API virtual void deleteAll();

private:
    GLMap  _displayLists;
    GLMap  _bufferObjects;
};
} // namespace ply


#endif // PLYLIB_VERTEXBUFFERSTATE_H
