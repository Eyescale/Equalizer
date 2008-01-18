/* Copyright (c) 2007       Maxim Makhinya
   All rights reserved. */

#include "rawVolModelRenderer.h"
#include "shader.h"

#include "fragmentShader_glsl.h"
#include "vertexShader_glsl.h"


namespace eVolve
{


RawVolumeModelRenderer::RawVolumeModelRenderer( const std::string& filename, 
                                                const uint32_t     precision )
        : _rawModel(  filename  )
        , _precision( precision )
        , _shadersLoaded( false )
        , _glewContext( 0 )
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

            glVertex3f( pos.x, pos.y, pos.z );
        }
        glEnd();
    }
}


void RawVolumeModelRenderer::_putVolumeDataToShader
(
    const VolumeInfo& volumeInfo,
    const double      sliceDistance
)
{
    EQASSERT( _glewContext );

    const DataInTextureDimensions& TD = volumeInfo.TD; 

    GLint tParamNameGL;

    // Put texture coordinates modifyers to the shader
    tParamNameGL = glGetUniformLocationARB( _shader, "W"  );
    glUniform1fARB( tParamNameGL, TD.W );

    tParamNameGL = glGetUniformLocationARB( _shader, "H"  );
    glUniform1fARB( tParamNameGL, TD.H );

    tParamNameGL = glGetUniformLocationARB( _shader, "D"  );
    glUniform1fARB( tParamNameGL, TD.D  );

    tParamNameGL = glGetUniformLocationARB( _shader, "Do" );
    glUniform1fARB( tParamNameGL, TD.Do );

    tParamNameGL = glGetUniformLocationARB( _shader, "Db" );
    glUniform1fARB( tParamNameGL, TD.Db );

    // Put Volume data to the shader
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, volumeInfo.preint ); //preintegrated values
    tParamNameGL = glGetUniformLocationARB( _shader, "preInt" );
    glUniform1iARB( tParamNameGL,  1    ); //f-shader

    // Activate last because it has to be the active texture
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_3D, volumeInfo.volume ); //gx, gy, gz, val
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER,GL_LINEAR    );

    tParamNameGL = glGetUniformLocationARB(  _shader,  "volume"        );
    glUniform1iARB( tParamNameGL ,  0            ); //f-shader

    tParamNameGL = glGetUniformLocationARB(  _shader,  "sliceDistance" );
    glUniform1fARB( tParamNameGL,  sliceDistance ); //v-shader

    tParamNameGL = glGetUniformLocationARB(  _shader,  "shininess"     );
    glUniform1fARB( tParamNameGL,  20.0f         ); //f-shader
}


bool RawVolumeModelRenderer::Render
(
    const eq::Range&        range,
    const vmml::Matrix4d&   modelviewM,
    const vmml::Matrix3d&   modelviewITM
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
    glUseProgramObjectARB( _shader );

    // Calculate and put necessary data to shaders 

    const uint32_t resolution    = _rawModel.getResolution();
    const double   sliceDistance = 3.6 / ( resolution * _precision );

//    if( !_rawModel.isSameRange() )
        _putVolumeDataToShader( volumeInfo, sliceDistance );

    _sliceClipper.updatePerFrameInfo( modelviewM, modelviewITM,
                                      sliceDistance, range );

    //Render slices
    glEnable( GL_BLEND );
#ifdef COMPOSE_MODE_NEW
    glBlendFuncSeparate( GL_ONE, GL_SRC_ALPHA, GL_ZERO, GL_SRC_ALPHA );
#else
    glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
#endif

    renderSlices( _sliceClipper );

    glDisable( GL_BLEND );

    // Disable shader
    glUseProgramObjectARB( 0 );

    return true;
}


bool RawVolumeModelRenderer::LoadShaders()
{
    if( !_shadersLoaded )
    {
        EQASSERT( _glewContext );

        EQLOG( eq::LOG_CUSTOM ) << "using glsl shaders" << std::endl;
        if( !eqShader::loadShaders( vertexShader_glsl,
                                    fragmentShader_glsl,
                                    _shader,
                                    _glewContext ))
        {
            EQERROR << "Can't load glsl shaders" << std::endl;
            return false;
        }

        _shadersLoaded = true;
        EQLOG( eq::LOG_CUSTOM ) << "shaders loaded" << std::endl;
    }
    return true;
}


}