
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

#include <eq/eq.h>

#ifndef EVOLVE_SLICECLIPPING_H
#define EVOLVE_SLICECLIPPING_H

namespace eVolve
{
struct SliceClipper
{
    SliceClipper();

    static const int    nSequence[8][8];
    static const float  sequence[64];
    static const float  v1[24];
    static const float  v2[24];

    typedef eq::Vector3f float3;

    void updatePerFrameInfo( const eq::Matrix4f& modelviewM,
                             const double sliceDistance,
                             const eq::Range& range );

    eq::Vector3f getPosition
    (
        const int vertexNum,
        const int sliceNum
    ) const;

    float           shaderVertices[24];
    eq::Vector3f    viewVecf;
    eq::Vector4f    viewVec;
    int             frontIndex;
    double          sliceDistance;
    double          planeStart;
};

}

#endif // EVOLVE_SLICECLIPPING_H
