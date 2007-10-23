
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com>
                 2007       Maxim Makhinya
   All rights reserved. */

#include "channel.h"

#include "frameData.h"
#include "initData.h"
#include "node.h"
#include "pipe.h"
#include "window.h"
#include "shader.h"
#include "hlp.h"


//#define DYNAMIC_NEAR_FAR 
#ifndef M_SQRT3
#  define M_SQRT3    1.7321f   /* sqrt(3) */
#endif
#ifndef M_SQRT3_2
#  define M_SQRT3_2  0.86603f  /* sqrt(3)/2 */
#endif

namespace eVolve
{

using namespace eqBase;
using namespace std;


Channel::Channel()
    :eq::Channel()
    ,_bgColor       ( 0.0f, 0.0f, 0.0f, 1.0f ) 
    ,_slicesListID  ( 0 )
{
    _curFrData.frameID = 0;
}


static void checkError( std::string msg ) 
{
    const GLenum error = glGetError();
    if (error != GL_NO_ERROR)
        EQERROR << msg << " GL Error: " << error << endl;
}

static void createSlicesHexagonsList( int num, GLuint &listId )
{
    if( listId != 0 )
        glDeleteLists( listId, 1 );

    listId = glGenLists(1);

    glNewList( listId, GL_COMPILE );

    for( int s = 0; s < num; ++s )
    {
        glBegin( GL_POLYGON );
        for( int i = 0; i < 6; ++i )
            glVertex2i( i, num-1-s );
        glEnd();
    }

    glEndList();
}

bool Channel::configInit( const uint32_t initID )
{
    EQINFO << "Init channel initID " << initID << " ptr " << this << endl;

    // chose projection type
    _perspective = true;

    const Pipe* pipe = static_cast<Pipe*>( getPipe( ));

    const Model* model = pipe->getModel();
    if( !model )
    {
        EQERROR << "model is not loaded" << std::endl;
        return false;
    }

    const Node*     node             = static_cast< Node* >( getNode( ));
    const InitData& initData         = node->getInitData();

    EQASSERT( initData.getPrecision() != 0 );
    createSlicesHexagonsList( model->getResolution() * initData.getPrecision(),
                              _slicesListID );

#ifndef DYNAMIC_NEAR_FAR
    setNearFar( 0.0001f, 10.0f );
#endif
    return true;
}


void Channel::applyFrustum() const
{
    const vmml::Frustumf& frustum = getFrustum();

    if( _perspective )
    {
        glFrustum( frustum.left, frustum.right, frustum.bottom, frustum.top,
                   frustum.nearPlane, frustum.farPlane );
    }else
    {
        double zc = ( 1.0 + frustum.farPlane/frustum.nearPlane ) / 3.0;
        glOrtho( 
            frustum.left   * zc,
            frustum.right  * zc,
            frustum.bottom * zc,
            frustum.top    * zc,
            frustum.nearPlane, 
            frustum.farPlane      );
    }

    EQVERB << "Apply " << frustum << endl;
}


void Channel::frameClear( const uint32_t frameID )
{
    applyBuffer();
    applyViewport();

#ifdef COMPOSE_MODE_NEW
    if( getRange().isFull() )
        glClearColor( _bgColor.r, _bgColor.g, _bgColor.b, _bgColor.a );
    else
        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
#else
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
#endif

    glClear( GL_COLOR_BUFFER_BIT );
}


static void putClippingDataToShader
(
    const SliceClipper& sliceClipper,
    const eqCgShaders&  cgShaders,
    const GLhandleARB&  glslShader,
    const bool          useCgShaders
)
{
    if( useCgShaders )
    {
#ifdef CG_INSTALLED
        // temporal variables to store cgGetNamedParameter
        // and glGetUniformLocationARB values, I need this 
        // only to make someone people be happy with their 
        // old school 80 characters per line and not to 
        // make code look completely ugly :(
        CGparameter tNameCg;
        CGprogram   m_vProg = cgShaders.cgVertex->get_program();

        tNameCg = cgGetNamedParameter( m_vProg, "vecVertices"                );
        cgGLSetParameterArray3f( tNameCg, 0,  8, sliceClipper.shaderVertices );

        tNameCg = cgGetNamedParameter( m_vProg, "sequence"                   );
        cgGLSetParameterArray1f( tNameCg, 0, 64, SliceClipper::sequence       );

        tNameCg = cgGetNamedParameter( m_vProg, "v1"                         );
        cgGLSetParameterArray1f( tNameCg, 0, 24, sliceClipper.v1             );

        tNameCg = cgGetNamedParameter( m_vProg, "v2"                         );
        cgGLSetParameterArray1f( tNameCg, 0, 24, sliceClipper.v2             );

        tNameCg = cgGetNamedParameter( m_vProg, "vecView"                    );
        cgGLSetParameter3dv(     tNameCg,        sliceClipper.viewVec.xyzw );

        tNameCg = cgGetNamedParameter( m_vProg, "dPlaneStart"                );
        cgGLSetParameter1d(      tNameCg,        sliceClipper.planeStart     );

        tNameCg = cgGetNamedParameter( m_vProg, "frontIndex"                 );
        cgGLSetParameter1f(      tNameCg,
                                static_cast<float>( sliceClipper.frontIndex ));
#endif
    }
    else // glsl shaders
    {
        GLint tNameGL;
    
        tNameGL = glGetUniformLocationARB( glslShader, "vecVertices"        );
        glUniform3fvARB( tNameGL,  8, sliceClipper.shaderVertices           );

        tNameGL = glGetUniformLocationARB( glslShader, "sequence"           );
        glUniform1fvARB( tNameGL, 64, sliceClipper.sequence                 );

        tNameGL = glGetUniformLocationARB( glslShader, "v1"                 );
        glUniform1fvARB( tNameGL, 24, sliceClipper.v1                       );

        tNameGL = glGetUniformLocationARB( glslShader, "v2"                 );
        glUniform1fvARB( tNameGL, 24, sliceClipper.v2                       );

        tNameGL = glGetUniformLocationARB( glslShader, "vecView"            );
        glUniform3fARB(  tNameGL,     sliceClipper.viewVec.x,
                                      sliceClipper.viewVec.y,
                                      sliceClipper.viewVec.z                );

        tNameGL = glGetUniformLocationARB(  glslShader, "dPlaneStart"       );
        glUniform1fARB(  tNameGL,     sliceClipper.planeStart               );

        tNameGL = glGetUniformLocationARB( glslShader, "frontIndex"    );
        glUniform1iARB( tNameGL, static_cast<GLint>(sliceClipper.frontIndex));
    }
}



static void putTextureCoordinatesModifyersToShader
(
    DataInTextureDimensions&    TD, 
    eqCgShaders                 cgShaders,
    GLhandleARB                 glslShader,
    bool                        useCgShaders
)
{
    if( useCgShaders )
    {
#ifdef CG_INSTALLED
        CGparameter tParamNameCg;
        CGprogram   m_vProg = cgShaders.cgVertex->get_program();

        tParamNameCg = cgGetNamedParameter(  m_vProg, "W"   );
        cgGLSetParameter1d( tParamNameCg,   TD.W            );

        tParamNameCg = cgGetNamedParameter(  m_vProg, "H"   );
        cgGLSetParameter1d( tParamNameCg,   TD.H            );

        tParamNameCg = cgGetNamedParameter(  m_vProg, "D"   );
        cgGLSetParameter1d( tParamNameCg,   TD.D            );

        tParamNameCg = cgGetNamedParameter(  m_vProg, "Do"  );
        cgGLSetParameter1d( tParamNameCg,   TD.Do           );

        tParamNameCg = cgGetNamedParameter(  m_vProg, "Db"  );
        cgGLSetParameter1d( tParamNameCg,   TD.Db           );
#endif
    }
    else // glsl shaders
    {
    }
}


static void putVolumeDataToShader
(
    GLuint      preIntID,
    GLuint      volumeID,
    double      sliceDistance,
    eqCgShaders cgShaders,
    GLhandleARB glslShader,
    bool        useCgShaders
)
{
    if( useCgShaders )
    {
#ifdef CG_INSTALLED
        CGparameter tParamNameCg;
        CGprogram   m_vProg = cgShaders.cgVertex->get_program();
        CGprogram   m_fProg = cgShaders.cgFragment->get_program();

        glActiveTexture( GL_TEXTURE1 );
        glBindTexture( GL_TEXTURE_2D, preIntID ); //preintegrated values

        tParamNameCg = cgGetNamedParameter( m_fProg,"preInt"  );
        cgGLSetTextureParameter(    tParamNameCg   , preIntID );
        cgGLEnableTextureParameter( tParamNameCg              );

        // Activate last because it has to be the active texture
        glActiveTexture( GL_TEXTURE0 ); 
        glBindTexture( GL_TEXTURE_3D, volumeID ); //gx, gy, gz, val
        glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

        tParamNameCg = cgGetNamedParameter( m_fProg, "volume" );
        cgGLSetTextureParameter(    tParamNameCg, volumeID    );
        cgGLEnableTextureParameter( tParamNameCg              );

        tParamNameCg = cgGetNamedParameter( m_vProg, "sliceDistance" );
        cgGLSetParameter1f( tParamNameCg, sliceDistance  );

        tParamNameCg = cgGetNamedParameter( m_fProg, "shininess"     );
        cgGLSetParameter1f( tParamNameCg, 20.0f         );
#endif
    }
    else // glsl shaders
    {
        GLint tParamNameGL;

        glActiveTexture( GL_TEXTURE1 );
        glBindTexture( GL_TEXTURE_2D, preIntID ); //preintegrated values
        tParamNameGL = glGetUniformLocationARB( glslShader, "preInt" );
        glUniform1iARB( tParamNameGL,  1    ); //f-shader

        // Activate last because it has to be the active texture
        glActiveTexture( GL_TEXTURE0 ); 
        glBindTexture( GL_TEXTURE_3D, volumeID ); //gx, gy, gz, val
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER,GL_LINEAR    );

        tParamNameGL = glGetUniformLocationARB(  glslShader,  "volume"        );
        glUniform1iARB( tParamNameGL ,  0            ); //f-shader

        tParamNameGL = glGetUniformLocationARB(  glslShader,  "sliceDistance" );
        glUniform1fARB( tParamNameGL,  sliceDistance ); //v-shader

        tParamNameGL = glGetUniformLocationARB(  glslShader,  "shininess"     );
        glUniform1fARB( tParamNameGL,  20.0f         ); //f-shader
    }
}


static void setLights()
{
    GLfloat lightAmbient[]  = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat lightDiffuse[]  = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat lightSpecular[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat lightPosition[] = {0.0f, 0.0f, 1.0f, 0.0f};

    glLightfv( GL_LIGHT0, GL_AMBIENT,  lightAmbient  );
    glLightfv( GL_LIGHT0, GL_DIFFUSE,  lightDiffuse  );
    glLightfv( GL_LIGHT0, GL_SPECULAR, lightSpecular );
    glLightfv( GL_LIGHT0, GL_POSITION, lightPosition );

/*    glPushMatrix(); //Rotate lights
      {
          glLoadIdentity();
          vmml::Matrix4f r = frameData.data.rotation;
          r.ml[1] = -r.ml[1];
          r.ml[2] = -r.ml[2];
          r.ml[4] = -r.ml[4];
          r.ml[6] = -r.ml[6];
          r.ml[8] = -r.ml[8];
          r.ml[9] = -r.ml[9];

        vmml::Matrix4f ri; r.getInverse(r);
        glMultMatrixf( ri.ml );
        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    }
    glPopMatrix();
*/
}


static void setCamera( const FrameData::Data &data, 
                       const VolumeScaling &scaling )
{
    glTranslatef(  data.translation.x, data.translation.y, data.translation.z );

    glMultMatrixf( data.rotation.ml );

    glScalef( scaling.W, scaling.H, scaling.D );
}


static void enableShaders
(   
          eqCgShaders cgShaders,
    const GLhandleARB glslShader,
    const bool        useCgShaders
)
{
    if( useCgShaders )
    {
#ifdef CG_INSTALLED
        cgShaders.cgVertex->use();
        cgShaders.cgFragment->use();
#endif
    }
    else
    {
        glUseProgramObjectARB( glslShader );
    }
}


static void disableShaders
(   
          eqCgShaders cgShaders,
    const GLhandleARB glslShader,
    const bool        useCgShaders
)
{
    if( useCgShaders )
    {
#ifdef CG_INSTALLED
        cgShaders.cgVertex->unbind_and_disable();
        cgShaders.cgFragment->unbind_and_disable();
#endif
    }
    else
    {    
        glUseProgramObjectARB( 0 );
    }
}

static void renderSlices
( 
    const GLuint        slicesListID,
    const SliceClipper& sliceClipper,
    const bool          useSimpleShader
)
{
    if( useSimpleShader )
    {
        int numberOfSlices = 
                    static_cast<int>( 3.6 / sliceClipper.sliceDistance );

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
        return;
    }
    // else normal shaders
     
    glCallList( slicesListID );
}

void Channel::frameDraw( const uint32_t frameID )
{
    // Setup frustum
    eq::Channel::frameDraw( frameID );

    // Save ID and range for compositing
    _curFrData.lastRange    = getRange();
    _curFrData.frameID      = frameID;

    // Setup / load textures
    const Pipe* pipe = static_cast<Pipe*>( getPipe( ));

    Model* model = pipe->getModel();

    VolumeInfo volumeInfo;

    EQASSERT( model );
    if( !model || !model->getVolumeInfo( volumeInfo, _curFrData.lastRange ))
    {
        EQERROR << "Can't get volume data" << endl;
        return;
    }

    const Node*           node = static_cast< Node* >( getNode( ));
    const InitData&   initData = node->getInitData();
    const uint32_t   precision = initData.getPrecision();
    const double sliceDistance = 3.6 / ( model->getResolution() * precision );

    // Setup camera and lighting
    const FrameData& frameData = pipe->getFrameData();
    
    setCamera( frameData.data, volumeInfo.volScaling );
    setLights();

    // Enable shaders
    const bool      useCg            = !initData.useGLSL();
    eqCgShaders     cgShaders        = pipe->getShaders();
    GLhandleARB     glslShader       = pipe->getShader();
    bool            useSimpleShaders = pipe->isSimpleShaderUsed();

    enableShaders( cgShaders, glslShader, useCg );
    
    // Calculate and put necessary data to shaders 
    vmml::Matrix4d  modelviewM;     // modelview matrix
    vmml::Matrix3d  modelviewITM;   // modelview inversed transposed matrix
    _calcMVandITMV( modelviewM, modelviewITM );

    putTextureCoordinatesModifyersToShader
    (
        volumeInfo.TD,
        cgShaders, glslShader, useCg 
    );

    // Fill volume data
    putVolumeDataToShader
    ( 
        volumeInfo.preint, volumeInfo.volume, sliceDistance,
        cgShaders, glslShader, useCg 
    );

    _sliceClipper.updatePerFrameInfo
    (
        modelviewM, modelviewITM, sliceDistance, _curFrData.lastRange
    );


    if( !useSimpleShaders )
        putClippingDataToShader( _sliceClipper, cgShaders, glslShader, useCg );
    
    //Render slices
    glEnable(GL_BLEND);
#ifdef COMPOSE_MODE_NEW
	const eq::Window* window = getWindow();
	const eq::GLFunctions* gl = window->getGLFunctions();
    gl->blendFuncSeparate( GL_ONE, GL_SRC_ALPHA, GL_ZERO, GL_SRC_ALPHA );
#else
    glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
#endif

    renderSlices( _slicesListID,  _sliceClipper, useSimpleShaders );

    glDisable(GL_BLEND);

    checkError( "error during rendering " );

    disableShaders( cgShaders, glslShader, useCg );
    
    //Draw logo
    const eq::Viewport& vp = getViewport();
    if( _curFrData.lastRange.isFull() && vp.isFullScreen( ))
       _drawLogo();
}

struct Frame
{
    Frame( eq::Frame* f=0, eq::Image* i=0, eq::Range r=eq::Range(0.f, 0.f) )
    : frame( f ), image( i ), range( r ) {}

    eq::Range getRange() const 
    { return frame ? frame->getRange() : range; }

    eq::Frame* frame;
    eq::Image* image;
    
    eq::Range  range;
};

static bool cmpRangesDec(const Frame& frame1, const Frame& frame2)
{
    return frame1.getRange().start < frame2.getRange().start;
}

static bool cmpRangesInc(const Frame& frame1, const Frame& frame2)
{
    return frame1.getRange().start > frame2.getRange().start;
}

const FrameData& Channel::_getFrameData() const
{
    const Pipe*      pipe      = static_cast<Pipe*>( getPipe( ));
    return pipe->getFrameData();
}

void Channel::_calcMVandITMV(
    vmml::Matrix4d& modelviewM,
    vmml::Matrix3d& modelviewITM ) const
{
    const FrameData& frameData = _getFrameData();

    const Pipe*  pipe = static_cast<Pipe*>( getPipe( ));
    const Model* model = pipe->getModel();
    if( model )
    {
        const VolumeScaling& volScaling = model->getVolumeScaling();
        
        vmml::Matrix4f scale( 
            volScaling.W, 0, 0, 0,
            0, volScaling.H, 0, 0,
            0, 0, volScaling.D, 0,
            0, 0,            0, 1 );

        modelviewM = scale * frameData.data.rotation;
    }
    modelviewM.setTranslation( frameData.data.translation );

    modelviewM = getHeadTransform() * modelviewM;

    //calculate inverse transposed matrix
    vmml::Matrix4d modelviewIM;
    modelviewM.getInverse( modelviewIM );
    modelviewITM = modelviewIM.getTransposed();
}

void Channel::_orderFrames( vector< Frame >& frames )
{
    if( !_perspective ) // parallel/ortho projection
    {
        const bool orientation = _getFrameData().data.rotation.ml[10] < 0;
        sort( frames.begin(), frames.end(),
              orientation ? cmpRangesDec : cmpRangesInc );
        return;
    }

    //perspective projection
    vmml::Matrix4d  modelviewM;     // modelview matrix
    vmml::Matrix3d  modelviewITM;   // modelview inversed transposed matrix
    _calcMVandITMV( modelviewM, modelviewITM );

    vmml::Vector3d norm = modelviewITM * vmml::Vector3d( 0.0, 0.0, 1.0 );
    norm.normalize();

    sort( frames.begin(), frames.end(), cmpRangesDec );

    // cos of angle between normal and vectors from center
    vector<double> dotVals;

    // of projection to the middle of slices' boundaries
    for( vector< Frame >::const_iterator i = frames.begin();
         i != frames.end(); ++i )
    {
        const Frame& frame = *i;
        double       px    = -1.0 + frame.getRange().start*2.0;

        vmml::Vector4d pS = 
            modelviewM * vmml::Vector4d( 0.0, 0.0, px , 1.0 );
            
        dotVals.push_back( norm.dot( pS.getNormalizedVector3() ) );
    }

    const vmml::Vector4d pS = modelviewM * vmml::Vector4d( 0.0, 0.0, 1.0, 1.0 );
    dotVals.push_back( norm.dot( pS.getNormalizedVector3() ) );

    //check if any slices need to be rendered in rederse order
    size_t minPos = 0xffffffffu;
    for( size_t i=0; i<dotVals.size()-1; i++ )
        if( dotVals[i] < 0 && dotVals[i+1] < 0 )
            minPos = static_cast< int >( i );

    if( minPos < frames.size() )
    {
        uint32_t        nFrames   = frames.size();
        vector< Frame > framesTmp = frames;

        //Copy slices that should be rendered first
        memcpy( &frames[0], &framesTmp[minPos+1],
                (nFrames-minPos-1)*sizeof( Frame ) );

        //Copy sliced that shouls be rendered last in reversed order
        for( size_t i=0; i<=minPos; i++ )
            frames[ nFrames-i-1 ] = framesTmp[i];
    }
}


static void _expandPVP( eq::PixelViewport& pvp, 
                        const vector< eq::Image* >& images,
                        const vmml::Vector2i& offset )
{
    for( vector< eq::Image* >::const_iterator i = images.begin();
         i != images.end(); ++i )
    {
        const eq::PixelViewport imagePVP = (*i)->getPixelViewport() + offset;
        pvp.merge( imagePVP );
    }
}

#define SOLID_BG

void Channel::clearViewport( const eq::PixelViewport &pvp )
{
    glViewport( pvp.x, pvp.y, pvp.w, pvp.h );
    glScissor(  pvp.x, pvp.y, pvp.w, pvp.h );

#ifndef NDEBUG
    if( getenv( "EQ_TAINT_CHANNELS" ))
    {
        const vmml::Vector3ub color = getUniqueColor();
        glClearColor( color.r/255., color.g/255., color.b/255., 1. );
    }
#else 
#ifdef SOLID_BG
#ifdef COMPOSE_MODE_NEW
    if( getRange().isFull() )
        glClearColor( _bgColor.r, _bgColor.g, _bgColor.b, _bgColor.a );
    else
        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
//    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
#else
    glClearColor( 0.5f, 0.0f, 0.0f, 1.0f );
#endif
#endif //SOLID_BG
#endif //NDEBUG

    glClear( GL_COLOR_BUFFER_BIT );

    applyViewport();
}

void Channel::frameAssemble( const uint32_t frameID )
{
//    EQWARN << getRange() << " " << _curFrData.lastRange << endl;
    
    const bool composeOnly  = frameID != _curFrData.frameID || 
                              _curFrData.lastRange.isFull();
    
    _startAssemble();

    const vector< eq::Frame* >& frames  = getInputFrames();
    eq::PixelViewport coveredPVP;

    vector< Frame > dbFrames;

    // Make sure all frames are ready and gather some information on them
    for( vector< eq::Frame* >::const_iterator i = frames.begin();
         i != frames.end(); ++i )
    {
        eq::Frame* frame = *i;
        frame->waitReady( );

        const eq::Range& range = frame->getRange();
        if( range.isFull() ) // 2D frame, assemble directly
            frame->startAssemble();
        else
        {
            dbFrames.push_back( Frame( frame ) );
            _expandPVP( coveredPVP, frame->getImages(), frame->getOffset() );
        }
    }

    coveredPVP.intersect( getPixelViewport( ));

    if( dbFrames.empty( ))
    {
        _finishAssemble();
        return;
    }

    //calculate correct frames sequence
    if( !composeOnly )
        dbFrames.push_back( Frame( 0, &_image, _curFrData.lastRange ) );
        
    _orderFrames( dbFrames );


    glEnable( GL_BLEND );

#ifdef COMPOSE_MODE_NEW
    glBlendFunc( GL_ONE, GL_SRC_ALPHA );
#else
    glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
#endif

    //check if current frame in proper position, redback if not
    if( !composeOnly )
    {
#ifndef SOLID_BG
        if( !dbFrames.back().frame )
            dbFrames.pop_back();
        else
#endif //SOLID_BG
            if( coveredPVP.hasArea() )
            {
                _image.setFormat(eq::Frame::BUFFER_COLOR, GL_RGBA   );
                _image.setType(  eq::Frame::BUFFER_COLOR, GL_UNSIGNED_BYTE );

                _image.startReadback(eq::Frame::BUFFER_COLOR, coveredPVP );

#ifdef __APPLE__
                //clear part of a screen
                clearViewport( coveredPVP );
#endif
                _image.syncReadback();
            }
    }

    // blend DB frames in order
    while( !dbFrames.empty() )
    {
        Frame frame = dbFrames.back();
        dbFrames.pop_back();

        if( frame.frame )
            frame.frame->startAssemble();
        else
            if( coveredPVP.hasArea() )
            {
                EQASSERT( frame.image == &_image );
                
                vmml::Vector2i offset( coveredPVP.x, coveredPVP.y );
                _image.startAssemble( eq::Frame::BUFFER_COLOR, offset ); 
                _image.syncAssemble();
            }
    }

    _finishAssemble();
}

void Channel::_startAssemble()
{
    applyBuffer();
    applyViewport();
    setupAssemblyState();
}   

void Channel::_finishAssemble()
{
    const vector< eq::Frame* >&  frames = getInputFrames();

    for( vector< eq::Frame* >::const_iterator i = frames.begin();
         i != frames.end(); ++i )
    {
        eq::Frame* frame = *i;
        frame->syncAssemble();
    }

    _drawLogo();
    resetAssemblyState();
}


void Channel::frameReadback( const uint32_t frameID )
{
    // Drop depth buffer flag from all outframes
    const vector< eq::Frame* >& frames = getOutputFrames();
    for( vector< eq::Frame* >::const_iterator i = frames.begin(); 
         i != frames.end(); ++i )
    {
        (*i)->disableBuffer( eq::Frame::BUFFER_DEPTH );
    }

    eq::Channel::frameReadback( frameID );
}

void Channel::_drawLogo()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    const Window*  window      = static_cast<Window*>( getWindow( ));
    GLuint         texture;
    vmml::Vector2i size;

    window->getLogoTexture( texture, size );
    if( !texture )
        return;

    const eq::PixelViewport pvp    = getPixelViewport();
    const vmml::Vector2i    offset = getPixelOffset();

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( offset.x, offset.x + pvp.w, offset.y, offset.y + pvp.h, 0., 1. );

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glDisable( GL_DEPTH_TEST );
    glDisable( GL_LIGHTING );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glEnable( GL_TEXTURE_RECTANGLE_ARB );
    glBindTexture( GL_TEXTURE_RECTANGLE_ARB, texture );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER,
                     GL_LINEAR );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, 
                     GL_LINEAR );

    glColor3f( 1.0f, 1.0f, 1.0f );
    glBegin( GL_TRIANGLE_STRIP );
    {
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3f( 0.0f, 0.0f, 0.0f );

        glTexCoord2f( size.x, 0.0f );
        glVertex3f( size.x, 0.0f, 0.0f );

        glTexCoord2f( 0.0f, size.y );
        glVertex3f( 0.0f, size.y, 0.0f );

        glTexCoord2f( size.x, size.y );
        glVertex3f( size.x, size.y, 0.0f );
    } glEnd();

    glDisable( GL_TEXTURE_RECTANGLE_ARB );
    glDisable( GL_BLEND );

    glPopAttrib( );
}

}
