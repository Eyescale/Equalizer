
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

#ifndef MASS_VOL__SLICECLIPPING_H
#define MASS_VOL__SLICECLIPPING_H


#include <msv/types/box.h>
#include <msv/types/vmmlTypes.h>

namespace massVolVis
{

class SliceClipper
{
public:
    SliceClipper();

    void updatePerFrameInfo( const Matrix4d& modelviewM );

    void updatePerBlockInfo( const uint32_t numSlices, const double cubeDiagonal, const Box_f& coords );

    Vector3f getPosition( const int vertexNum, const uint32_t sliceNum ) const;

    double   getSliceDistance()  const { return _sliceDistance; }
    uint32_t getNumberOfSlices() const { return _numSlices;     }

private:
    static const int    _nSequence[8][8];
    static const float  _sequence[64];
    static const float  _v1[24];
    static const float  _v2[24];

    Vector4d    _vertices[8];

    float       _shaderVertices[24];
    Vector3f    _viewVecf;
    Vector4d    _viewVec;

    int         _frontIndex;
    double      _sliceDistance;
    double      _planeStart;

    uint32_t    _numSlices;
};

}

#endif // MASS_VOL__SLICECLIPPING_H
