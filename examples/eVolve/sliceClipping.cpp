
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


namespace eVolve
{

    const int SliceClipper::nSequence[8][8] = {
        {7,3,5,6,1,2,4,0},
        {6,2,4,7,0,3,5,1},
        {5,1,4,7,0,3,6,2},
        {4,0,5,6,1,2,7,3},
        {3,1,2,7,0,5,6,4},
        {2,0,3,6,1,4,7,5},
        {1,0,3,5,2,4,7,6},
        {0,1,2,4,3,5,6,7},
    };

    const float SliceClipper::sequence[64] = {
        0, 1, 4, 2, 3, 5, 6, 7,
        1, 0, 3, 5, 4, 2, 7, 6,
        2, 0, 6, 3, 1, 4, 7, 5,
        3, 1, 2, 7, 5, 0, 6, 4,
        4, 0, 5, 6, 2, 1, 7, 3,
        5, 1, 7, 4, 0, 3, 6, 2,
        6, 2, 4, 7, 3, 0, 5, 1,
        7, 3, 6, 5, 1, 2, 4, 0 };

    const float SliceClipper::v1[24] = {
        0, 1, 4, 4,
        1, 0, 1, 4,
        0, 2, 5, 5,
        2, 0, 2, 5,
        0, 3, 6, 6,
        3, 0, 3, 6 };

    const float SliceClipper::v2[24] = {
        1, 4, 7, 7,
        5, 1, 4, 7,
        2, 5, 7, 7,
        6, 2, 5, 7,
        3, 6, 7, 7,
        4, 3, 6, 7 };

SliceClipper::SliceClipper()
    : shaderVertices()
    , frontIndex( 0 )
    , sliceDistance( 0 )
    , planeStart( 0 )
{
}

void SliceClipper::updatePerFrameInfo( const eq::Matrix4f& modelviewM,
                                       const double newSliceDistance,
                                       const eq::Range& range
)
{
    double zRs = -1+2.*range.start;
    double zRe = -1+2.*range.end;

    //rendering parallelepipid's verteces
    eq::Vector4f vertices[8];
    vertices[0] = eq::Vector4f(-1.0,-1.0,zRs, 1.0);
    vertices[1] = eq::Vector4f( 1.0,-1.0,zRs, 1.0);
    vertices[2] = eq::Vector4f(-1.0, 1.0,zRs, 1.0);
    vertices[3] = eq::Vector4f( 1.0, 1.0,zRs, 1.0);

    vertices[4] = eq::Vector4f(-1.0,-1.0,zRe, 1.0);
    vertices[5] = eq::Vector4f( 1.0,-1.0,zRe, 1.0);
    vertices[6] = eq::Vector4f(-1.0, 1.0,zRe, 1.0);
    vertices[7] = eq::Vector4f( 1.0, 1.0,zRe, 1.0);

    for( int i=0; i<8; i++ )
        for( int j=0; j<3; j++)
            shaderVertices[ i*3+j ] = float( vertices[i][j] );


    viewVec = eq::Vector4f( -modelviewM.array[ 2],
                            -modelviewM.array[ 6],
                            -modelviewM.array[10],
                            0.0 );

    viewVecf = eq::Vector3f( float( viewVec.x( )), float( viewVec.y( )),
                             float( viewVec.z( )));

    sliceDistance = newSliceDistance;

    frontIndex = 0;
    float maxDist = float( viewVec.dot( vertices[0] ));
    for( int i = 1; i < 8; i++ )
    {
        const float dist = float( viewVec.dot( vertices[i] ));
        if ( dist > maxDist)
        {
            maxDist = dist;
            frontIndex = i;
        }
    }

    planeStart  = viewVec.dot( vertices[nSequence[frontIndex][0]] );
    double dS   = ceil( planeStart/sliceDistance );
    planeStart  = dS * sliceDistance;
}


eq::Vector3f SliceClipper::getPosition
(
    const int vertexNum,
    const int sliceNum
) const
{
    const float dPlaneDist = float( planeStart + sliceNum * sliceDistance );
    float3  Position     = float3( 0.0, 0.0, 0.0 );

    for( int e = 0; e < 4; e++ )
    {
        int vidx1 = 3*static_cast<int>( sequence[
                        static_cast<int>(frontIndex * 8 + v1[vertexNum*4+e])]);
        int vidx2 = 3*static_cast<int>( sequence[
                        static_cast<int>(frontIndex * 8 + v2[vertexNum*4+e])]);

        float3 vecV1( shaderVertices[vidx1  ],
                      shaderVertices[vidx1+1],
                      shaderVertices[vidx1+2] );

        float3 vecV2( shaderVertices[vidx2  ],
                      shaderVertices[vidx2+1],
                      shaderVertices[vidx2+2] );

        float3 vecStart = vecV1;
        float3 vecDir   = vecV2-vecV1;

        float denom = vecDir.dot( viewVecf );
        float lambda = (denom != 0.0f) ?
                       (dPlaneDist - vecStart.dot(viewVecf))/denom : -1.0f;

        if(( lambda >= 0.0f ) && ( lambda <= 1.0f ))
        {
            Position = vecStart + vecDir * lambda;
            break;
        }
    }
    return Position;
}


}
