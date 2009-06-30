
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

#include <eq/eq.h>

#ifndef EVOLVE_SLICECLIPPING_H
#define EVOLVE_SLICECLIPPING_H

namespace eVolve
{


struct SliceClipper
{
    SliceClipper(){};

    static const int    nSequence[8][8];
    static const float  sequence[64];
    static const float  v1[24];
    static const float  v2[24];

    typedef eq::Vector3f float3;

    void updatePerFrameInfo
    (
        const eq::Matrix4d&   modelviewM,
        const eq::Matrix3d&   modelviewITM,
        const double            sliceDistance,
        const eq::Range&        range
    );

    eq::Vector3f getPosition
    (
        const int vertexNum,
        const int sliceNum
    ) const;

    float           shaderVertices[24];
    eq::Vector3f    viewVecf;
    eq::Vector4d    viewVec;
    int             frontIndex;
    double          sliceDistance;
    double          planeStart;
};

}

#endif // EVOLVE_SLICECLIPPING_H
