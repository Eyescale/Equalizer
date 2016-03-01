
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

#include "rawVolModelRenderer.h"

#include "fragmentShader.glsl.h"
#include "vertexShader.glsl.h"


namespace eVolve
{


RawVolumeModelRenderer::RawVolumeModelRenderer( const std::string& filename,
                                                const uint32_t     precision )
    : _rawModel(  filename  )
    , _precision( precision )
    , _glewContext( 0 )
    , _ortho( false )
{
}


static void renderSlices( const SliceClipper& sliceClipper )
{
    int numberOfSlices = static_cast<int>( 3.6 / sliceClipper.sliceDistance );

    for( int s = 0; s < numberOfSlices; ++s )
    {
        glBegin( GL_POLYGON );
        for( int i = 0; i < 6; ++i )
        {
            eq::Vector3f pos =
                sliceClipper.getPosition( i, numberOfSlices-1-s );

            glVertex4f( pos.x(), pos.y(), pos.z(), 1.0 );
        }
        glEnd();
    }
}


void RawVolumeModelRenderer::_putVolumeDataToShader(
    const VolumeInfo&   volumeInfo,
        const float         sliceDistance,
        const eq::Matrix4f& invRotationM,
        const eq::Vector4f& taintColor,
        const int           normalsQuality )
{
    LBASSERT( _glewContext );

    GLhandleARB shader = _shaders.getProgram();
    LBASSERT( shader );

    const DataInTextureDimensions& TD = volumeInfo.TD;

    GLint tParamNameGL;

    // Put texture coordinates modifyers to the shader
    tParamNameGL = glGetUniformLocationARB( shader, "W"  );
    glUniform1fARB( tParamNameGL, TD.W );

    tParamNameGL = glGetUniformLocationARB( shader, "H"  );
    glUniform1fARB( tParamNameGL, TD.H );

    tParamNameGL = glGetUniformLocationARB( shader, "D"  );
    glUniform1fARB( tParamNameGL, TD.D  );

    tParamNameGL = glGetUniformLocationARB( shader, "Do" );
    glUniform1fARB( tParamNameGL, TD.Do );

    tParamNameGL = glGetUniformLocationARB( shader, "Db" );
    glUniform1fARB( tParamNameGL, TD.Db );

    // Put Volume data to the shader
    glActiveTextureARB( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, volumeInfo.preint ); //preintegrated values
    tParamNameGL = glGetUniformLocationARB( shader, "preInt" );
    glUniform1iARB( tParamNameGL,  1    ); //f-shader

    // Activate last because it has to be the active texture
    glActiveTextureARB( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_3D, volumeInfo.volume ); //gx, gy, gz, val
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER,GL_LINEAR    );

    tParamNameGL = glGetUniformLocationARB(  shader,  "volume"        );
    glUniform1iARB( tParamNameGL ,  0            ); //f-shader

    tParamNameGL = glGetUniformLocationARB(  shader,  "sliceDistance" );
    glUniform1fARB( tParamNameGL,  sliceDistance ); //v-shader

    tParamNameGL = glGetUniformLocationARB(  shader,  "perspProj"     );
    glUniform1fARB( tParamNameGL,  _ortho ? 0.0f : 1.0f ); //v-shader

    tParamNameGL = glGetUniformLocationARB(  shader,  "shininess"     );
    glUniform1fARB( tParamNameGL,  8.0f         ); //f-shader

    tParamNameGL = glGetUniformLocationARB(  shader,  "taint"         );
    glUniform4fARB( tParamNameGL,   taintColor.r(),
                                    taintColor.g(),
                                    taintColor.b(),
                                    taintColor.a()  ); //f-shader

    tParamNameGL = glGetUniformLocationARB(  shader,  "sizeVec"       );
    glUniform3fARB( tParamNameGL,   volumeInfo.voxelSize.W,
                                    volumeInfo.voxelSize.H,
                                    volumeInfo.voxelSize.D  ); //f-shader

    tParamNameGL = glGetUniformLocationARB(  shader,  "normalsQuality");
    glUniform1iARB( tParamNameGL, normalsQuality ); //f-shader

    // rotate viewPosition in the opposite direction of model rotation
    // to keep light position constant but not recalculate normals
    // in the fragment shader
    // viewPosition = invRotationM * eq::Vector4f( 0, 0, 1, 0 );
    tParamNameGL = glGetUniformLocationARB(  shader,  "viewVec"       );
    glUniform3fARB( tParamNameGL, invRotationM.array[8],
                                  invRotationM.array[9],
                                  invRotationM.array[10] ); //f-shader
}


bool RawVolumeModelRenderer::render( const eq::Range& range,
                                     const eq::Matrix4f& modelviewM,
                                     const eq::Matrix4f& invRotationM,
                                     const eq::Vector4f& taintColor,
                                     const int normalsQuality )
{
    VolumeInfo volumeInfo;

    if( !_rawModel.getVolumeInfo( volumeInfo, range ))
    {
        LBERROR << "Can't get volume data" << std::endl;
        return false;
    }

    glScalef( volumeInfo.volScaling.W,
              volumeInfo.volScaling.H,
              volumeInfo.volScaling.D );

    // Enable shaders
    glUseProgramObjectARB( _shaders.getProgram( ));

    // Calculate and put necessary data to shaders

    const uint32_t resolution    = _rawModel.getResolution();
    const double   sliceDistance = 3.6 / ( resolution * _precision );

    _putVolumeDataToShader( volumeInfo, float( sliceDistance ),
                            invRotationM, taintColor, normalsQuality );

    _sliceClipper.updatePerFrameInfo( modelviewM, sliceDistance, range );

    //Render slices
    glEnable( GL_BLEND );
    glBlendFuncSeparateEXT( GL_ONE, GL_SRC_ALPHA, GL_ZERO, GL_SRC_ALPHA );

    renderSlices( _sliceClipper );

    glDisable( GL_BLEND );

    // Disable shader
    glUseProgramObjectARB( 0 );

    return true;
}


bool RawVolumeModelRenderer::loadShaders()
{
    if( !_shaders.loadShaders( vertexShader_glsl, fragmentShader_glsl,
                               _glewContext ))
    {
        LBERROR << "Can't load glsl shaders" << std::endl;
        return false;
    }

    LBLOG( eq::LOG_CUSTOM ) << "glsl shaders loaded" << std::endl;
    return true;
}
}
