
/* Copyright (c) 2007       Maxim Makhinya
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

#include "sliceClipping.h"


namespace massVolVis
{

const int SliceClipper::_nSequence[8][8] = {
    {7,3,5,6,1,2,4,0},
    {6,2,4,7,0,3,5,1},
    {5,1,4,7,0,3,6,2},
    {4,0,5,6,1,2,7,3},
    {3,1,2,7,0,5,6,4},
    {2,0,3,6,1,4,7,5},
    {1,0,3,5,2,4,7,6},
    {0,1,2,4,3,5,6,7},
};

const float SliceClipper::_sequence[64] = {
    0, 1, 4, 2, 3, 5, 6, 7,
    1, 0, 3, 5, 4, 2, 7, 6,
    2, 0, 6, 3, 1, 4, 7, 5,
    3, 1, 2, 7, 5, 0, 6, 4,
    4, 0, 5, 6, 2, 1, 7, 3,
    5, 1, 7, 4, 0, 3, 6, 2,
    6, 2, 4, 7, 3, 0, 5, 1,
    7, 3, 6, 5, 1, 2, 4, 0 };

const float SliceClipper::_v1[24] = {
    0, 1, 4, 4,
    1, 0, 1, 4,
    0, 2, 5, 5,
    2, 0, 2, 5,
    0, 3, 6, 6,
    3, 0, 3, 6 };

const float SliceClipper::_v2[24] = {
    1, 4, 7, 7,
    5, 1, 4, 7,
    2, 5, 7, 7,
    6, 2, 5, 7,
    3, 6, 7, 7,
    4, 3, 6, 7 };


SliceClipper::SliceClipper()
    : _frontIndex(      0 )
    , _sliceDistance(   0 )
    , _planeStart(      0 )
    , _numSlices(       0 )

{
    for( uint32_t i = 0; i < 24; ++i )
        _shaderVertices[i] = 0;

    LBERROR << "vec size: " << sizeof( _shaderVertices ) << std::endl;
}


void SliceClipper::updatePerFrameInfo( const Matrix4d& modelviewM )
{
    _viewVec = Vector4d( -modelviewM.array[ 2],
                         -modelviewM.array[ 6],
                         -modelviewM.array[10],
                          0.0                 );

    _viewVecf = Vector3f( float( _viewVec.x( )),
                          float( _viewVec.y( )),
                          float( _viewVec.z( )));
}


void SliceClipper::updatePerBlockInfo( const uint32_t numSlices, const double cubeDiagonal, const Box_f& coords )
{
    _numSlices     = numSlices;
    if( numSlices < 1 )
    {
        LBERROR << "number of slices has to be more than 0" << std::endl;
        return;
    }
    _sliceDistance = cubeDiagonal / numSlices;
//    LBWARN << " slice distance: " << _sliceDistance << std::endl;

    const Vec3_f& s = coords.s;
    const Vec3_f& e = coords.e;

    //rendering parallelepipid's verteces
    _vertices[0].set( s.x, s.y, s.z, 1.0 );
    _vertices[1].set( e.x, s.y, s.z, 1.0 );
    _vertices[2].set( s.x, e.y, s.z, 1.0 );
    _vertices[3].set( e.x, e.y, s.z, 1.0 );

    _vertices[4].set( s.x, s.y, e.z, 1.0 );
    _vertices[5].set( e.x, s.y, e.z, 1.0 );
    _vertices[6].set( s.x, e.y, e.z, 1.0 );
    _vertices[7].set( e.x, e.y, e.z, 1.0 );

    for( int i = 0; i < 8; ++i )
        for( int j = 0; j < 3; ++j )
            _shaderVertices[ i*3+j ] = float( _vertices[i][j] );

    // find a closest point to the screen
    _frontIndex = 0;
    float maxDist = float( _viewVec.dot( _vertices[0] ));
    for( int i = 1; i < 8; ++i )
    {
        const float dist = float( _viewVec.dot( _vertices[i] ));
        if( dist > maxDist )
        {
            maxDist = dist;
            _frontIndex = i;
        }
    }

    _planeStart = _viewVec.dot( _vertices[ _nSequence[ _frontIndex ][0]] );
    _planeStart = ceil( _planeStart / _sliceDistance ) * _sliceDistance;
}


Vector3f SliceClipper::getPosition( const int vertexNum, const uint32_t sliceNum ) const
{
    const float dPlaneDist = float( _planeStart + sliceNum * _sliceDistance );

    for( int e = 0; e < 4; ++e )
    {
        const int vidx1 = 3*static_cast<int>( _sequence[ static_cast<int>(_frontIndex * 8 + _v1[vertexNum*4+e]) ]);
        const int vidx2 = 3*static_cast<int>( _sequence[ static_cast<int>(_frontIndex * 8 + _v2[vertexNum*4+e]) ]);

        const Vector3f vecV1( _shaderVertices[vidx1  ],
                              _shaderVertices[vidx1+1],
                              _shaderVertices[vidx1+2] );

        const Vector3f vecV2( _shaderVertices[vidx2  ],
                              _shaderVertices[vidx2+1],
                              _shaderVertices[vidx2+2] );

        const Vector3f vecStart = vecV1;
        const Vector3f vecDir   = vecV2-vecV1;

        const float denom  = vecDir.dot( _viewVecf );
        const float lambda = (denom != 0.0f) ? ( dPlaneDist - vecStart.dot(_viewVecf) ) / denom : -1.0f;

        if(( lambda >= 0.0f ) && ( lambda <= 1.0f ))
        {
            return vecStart + vecDir * lambda;
        }
    }
    return Vector3f( 0.f, 0.f, 0.f );
}


}

