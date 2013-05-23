//std12

#include "rendererRaycast2.h"

#include "nearPlaneClipper.h"

#include "../transferFunction.h"

#include "../glslShaders.h"
#include "../../LB/renderNode.h"

//#define USE_2D_TF

#ifdef USE_2D_TF
    #include "renderer/raycasting/fragmentShader_raycast.glsl.h"
#else //USE_2D_TF
    #include "renderer/raycasting/fragmentShader_raycast_ext.glsl.h"
#endif //USE_2D_TF

#include "renderer/raycasting/vertexShader_raycast.glsl.h"

#include <msv/util/hlp.h>
#include <msv/types/box.h>

#include <eq/util/frameBufferObject.h>

#include <eq/client/compositor.h>
#include <eq/client/log.h>
#include <eq/client/image.h>
#include <eq/fabric/pixelViewport.h>

#include <bitset>


namespace massVolVis
{

namespace
{
void _grawBox( const Box_f& box, GLuint drawFace );
}

RendererRaycast2::RendererRaycast2()
    : RendererBase()
    , _isInitialized( false )
    , _volumeName( 0 )
    , _preintRGBA( 0 )
    , _preintSDA( 0 )
    , _precision( 2 )
    , _nearPlaneClipperPtr( new NearPlaneClipper() )
    , _ortho( false )
    , _treeDepth( 0 )
    , _tfHist( 0xFFFFFFFF )
{
    _preintData.resize( 256*256*4 );
}


RendererRaycast2::~RendererRaycast2()
{
    // deinit TF
    if( _preintRGBA )
        glDeleteTextures( 1, &_preintRGBA );
    if( _preintSDA )
        glDeleteTextures( 1, &_preintSDA );
}


bool RendererRaycast2::init( GLuint texture3D )
{
    if( _isInitialized )
        return true;

    const GLEWContext* context = glewGetContext();
    if( !context )
    {
        LBERROR << "Can't get GLEW context" << std::endl;
        return false;
    }

#ifdef USE_2D_TF
    if( !_loadShaders( vertexShader_raycast_glsl, fragmentShader_raycast_glsl ))
#else // ifdef USE_2D_TF
    if( !_loadShaders( vertexShader_raycast_glsl, fragmentShader_raycast_ext_glsl ))
#endif // USE_2D_TF
        return false;

    if( !_cubeFacesFboPtr.get() )
    {
        _cubeFacesFboPtr = FboAutoPtr( new eq::util::FrameBufferObject( context, GL_TEXTURE_RECTANGLE_ARB ));
        LBCHECK( _cubeFacesFboPtr->init( 64, 64, GL_RGB, 0, 0 ));
        _cubeFacesFboPtr->unbind();
    }
    if( !_frameBufferFboPtr.get() )
    {
        _frameBufferFboPtr = FboAutoPtr( new eq::util::FrameBufferObject( context, GL_TEXTURE_RECTANGLE_ARB ));
        LBCHECK( _frameBufferFboPtr->init( 64, 64, GL_RGBA, 0, 0 ));
        _frameBufferFboPtr->unbind();
    }

    _volumeName = texture3D;

    _isInitialized = true;

    return true;
}


void RendererRaycast2::frameInit( const FrameInitParameters& fInitParams )
{
    if( !_isInitialized )
        return;

    if( _preintRGBA == 0  || _preintSDA == 0  || _volumeName == 0 )
        return;

    _nearPlaneClipperPtr->setup( fInitParams.projectionMV );

    _treeDepth = fInitParams.treeDepth;

    glDisable( GL_LIGHTING );
    glDisable( GL_DEPTH_TEST );

    LBASSERT( glewGetContext( ));

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


    tParamNameGL = glGetUniformLocationARB( shader, "samplingDistance" );
    glUniform1fARB( tParamNameGL, 1.f / 64.f );
//    glUniform1fARB( tParamNameGL, 1.f / 128.f );
//    glUniform1fARB( tParamNameGL, 1.f / 256.f );

    tParamNameGL = glGetUniformLocationARB( shader, "drawBB" );
    glUniform1iARB( tParamNameGL, fInitParams.drawBB ? 1 : 0 );

    tParamNameGL = glGetUniformLocationARB(  shader,  "voxSize"       );
    glUniform3fARB( tParamNameGL, fInitParams.voxelSize.w,
                                  fInitParams.voxelSize.h,
                                  fInitParams.voxelSize.d  ); //f-shader


    tParamNameGL = glGetUniformLocationARB(  shader,  "taint"         );
    glUniform4fARB( tParamNameGL,   fInitParams.taintColor.r(),
                                    fInitParams.taintColor.g(),
                                    fInitParams.taintColor.b(),
                                    fInitParams.taintColor.a()  ); //f-shader

    tParamNameGL = glGetUniformLocationARB(  shader,  "shininess"     );
    glUniform1fARB( tParamNameGL,  50.0f         ); //f-shader

    // rotate viewPosition in the opposite direction of model rotation
    // to keep light position constant but not recalculate normals 
    // in the fragment shader
    tParamNameGL = glGetUniformLocationARB(  shader,  "viewVec"       );
    glUniform3fARB( tParamNameGL, fInitParams.viewVector.x(),
                                  fInitParams.viewVector.y(),
                                  fInitParams.viewVector.z() ); //f-shader

/*
//    tParamNameGL = glGetUniformLocationARB(  shader,  "perspProj"     );
//    glUniform1fARB( tParamNameGL,  _ortho ? 0.0f : 1.0f ); //v-shader

    tParamNameGL = glGetUniformLocationARB(  shader,  "normalsQuality");
    glUniform1iARB( tParamNameGL, fInitParams.normalsQuality ); //f-shader
*/
    // Disable shader
    glUseProgramObjectARB( 0 );

    if( !_cubeFacesFboPtr->resize( fInitParams.screenSize.x, fInitParams.screenSize.y ))
        LBERROR << "FBO ERROR: " << _cubeFacesFboPtr->getError() << std::endl;

    if( !_frameBufferFboPtr->resize( fInitParams.screenSize.x, fInitParams.screenSize.y ))
        LBERROR << "FBO ERROR: " << _frameBufferFboPtr->getError() << std::endl;

/*    _frameBufferFboPtr->getColorTextures()[0]->copyFromFrameBuffer(
            GL_RGBA, eq::fabric::PixelViewport(
            0, 0, fInitParams.screenSize.x, fInitParams.screenSize.y ));
//*/
    _frameBufferFboPtr->bind();
    if( _frameBufferFboPtr->getError() != co::ERROR_NONE )
        LBERROR << "FBO ERROR: " << _frameBufferFboPtr->getError() << std::endl;
    EQ_GL_CALL( glClearColor( 0.f, 0.f, 0.f, 1.f ) );
    EQ_GL_CALL( glClear( GL_COLOR_BUFFER_BIT ));
    _frameBufferFboPtr->unbind();

//*/
}


void RendererRaycast2::frameFinish()
{
    EQ_GL_CALL( glFinish() );

    if( !_isInitialized )
        return;

    if( _preintRGBA == 0 || _preintSDA == 0 || _volumeName == 0 )
        return;

    _drawFbFBO();
}

namespace
{


void _dumpDebug( const eq::util::Texture& texture, const GLEWContext* glewContext )
{
    static eq::Image tmpImg;
    static uint32_t counter = 0;
    std::ostringstream ss;
    ss << "~/_img_" << ++counter;

    tmpImg.reset();
    tmpImg.setPixelViewport( eq::PixelViewport( 0, 0, texture.getWidth(), texture.getHeight( )));
    tmpImg.allocDownloader( eq::Frame::BUFFER_COLOR,
//                             EQ_COMPRESSOR_TRANSFER_RGBA_TO_BGR,
                             EQ_COMPRESSOR_TRANSFER_RGBA_TO_RGBA,
                             glewContext );
    tmpImg.clearPixelData( eq::Frame::BUFFER_COLOR );

    uint8_t* dst  = tmpImg.getPixelPointer( eq::Frame::BUFFER_COLOR );
    texture.download( dst );

    LBWARN << "Dumping image: " << ss.str( ) << std::endl;
    tmpImg.writeImages( ss.str( ));
}
}


// TODO: render blocks that are given by drawCoords, not blockCoords!
//       should adjust color values in _drawBox accordingly
void RendererRaycast2::renderBrick
(
    const RenderNode&   renderNode,
    const Box_f&  ,//      drawCoords, - see TODO above
    const Vec3_f&       memCoords,
    const uint32_t//      numberOfSlices  - nor used currently, requires adjustment of density in the f-shader
)
{
    if( !_isInitialized )
        return;

    if( _preintRGBA == 0  ||  _preintSDA == 0  || _volumeName == 0 )
        return;
//*
    // Prepare exit coordinates through drawing back-faces to fbo
    _cubeFacesFboPtr->bind();
    if( _cubeFacesFboPtr->getError() != co::ERROR_NONE )
        LBERROR << "FBO ERROR: " << _cubeFacesFboPtr->getError() << std::endl;
    EQ_GL_CALL( glClearColor( 0, 0, 0, 0 ) );
    EQ_GL_CALL( glClear( GL_COLOR_BUFFER_BIT ));
    _grawBox( renderNode.coords, GL_BACK );
    _cubeFacesFboPtr->unbind();

    const eq::util::Texture* texture = _cubeFacesFboPtr->getColorTextures()[0];
//    _dumpDebug( *texture, glewGetContext());

    _frameBufferFboPtr->bind();
    if( _frameBufferFboPtr->getError() != co::ERROR_NONE )
        LBERROR << "FBO ERROR: " << _frameBufferFboPtr->getError() << std::endl;

    // Put data to the shader
    GLint tParamNameGL;

    // Put output coordinates to the texture
    GLhandleARB shader = _getShaderProgram();
    EQ_GL_CALL( glUseProgramObjectARB( shader ));

    float r = 0.f;
    float g = 0.f;
    float b = 0.f;

    tParamNameGL = glGetUniformLocationARB(  shader,  "bbColor" );
    glUniform3fARB( tParamNameGL, r, g, b ); //v-shader

    tParamNameGL = glGetUniformLocationARB(  shader,  "memOffset" );
    glUniform3fARB( tParamNameGL, memCoords.x, memCoords.y, memCoords.z ); //v-shader

    tParamNameGL = glGetUniformLocationARB(  shader,  "pvpOffset" );
//    glUniform2fARB( tParamNameGL, renderNode.screenRect.s.x, renderNode.screenRect.s.y ); //v-shader
    glUniform2fARB( tParamNameGL, 0.f, 0.f ); //v-shader

    EQASSERT( renderNode.treeLevel > 0 );
    tParamNameGL = glGetUniformLocationARB(  shader,  "samplingDelta" );

#if 1 // use opacity correction

#if 1// ROOT_IS_BASE
    LBASSERT( _treeDepth-renderNode.treeLevel >= 0 );
    glUniform1fARB( tParamNameGL, float( 1 << (_treeDepth-renderNode.treeLevel)) ); //f-shader
#else
    LBASSERT( renderNode.treeLevel-1 >= 0 );
    glUniform1fARB( tParamNameGL, 1. / float( 1 << (renderNode.treeLevel-1)) ); //f-shader
# endif // ROOT_IS_BASE

#else
    glUniform1fARB( tParamNameGL, 1. ); //f-shader
#endif

    EQ_GL_CALL( glActiveTextureARB( GL_TEXTURE4 ));
    const eq::util::Texture* fbTmp = _frameBufferFboPtr->getColorTextures()[0];
    fbTmp->bind();
    fbTmp->applyZoomFilter( eq::FILTER_LINEAR );
    fbTmp->applyWrap();
    tParamNameGL = glGetUniformLocationARB( shader, "frameBuffer" );
    EQ_GL_CALL( glUniform1iARB( tParamNameGL, 4 )); //f-shader

    EQ_GL_CALL( glActiveTextureARB( GL_TEXTURE3 ));
    texture->bind();
    texture->applyZoomFilter( eq::FILTER_LINEAR );
    texture->applyWrap();
    tParamNameGL = glGetUniformLocationARB( shader, "backCoordsTexture" );
    EQ_GL_CALL( glUniform1iARB( tParamNameGL, 3 )); //f-shader

    // Put Volume data to the shader
    EQ_GL_CALL( glActiveTextureARB( GL_TEXTURE2 ));
#ifdef USE_2D_TF
    EQ_GL_CALL( glBindTexture( GL_TEXTURE_2D, _preintRGBA )); //preintegrated values
#else // USE_2D_TF
    EQ_GL_CALL( glBindTexture( GL_TEXTURE_1D, _preintRGBA )); //preintegrated values
#endif // USE_2D_TF
    tParamNameGL = glGetUniformLocationARB( shader, "preIntRGBA" );
    glUniform1iARB( tParamNameGL, 2 ); //f-shader

    // Put Volume data to the shader
    EQ_GL_CALL( glActiveTextureARB( GL_TEXTURE1 ));
#ifdef USE_2D_TF
    EQ_GL_CALL( glBindTexture( GL_TEXTURE_2D, _preintSDA )); //preintegrated values
#else // USE_2D_TF
    EQ_GL_CALL( glBindTexture( GL_TEXTURE_1D, _preintSDA )); //preintegrated values
#endif // USE_2D_TF
    tParamNameGL = glGetUniformLocationARB( shader, "preIntSDA" );
    glUniform1iARB( tParamNameGL, 1 ); //f-shader

    // Activate last because it has to be the active texture
    EQ_GL_CALL( glActiveTextureARB( GL_TEXTURE0 ));
    EQ_GL_CALL( glBindTexture( GL_TEXTURE_3D, _volumeName ));
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
    tParamNameGL = glGetUniformLocationARB( shader, "volume" );
    glUniform1iARB( tParamNameGL, 0 ); //f-shader

    _grawBox( renderNode.coords, GL_FRONT );

    if( size_t validPoints = _nearPlaneClipperPtr->intersectBrick( renderNode ) )
    {
        glTextureBarrierNV(); // not sure if this is necessary
        glBegin( GL_POLYGON );
        for( size_t i = 0; i < validPoints; ++i )
        {
            const Vec3_f& coord = _nearPlaneClipperPtr->getCoordinateForPoint( i );
            const Vec3_f& color = _nearPlaneClipperPtr->getColorForPoint( i );
            glColor3f(  color.x, color.y, color.z );
            glVertex3f( coord.x, coord.y, coord.z );
        }
        glEnd();
        glBegin( GL_POLYGON );
        for( size_t i1 = validPoints; i1 >= 1 ; --i1 )
        {
            size_t i = i1-1;
            const Vec3_f& coord = _nearPlaneClipperPtr->getCoordinateForPoint( i );
            const Vec3_f& color = _nearPlaneClipperPtr->getColorForPoint( i );
            glColor3f(  color.x, color.y, color.z );
            glVertex3f( coord.x, coord.y, coord.z );
        }
        glEnd();
    }

    EQ_GL_CALL( glUseProgramObjectARB( 0 ));

    _frameBufferFboPtr->unbind();
//    glTextureBarrierNV(); // not sure if this is necessary

//    const eq::util::Texture* texture1 = _frameBufferFboPtr->getColorTextures()[0];
//    _dumpDebug( *texture1, glewGetContext());
//*/

/*
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    _grawBox( renderNode.coords, GL_BACK );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    if( size_t validPoints = _nearPlaneClipperPtr->intersectBrick( renderNode ) )
    {
        glTextureBarrierNV(); // not sure if this is necessary
        glBegin( GL_POLYGON );
        for( size_t i = 0; i < validPoints; ++i )
        {
            const Vec3_f& coord = _nearPlaneClipperPtr->getCoordinateForPoint( i );
            const Vec3_f& color = _nearPlaneClipperPtr->getColorForPoint( i );
            glColor3f(  color.x, color.y, color.z );
            glVertex3f( coord.x, coord.y, coord.z );
        }
        glEnd();
        glBegin( GL_POLYGON );
        for( size_t i1 = validPoints; i1 >= 1 ; --i1 )
        {
            size_t i = i1-1;
            const Vec3_f& coord = _nearPlaneClipperPtr->getCoordinateForPoint( i );
            const Vec3_f& color = _nearPlaneClipperPtr->getColorForPoint( i );
            glColor3f(  color.x, color.y, color.z );
            glVertex3f( coord.x, coord.y, coord.z );
        }
        glEnd();
    }

    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    _grawBox( renderNode.coords, GL_BACK );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
//*/
}


void RendererRaycast2::_drawFbFBO()
{
    const eq::PixelViewport pvp = eq::fabric::PixelViewport( 0, 0, _frameBufferFboPtr->getWidth(), _frameBufferFboPtr->getHeight() );

    eq::Compositor::setupAssemblyState( pvp, glewGetContext() );

    glEnable( GL_BLEND );
    LBASSERT( GLEW_EXT_blend_func_separate );
    glBlendFuncSeparate( GL_ONE, GL_SRC_ALPHA, GL_ZERO, GL_SRC_ALPHA );

    const eq::util::Texture* texture = _frameBufferFboPtr->getColorTextures()[0];

    texture->bind();
    texture->applyZoomFilter( eq::FILTER_LINEAR  );
    texture->applyWrap();

    glDepthMask( false );
    glDisable( GL_LIGHTING );
    glEnable( GL_TEXTURE_RECTANGLE_ARB );

    glColor3f( 1.0f, 1.0f, 1.0f );

    const float startX = 0.0f;
    const float endX   = pvp.w;
    const float startY = 0.0f;
    const float endY   = pvp.h;

    glBegin( GL_QUADS );
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3f( startX, startY, 0.0f );

        glTexCoord2f( float( pvp.w ), 0.0f );
        glVertex3f( endX, startY, 0.0f );

        glTexCoord2f( float( pvp.w ), float( pvp.h ));
        glVertex3f( endX, endY, 0.0f );

        glTexCoord2f( 0.0f, float( pvp.h ));
        glVertex3f( startX, endY, 0.0f );
    glEnd();

    // restore state
    glDisable( GL_TEXTURE_RECTANGLE_ARB );
    glDepthMask( true );

    glDisable( GL_BLEND );

    eq::Compositor::resetAssemblyState();
}


void RendererRaycast2::initTF( const TransferFunction& tf )
{
    using hlpFuncs::myClip;

    LBASSERT( tf.rgba.size() == 256*4 );

// update TF histogram
    const uint32_t rangeSpan = 256 / 32; // quantize to 32 bit
          uint32_t counter = 0;
          uint32_t sum = 0;
    _tfHist = 0;
    for( int i = 3; i < 256*4; i+=4 ) // check all alphas
    {
        sum += tf.rgba[i];
        ++counter;
        if( counter == rangeSpan )
        {
            counter = 0;
            _tfHist <<= 1;
            if( sum > rangeSpan/2 ) // some small threshold for alphas
                _tfHist |= 1;
            sum = 0;
        }
    }
    std::cout << "New TF hist: " << std::bitset<32>(_tfHist) << std::endl;

#ifdef USE_2D_TF
// create preintegration table
    const byte* rgba = &tf.rgba[0];

    float rInt[256]; rInt[0] = 0.;
    float gInt[256]; gInt[0] = 0.;
    float bInt[256]; bInt[0] = 0.;
    float aInt[256]; aInt[0] = 0.;

    // Creating SAT (Summed Area Tables) from averaged neighbouring RGBA values
    for( int i=1; i<256; i++ )
    {
        // average Alpha from two neighbouring TF values
        const float tauc =    ( rgba[(i-1)*4+3] + rgba[i*4+3] ) / 2. / 255.;

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

            float rcol, gcol, bcol, acol;
            if( smax != smin )
            {
                const float factorA   = 1. /  (float)(smax-smin);
                const float factorRgb = 1. / ((float)(smax-smin)*255.);
                rcol =       ( rInt[smax] - rInt[smin] ) * factorRgb;
                gcol =       ( gInt[smax] - gInt[smin] ) * factorRgb;
                bcol =       ( bInt[smax] - bInt[smin] ) * factorRgb;
                acol = exp( -( aInt[smax] - aInt[smin] ) * factorA);
            } else
            {
                const int    index  = smin*4;
                const float factorA   = 1./ 255.;
                const float factorRgb = 1./(255.*255.);
                rcol =       rgba[index+0] * rgba[index+3] * factorRgb;
                gcol =       rgba[index+1] * rgba[index+3] * factorRgb;
                bcol =       rgba[index+2] * rgba[index+3] * factorRgb;
                acol = exp(                 -rgba[index+3] * factorA );
            }
            _preintData[lookupindex++] = myClip<float>( rcol, 0.f, 1.f );
            _preintData[lookupindex++] = myClip<float>( gcol, 0.f, 1.f );
            _preintData[lookupindex++] = myClip<float>( bcol, 0.f, 1.f );
            _preintData[lookupindex++] = myClip<float>( acol, 0.f, 1.f );
        }
    }

    // initial texture initialization
    if( _preintRGBA == 0 )
    {
        GLuint preintName = 0;
        glGenTextures( 1, &preintName );
        glBindTexture( GL_TEXTURE_2D, preintName );
        glTexImage2D(  GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0,
                       GL_RGBA, GL_FLOAT, &_preintData[0] );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR        );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR        );

        _preintRGBA  = preintName;
    }else
    {
        // else update existing data
        glBindTexture( GL_TEXTURE_2D, _preintRGBA );
        glTexImage2D(  GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0,
                       GL_RGBA, GL_FLOAT, &_preintData[0] );
    }

    // Creating SAT (Summed Area Tables) from averaged neighbouring SDA values
    LBASSERT( tf.sda.size() == 256*3 );
    const byte* sda = &tf.sda[0];

    for( int i=1; i<256; i++ )
    {
        // SAT of average SDAs from two neighbouring TF values 
        rInt[i] = rInt[i-1] + ( sda[(i-1)*3+0] + sda[i*3+0] )/2.;
        gInt[i] = gInt[i-1] + ( sda[(i-1)*3+1] + sda[i*3+1] )/2.;
        bInt[i] = bInt[i-1] + ( sda[(i-1)*3+2] + sda[i*3+2] )/2.;
    }

    lookupindex = 0;
    for( int sb=0; sb<256; sb++ )
    {
        for( int sf=0; sf<256; sf++ )
        {
            int smin, smax;
            if( sb<sf ) { smin=sb; smax=sf; }
            else        { smin=sf; smax=sb; }

            float s, d, a;
            if( smax != smin )
            {
                const float factor = 1. / ((float)(smax-smin)*255.);
                s = ( rInt[smax] - rInt[smin] ) * factor;
                d = ( gInt[smax] - gInt[smin] ) * factor;
                a = ( bInt[smax] - bInt[smin] ) * factor;
            } else
            {
                const int    index  = smin*3;
                const float factor = 1./255.;
                s = sda[index+0] * factor;
                d = sda[index+1] * factor;
                a = sda[index+2] * factor;
            }
            _preintData[lookupindex++] = myClip( s, 0.f, 1.f );
            _preintData[lookupindex++] = myClip( d, 0.f, 1.f );
            _preintData[lookupindex++] = myClip( a, 0.f, 1.f );
        }
    }

    // initial texture initialization
    if( _preintSDA == 0 )
    {
        GLuint preintName = 0;
        glGenTextures( 1, &preintName );
        glBindTexture( GL_TEXTURE_2D, preintName );
        glTexImage2D(  GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0,
                       GL_RGB, GL_FLOAT, &_preintData[0] );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR        );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR        );

        _preintSDA = preintName;
    }else
    {
        // else update existing data
        glBindTexture( GL_TEXTURE_2D, _preintSDA );
        glTexImage2D(  GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0,
                       GL_RGB, GL_FLOAT, &_preintData[0] );

    }
#else
    if( _preintRGBA == 0 )
    {
        GLuint preintName = 0;
        glGenTextures( 1, &preintName );
        glBindTexture( GL_TEXTURE_1D, preintName );
        glTexImage1D(  GL_TEXTURE_1D, 0, GL_RGBA, 256, 0,
                       GL_RGBA, GL_UNSIGNED_BYTE, &tf.rgba[0] );

        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR        );
        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR        );

        _preintRGBA  = preintName;
    }else
    {
        // else update existing data
        glBindTexture( GL_TEXTURE_1D, _preintRGBA );
        glTexImage1D(  GL_TEXTURE_1D, 0, GL_RGBA, 256, 0,
                       GL_RGBA, GL_UNSIGNED_BYTE, &tf.rgba[0] );
    }

    LBASSERT( tf.sda.size() == 256*3 );
    if( _preintSDA == 0 )
    {
        GLuint preintName = 0;
        glGenTextures( 1, &preintName );
        glBindTexture( GL_TEXTURE_1D, preintName );
        glTexImage1D(  GL_TEXTURE_1D, 0, GL_RGB, 256, 0,
                       GL_RGB, GL_UNSIGNED_BYTE, &tf.sda[0] );

        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR        );
        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR        );

        _preintSDA = preintName;
    }else
    {
        // else update existing data
        glBindTexture( GL_TEXTURE_1D, _preintSDA );
        glTexImage1D(  GL_TEXTURE_1D, 0, GL_RGB, 256, 0,
                       GL_RGB, GL_UNSIGNED_BYTE, &tf.sda[0] );

    }
#endif

}

namespace
{
void _grawBox( const Box_f& box, GLuint drawFace )
{
    GLuint cullFace = drawFace == GL_FRONT ? GL_BACK : GL_FRONT;

    glEnable( GL_CULL_FACE );
    glCullFace( cullFace );

    const float norm = -1.0f;
//
// Cube verticies layout:
//
//    7   5
//  1   3
//
//    6   4
//  0   2
//
    glBegin( GL_QUADS );
        glNormal3f(  norm, 0.0f, 0.0f );
        glColor3f( 0, 0, 0 );
        glVertex3f( box.s.x, box.s.y, box.s.z ); // 0
        glColor3f( 0, 0, 1 );
        glVertex3f( box.s.x, box.s.y, box.e.z ); // 1
        glColor3f( 0, 1, 1 );
        glVertex3f( box.s.x, box.e.y, box.e.z ); // 3
        glColor3f( 0, 1, 0 );
        glVertex3f( box.s.x, box.e.y, box.s.z ); // 2

        glNormal3f( 0.0f,  -norm, 0.0f );
        glColor3f( 0, 1, 0 );
        glVertex3f( box.s.x, box.e.y, box.s.z ); // 2
        glColor3f( 0, 1, 1 );
        glVertex3f( box.s.x, box.e.y, box.e.z ); // 3
        glColor3f( 1, 1, 1 );
        glVertex3f( box.e.x, box.e.y, box.e.z ); // 5
        glColor3f( 1, 1, 0 );
        glVertex3f( box.e.x, box.e.y, box.s.z ); // 4

        glNormal3f( -norm, 0.0f, 0.0f );
        glColor3f( 1, 1, 0 );
        glVertex3f( box.e.x, box.e.y, box.s.z ); // 4
        glColor3f( 1, 1, 1 );
        glVertex3f( box.e.x, box.e.y, box.e.z ); // 5
        glColor3f( 1, 0, 1 );
        glVertex3f( box.e.x, box.s.y, box.e.z ); // 7
        glColor3f( 1, 0, 0 );
        glVertex3f( box.e.x, box.s.y, box.s.z ); // 6

        glNormal3f( 0.0f,  norm, 0.0f );
        glColor3f( 1, 0, 0 );
        glVertex3f( box.e.x, box.s.y, box.s.z ); // 6
        glColor3f( 1, 0, 1 );
        glVertex3f( box.e.x, box.s.y, box.e.z ); // 7
        glColor3f( 0, 0, 1 );
        glVertex3f( box.s.x, box.s.y, box.e.z ); // 1
        glColor3f( 0, 0, 0 );
        glVertex3f( box.s.x, box.s.y, box.s.z ); // 0

        glNormal3f( 0.0f, 0.0f, -norm );
        glColor3f( 0, 0, 1 );
        glVertex3f( box.s.x, box.s.y, box.e.z ); // 1
        glColor3f( 1, 0, 1 );
        glVertex3f( box.e.x, box.s.y, box.e.z ); // 7
        glColor3f( 1, 1, 1 );
        glVertex3f( box.e.x, box.e.y, box.e.z ); // 5
        glColor3f( 0, 1, 1 );
        glVertex3f( box.s.x, box.e.y, box.e.z ); // 3

        glNormal3f( 0.0f, 0.0f,  norm );
        glColor3f( 0, 0, 0 );
        glVertex3f( box.s.x, box.s.y, box.s.z ); // 0
        glColor3f( 0, 1, 0 );
        glVertex3f( box.s.x, box.e.y, box.s.z ); // 2
        glColor3f( 1, 1, 0 );
        glVertex3f( box.e.x, box.e.y, box.s.z ); // 4
        glColor3f( 1, 0, 0 );
        glVertex3f( box.e.x, box.s.y, box.s.z ); // 6
    glEnd();
    glDisable( GL_CULL_FACE );
}
}


}//namespace massVolVis
