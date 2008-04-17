/* Copyright (c) 2007       Maxim Makhinya
   All rights reserved. */

#include "rawVolModelRenderer.h"

#include "fragmentShader_glsl.h"
#include "vertexShader_glsl.h"


namespace eVolve
{


RawVolumeModelRenderer::RawVolumeModelRenderer( const std::string& filename, 
                                                const uint32_t     precision,
                                                const bool         perspective )
        : _rawModel(  filename  )
        , _precision( precision )
        , _glewContext( 0 )
        , _perspective( perspective )
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
            vmml::Vector3f pos =
                    sliceClipper.getPosition( i, numberOfSlices-1-s );

            glVertex4f( pos.x, pos.y, pos.z, 1.0 );
        }
        glEnd();
    }
}


void RawVolumeModelRenderer::_putVolumeDataToShader
(
    const VolumeInfo&     volumeInfo,
    const double          sliceDistance,
    const vmml::Matrix4f& invRotationM
)
{
    EQASSERT( _glewContext );

    GLhandleARB shader = _shaders.getProgram();
    EQASSERT( shader );

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

    tParamNameGL = glGetUniformLocationARB(  shader,  "perspProj" );
    glUniform1fARB( tParamNameGL,  _perspective ? 1.0 : 0.0 ); //v-shader

    tParamNameGL = glGetUniformLocationARB(  shader,  "shininess"     );
    glUniform1fARB( tParamNameGL,  8.0f         ); //f-shader

    // rotate viewPosition in the oposite direction of model rotation
    // to keep light position constant but not recalculate normals 
    // in the fragment shader
    // viewPosition = invRotationM * vmml::Vector4f( 0, 0, 1, 0 );
    tParamNameGL = glGetUniformLocationARB(  shader,  "viewVec"       );
    glUniform3fARB( tParamNameGL, invRotationM.ml[8],
                                  invRotationM.ml[9],
                                  invRotationM.ml[10] ); //f-shader
}


bool RawVolumeModelRenderer::render
(
    const eq::Range&        range,
    const vmml::Matrix4d&   modelviewM,
    const vmml::Matrix3d&   modelviewITM,
    const vmml::Matrix4f&   invRotationM
)
{
    VolumeInfo volumeInfo;

    if( !_rawModel.getVolumeInfo( volumeInfo, range ))
    {
        EQERROR << "Can't get volume data" << std::endl;
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

    _putVolumeDataToShader( volumeInfo, sliceDistance, invRotationM );

    _sliceClipper.updatePerFrameInfo( modelviewM, modelviewITM,
                                      sliceDistance, range );

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
        EQERROR << "Can't load glsl shaders" << std::endl;
        return false;
    }

    EQLOG( eq::LOG_CUSTOM ) << "glsl shaders loaded" << std::endl;
    return true;
}
}


