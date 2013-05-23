
#include "rendererSlice.h"

#include "../transferFunction.h"

#include "sliceClipping.h"
#include "../glslShaders.h"
#include "../../LB/renderNode.h"

#include "renderer/slice/fragmentShader_slice.glsl.h"
#include "renderer/slice/vertexShader_slice.glsl.h"

#include <msv/util/hlp.h>
#include <msv/types/box.h>

#include <eq/client/log.h>


namespace massVolVis
{

RendererSlice::RendererSlice()
    : RendererBase()
    , _isInitialized( false )
    , _volumeName( 0 )
    , _preintName( 0 )
    , _sliceClipperPtr( new SliceClipper() )
    , _precision( 2 )
    , _ortho( false )
{
    _preintData.resize( 256*256*4 );
}


RendererSlice::~RendererSlice()
{
    // deinit TF
    if( _preintName )
        glDeleteTextures( 1, &_preintName );
}


bool RendererSlice::init( GLuint texture3D )
{
    if( _isInitialized )
        return true;

    const GLEWContext* context = glewGetContext();
    if( !context )
    {
        LBERROR << "Can't get GLEW context" << std::endl;
        return false;
    }

    if( !_loadShaders( vertexShader_slice_glsl, fragmentShader_slice_glsl ))
        return false;

    _volumeName = texture3D;

    _isInitialized = true;
    return true;
}


void RendererSlice::_renderSlices( const Vec3_f& memCoords, const Box_f& blockCoords )
{
// put the rest of the parameters to shader
    LBASSERT( glewGetContext( ));

    GLhandleARB shader = _getShaderProgram();
    LBASSERT( shader );

    GLint tParamNameGL;

    tParamNameGL = glGetUniformLocationARB(  shader,  "sliceDistance" );
    glUniform1fARB( tParamNameGL,  _sliceClipperPtr->getSliceDistance( )); //v-shader

    const Vec3_f offset = blockCoords.s;
    const Vec3_f dim    = blockCoords.getDim();

    tParamNameGL = glGetUniformLocationARB(  shader,  "coordOffset" );
    glUniform3fARB( tParamNameGL, offset.x, offset.y, offset.z ); //v-shader

    tParamNameGL = glGetUniformLocationARB(  shader,  "coordDim" );
    glUniform3fARB( tParamNameGL, dim.x, dim.y, dim.z ); //v-shader

    tParamNameGL = glGetUniformLocationARB(  shader,  "memOffset" );
    glUniform3fARB( tParamNameGL, memCoords.x, memCoords.y, memCoords.z ); //v-shader

// render
    for( uint32_t s = _sliceClipperPtr->getNumberOfSlices(); s > 0; --s )
    {
        glBegin( GL_POLYGON );
//        glBegin( GL_LINE_LOOP );
//        glColor3f( 1.f, 1.f, 1.f );
        for( int i = 0; i < 6; ++i )
        {
            Vector3f pos = _sliceClipperPtr->getPosition( i, s-1 );

            glVertex4f( pos.x(), pos.y(), pos.z(), 1.0 );
        }
        glEnd();
    }
}


void RendererSlice::frameInit( const FrameInitParameters& fInitParams )
{
    if( !_isInitialized )
        return;

    if( _preintName == 0  || _volumeName == 0 )
        return;

    LBASSERT( glewGetContext( ));

    _sliceClipperPtr->updatePerFrameInfo( fInitParams.modelviewM );

    GLhandleARB shader = _getShaderProgram();
    LBASSERT( shader );

    // Enable shaders
    EQ_GL_CALL( glUseProgramObjectARB( shader ));

    GLint tParamNameGL;

    // Put texture coordinates modifyers to the shader
    tParamNameGL = glGetUniformLocationARB( shader, "memBlockSize"  );
    glUniform3fARB( tParamNameGL, fInitParams.memBlockSize.w,
                                  fInitParams.memBlockSize.h,
                                  fInitParams.memBlockSize.d );

    tParamNameGL = glGetUniformLocationARB( shader, "memShift" );
    glUniform3fARB( tParamNameGL, fInitParams.innerShift.w,
                                  fInitParams.innerShift.h,
                                  fInitParams.innerShift.d );

    // Put Volume data to the shader
    glActiveTextureARB( GL_TEXTURE1 );
    EQ_GL_CALL( glBindTexture( GL_TEXTURE_2D, _preintName )); //preintegrated values
    tParamNameGL = glGetUniformLocationARB( shader, "preInt" );
    glUniform1iARB( tParamNameGL,  1    ); //f-shader

    // Activate last because it has to be the active texture
    EQ_GL_CALL( glActiveTextureARB( GL_TEXTURE0 ));
    glBindTexture( GL_TEXTURE_3D, _volumeName );
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER,GL_LINEAR    );

    tParamNameGL = glGetUniformLocationARB(  shader,  "volume"        );
    glUniform1iARB( tParamNameGL ,  0            ); //f-shader

//    tParamNameGL = glGetUniformLocationARB(  shader,  "perspProj"     );
//    glUniform1fARB( tParamNameGL,  _ortho ? 0.0f : 1.0f ); //v-shader

    tParamNameGL = glGetUniformLocationARB(  shader,  "shininess"     );
    glUniform1fARB( tParamNameGL,  8.0f         ); //f-shader

    tParamNameGL = glGetUniformLocationARB(  shader,  "taint"         );
    glUniform4fARB( tParamNameGL,   fInitParams.taintColor.r(),
                                    fInitParams.taintColor.g(),
                                    fInitParams.taintColor.b(),
                                    fInitParams.taintColor.a()  ); //f-shader

    tParamNameGL = glGetUniformLocationARB(  shader,  "sizeVec"       );
    glUniform3fARB( tParamNameGL, fInitParams.voxelSize.w,
                                  fInitParams.voxelSize.h,
                                  fInitParams.voxelSize.d  ); //f-shader

    tParamNameGL = glGetUniformLocationARB(  shader,  "normalsQuality");
    glUniform1iARB( tParamNameGL, fInitParams.normalsQuality ); //f-shader

    // rotate viewPosition in the opposite direction of model rotation
    // to keep light position constant but not recalculate normals 
    // in the fragment shader
    tParamNameGL = glGetUniformLocationARB(  shader,  "viewVec"       );
    glUniform3fARB( tParamNameGL, fInitParams.viewVector.x(),
                                  fInitParams.viewVector.y(),
                                  fInitParams.viewVector.z() ); //f-shader

    EQ_GL_CALL( glEnable( GL_BLEND ));
    EQ_GL_CALL( glBlendFuncSeparateEXT( GL_ONE, GL_SRC_ALPHA, GL_ZERO, GL_SRC_ALPHA ));
}


void RendererSlice::frameFinish()
{
    if( !_isInitialized )
        return;

    if( _preintName == 0  || _volumeName == 0 )
        return;

    EQ_GL_CALL( glDisable( GL_BLEND ));

    // Disable shader
    glUseProgramObjectARB( 0 );
}


void RendererSlice::renderBrick
(
    const RenderNode&   renderNode,
    const Box_f&        drawCoords,
    const Vec3_f&       memCoords,
    const uint32_t      numberOfSlices
)
{
    if( !_isInitialized )
        return;

    if( _preintName == 0  || _volumeName == 0 )
        return;

    // cube diagonal = sqrt(( sqrt( a^2 + a^2 ))^2 + a^2 ) == a*sqrt(3); a == 1
    // when scale is not 1.0, then diagonal has to be multiplied by it as well
    const Vec3_f dim = renderNode.coords.getDim();
    const float diagonal = sqrt( dim.x*dim.x + dim.y*dim.y + dim.z*dim.z );
    _sliceClipperPtr->updatePerBlockInfo( numberOfSlices, diagonal*1.0, drawCoords );

    _renderSlices( memCoords, renderNode.coords );

/*
    if( _preintName )
    {
        glEnable( GL_TEXTURE_2D );
        glDisable( GL_LIGHTING );
        glBindTexture( GL_TEXTURE_2D, _preintName );
    }

    float tMin = 0.f;
    float tMax = 1.f;
    // render six axis-aligned colored quads around the origin
    for( int i = 0; i < 6; i++ )
    {
        glColor3f( 1.0f, 1.0f, 1.0f );

        glNormal3f( 0.0f, 0.0f, 1.0f );
        glBegin( GL_TRIANGLE_STRIP );
            glTexCoord2f( tMax, tMax );
            glVertex3f(  .7f,  .7f, -1.0f );
            glTexCoord2f( tMin, tMax );
            glVertex3f( -.7f,  .7f, -1.0f );
            glTexCoord2f( tMax, tMin );
            glVertex3f(  .7f, -.7f, -1.0f );
            glTexCoord2f( tMin, tMin );
            glVertex3f( -.7f, -.7f, -1.0f );
        glEnd();

        if( i < 3 )
            glRotatef(  90.0f, 0.0f, 1.0f, 0.0f );
        else if( i == 3 )
            glRotatef(  90.0f, 1.0f, 0.0f, 0.0f );
        else
            glRotatef( 180.0f, 1.0f, 0.0f, 0.0f );
    }

    if( _preintName )
    {
        glDisable( GL_TEXTURE_2D );
    }
*/
}


void RendererSlice::initTF( const TransferFunction& tf )
{
    using hlpFuncs::myClip;

// create preintegration table
    LBASSERT( tf.rgba.size() == 256*4 );
    const byte* rgba = &tf.rgba[0];

    double rInt[256]; rInt[0] = 0.;
    double gInt[256]; gInt[0] = 0.;
    double bInt[256]; bInt[0] = 0.;
    double aInt[256]; aInt[0] = 0.;

    // Creating SAT (Summed Area Tables) from averaged neighbouring RGBA values
    for( int i=1; i<256; i++ )
    {
        // average Alpha from two neighbouring TF values
        const double tauc =   ( rgba[(i-1)*4+3] + rgba[i*4+3] ) / 2. / 255.;

        // SAT of average RGBs from two neighbouring TF values 
        // multiplied with Alpha
        rInt[i] = rInt[i-1] + ( rgba[(i-1)*4+0] + rgba[i*4+0] )/2.*tauc;
        gInt[i] = gInt[i-1] + ( rgba[(i-1)*4+1] + rgba[i*4+1] )/2.*tauc;
        bInt[i] = bInt[i-1] + ( rgba[(i-1)*4+2] + rgba[i*4+2] )/2.*tauc;

        // SAT of average Alpha values
        aInt[i] = aInt[i-1] + tauc;
    }

    int lookupindex=0;

    for( int sb=0; sb<256; sb++ )
    {
        for( int sf=0; sf<256; sf++ )
        {
            int smin, smax;
            if( sb<sf ) { smin=sb; smax=sf; }
            else        { smin=sf; smax=sb; }

            double rcol, gcol, bcol, acol;
            if( smax != smin )
            {
                const double factor = 1. / (double)(smax-smin);
                rcol =       ( rInt[smax] - rInt[smin] ) * factor;
                gcol =       ( gInt[smax] - gInt[smin] ) * factor;
                bcol =       ( bInt[smax] - bInt[smin] ) * factor;
                acol = exp( -( aInt[smax] - aInt[smin] ) * factor) * 255.;
            } else
            {
                const int    index  = smin*4;
                const double factor = 1./255.;
                rcol =       rgba[index+0] * rgba[index+3] * factor;
                gcol =       rgba[index+1] * rgba[index+3] * factor;
                bcol =       rgba[index+2] * rgba[index+3] * factor;
                acol = exp(                 -rgba[index+3] * factor ) * 256.;
            }
            _preintData[lookupindex++] = myClip( static_cast<int>(rcol), 0, 255 );
            _preintData[lookupindex++] = myClip( static_cast<int>(gcol), 0, 255 );
            _preintData[lookupindex++] = myClip( static_cast<int>(bcol), 0, 255 );
            _preintData[lookupindex++] = myClip( static_cast<int>(acol), 0, 255 );
        }
    }

    // initial texture initialization
    if( _preintName == 0 )
    {
        GLuint preintName = 0;
        glGenTextures( 1, &preintName );
        glBindTexture( GL_TEXTURE_2D, preintName );
        glTexImage2D(  GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, 
                       GL_RGBA, GL_UNSIGNED_BYTE, &_preintData[0] );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR        );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR        );

        _preintName  = preintName;
        return;
    }
    // else update existing data
    glBindTexture( GL_TEXTURE_2D, _preintName );
    glTexImage2D(  GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, &_preintData[0] );
}


}//namespace massVolVis
