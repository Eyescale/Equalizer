
/* Copyright (c) 2007       Maxim Makhinya
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


void SliceClipper::updatePerFrameInfo
(
    const eq::Matrix4d&   modelviewM,
    const eq::Matrix3d&   modelviewITM,
    const double            newSliceDistance,
    const eq::Range&        range
)
{
    double zRs = -1+2.*range.start;
    double zRe = -1+2.*range.end;

    //rendering parallelepipid's verteces
    eq::Vector4d vertices[8];
    vertices[0] = eq::Vector4d(-1.0,-1.0,zRs, 1.0);
    vertices[1] = eq::Vector4d( 1.0,-1.0,zRs, 1.0);
    vertices[2] = eq::Vector4d(-1.0, 1.0,zRs, 1.0);
    vertices[3] = eq::Vector4d( 1.0, 1.0,zRs, 1.0);

    vertices[4] = eq::Vector4d(-1.0,-1.0,zRe, 1.0);
    vertices[5] = eq::Vector4d( 1.0,-1.0,zRe, 1.0);
    vertices[6] = eq::Vector4d(-1.0, 1.0,zRe, 1.0);
    vertices[7] = eq::Vector4d( 1.0, 1.0,zRe, 1.0);

    for( int i=0; i<8; i++ )
        for( int j=0; j<3; j++)
            shaderVertices[ i*3+j ] = vertices[i][j];


    this->viewVec = eq::Vector4d( -modelviewM.array[ 2],
                                  -modelviewM.array[ 6],
                                  -modelviewM.array[10],
                                  0.0                 );

    viewVecf = eq::Vector3f( viewVec.x(), viewVec.y(), viewVec.z() );

    sliceDistance = newSliceDistance;

    frontIndex = 0;
    float maxDist = viewVec.dot( vertices[0] );
    for( int i = 1; i < 8; i++ )
    {
        float dist = viewVec.dot( vertices[i] );
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
    float   dPlaneDist   = planeStart + sliceNum * sliceDistance;
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
        float lambda = 
            (denom != 0.0) ? (dPlaneDist - vecStart.dot(viewVecf))/denom : -1.0;

        if(( lambda >= 0.0 ) && ( lambda <= 1.0 ))
        {
            Position = vecStart + vecDir * lambda;
            break;
        }
    }
    return Position;
}


}

