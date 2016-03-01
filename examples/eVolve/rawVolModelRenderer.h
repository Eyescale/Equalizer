
/* Copyright (c) 2007-2011, Maxim Makhinya  <maxmah@gmail.com>
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

#ifndef EVOLVE_RAW_VOL_MODEL_RENDERER_H
#define EVOLVE_RAW_VOL_MODEL_RENDERER_H

#include "rawVolModel.h"
#include "sliceClipping.h"
#include "glslShaders.h"


namespace eVolve
{

class RawVolumeModelRenderer
{
public:
    RawVolumeModelRenderer( const std::string& filename,
                            const uint32_t     precision   = 1 );

    bool loadHeader( const float brightness, const float alpha )
    {
        return _rawModel.loadHeader( brightness, alpha );
    }

    const VolumeScaling& getVolumeScaling() const
    {
        return _rawModel.getVolumeScaling();
    }

    void glewSetContext( const GLEWContext* context )
    {
        _glewContext = context;
        _rawModel.glewSetContext( context );
    }


    bool render( const eq::Range&     range,
                 const eq::Matrix4f&  modelviewM,
                 const eq::Matrix4f&  invRotationM,
                 const eq::Vector4f&  taintColor,
                 const int            normalsQuality );

    void setPrecision( const uint32_t precision ){ _precision = precision; }
    void setOrtho( const uint32_t ortho )        { _ortho = ortho; }

    const GLEWContext* glewGetContext() { return _glewContext; }
    bool loadShaders();

private:
    void _putVolumeDataToShader( const VolumeInfo&   volumeInfo,
                                 const float         sliceDistance,
                                 const eq::Matrix4f& invRotationM,
                                 const eq::Vector4f& taintColor,
                                 const int           normalsQuality );

    RawVolumeModel  _rawModel;      //!< volume data
    SliceClipper    _sliceClipper;  //!< frame clipping algorithm
    uint32_t        _precision;     //!< multiplyer for number of slices
    GLSLShaders     _shaders;       //!< GLSL shaders

    const GLEWContext*    _glewContext;   //!< OpenGL function table

    bool            _ortho;         //!< ortogonal/perspective projection

};

}
#endif // EVOLVE_RAW_VOL_MODEL_RENDERER_H
