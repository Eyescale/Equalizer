
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
    ,_useCg         ( true )
    ,_bgColor       ( 0.0f, 0.0f, 0.0f, 1.0f ) 
    ,_model         ( 0 )
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

    if( _model )
        delete _model;

    Node* node = static_cast< Node* >( getNode( ));
    _model = new Model( node->getInitData().getFilename() );

    if( !_model || !_model->getLoadingResult() )
        return false;

    createSlicesHexagonsList( _model->getResolution() * 2, _slicesListID );


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


static void calcAndPutDataForSlicesClippingToShader
(
    vmml::Matrix4d&     modelviewM,
    vmml::Matrix3d&     modelviewITM,
    double              sliceDistance,
    eq::Range           range,
    eqCgShaders         cgShaders,
    GLhandleARB         glslShader,
    bool                useCgShaders
)
{
    vmml::Vector4d camPosition( 
                        modelviewITM.ml[3],
                        modelviewITM.ml[7],
                        1.0               ,
                        1.0                  );

    vmml::Vector4d viewVec( 
                        -modelviewM.ml[ 2],
                        -modelviewM.ml[ 6],
                        -modelviewM.ml[10],
                         0.0                );

    double zRs = -1+2.*range.start;
    double zRe = -1+2.*range.end;

    //rendering parallelepipid's verteces 
    vmml::Vector4d vertices[8]; 
    vertices[0] = vmml::Vector4d(-1.0,-1.0,zRs, 1.0);
    vertices[1] = vmml::Vector4d( 1.0,-1.0,zRs, 1.0);
    vertices[2] = vmml::Vector4d(-1.0, 1.0,zRs, 1.0);
    vertices[3] = vmml::Vector4d( 1.0, 1.0,zRs, 1.0);

    vertices[4] = vmml::Vector4d(-1.0,-1.0,zRe, 1.0);
    vertices[5] = vmml::Vector4d( 1.0,-1.0,zRe, 1.0);
    vertices[6] = vmml::Vector4d(-1.0, 1.0,zRe, 1.0);
    vertices[7] = vmml::Vector4d( 1.0, 1.0,zRe, 1.0);


    float dMaxDist = viewVec.dot( vertices[0] );
    int nMaxIdx = 0;
    for( int i = 1; i < 8; i++ )
    {
        float dist = viewVec.dot( vertices[i] );
        if ( dist > dMaxDist)
        {
            dMaxDist = dist;
            nMaxIdx = i;
        }
    }

    const int nSequence[8][8] = {
        {7,3,5,6,1,2,4,0},
        {6,2,4,7,0,3,5,1},
        {5,1,4,7,0,3,6,2},
        {4,0,5,6,1,2,7,3},
        {3,1,2,7,0,5,6,4},
        {2,0,3,6,1,4,7,5},
        {1,0,3,5,2,4,7,6},
        {0,1,2,4,3,5,6,7},
    };

    double dStartDist   = viewVec.dot( vertices[nSequence[nMaxIdx][0]] );
    double dS           = ceil( dStartDist/sliceDistance );
    dStartDist          = dS * sliceDistance;
    
    
    const float sequence[64] = {
        0, 1, 4, 2, 3, 5, 6, 7,
        1, 0, 3, 5, 4, 2, 7, 6,
        2, 0, 6, 3, 1, 4, 7, 5,
        3, 1, 2, 7, 5, 0, 6, 4,
        4, 0, 5, 6, 2, 1, 7, 3,
        5, 1, 7, 4, 0, 3, 6, 2,
        6, 2, 4, 7, 3, 0, 5, 1,
        7, 3, 6, 5, 1, 2, 4, 0 };

    const float e1[24] = {
        0, 1, 4, 4,
        1, 0, 1, 4,
        0, 2, 5, 5,
        2, 0, 2, 5,
        0, 3, 6, 6, 
        3, 0, 3, 6 };

    const float e2[24] = {
        1, 4, 7, 7,
        5, 1, 4, 7,
        2, 5, 7, 7,
        6, 2, 5, 7,
        3, 6, 7, 7,
        4, 3, 6, 7 };

    float shaderVertices[24];
    for( int i=0; i<8; i++ )
        for( int j=0; j<3; j++)
            shaderVertices[ i*3+j ] = vertices[i][j];

    if( useCgShaders )
    {
#ifdef CG_INSTALLED
        // temporal variables to store cgGetNamedParameter
        // and glGetUniformLocationARB values, I need this 
        // only to make someone people be happy with their 
        // old school 80 characters per line and not to 
        // make code look completely ugly :(
        CGparameter tParamNameCg;
        CGprogram   m_vProg = cgShaders.cgVertex->get_program();
    
        tParamNameCg = cgGetNamedParameter( m_vProg, "vecVertices" );
        cgGLSetParameterArray3f( tParamNameCg, 0,  8, shaderVertices );

        tParamNameCg = cgGetNamedParameter( m_vProg, "sequence"    );
        cgGLSetParameterArray1f( tParamNameCg, 0, 64, sequence     );

        tParamNameCg = cgGetNamedParameter( m_vProg, "v1"          );
        cgGLSetParameterArray1f( tParamNameCg, 0, 24, e1           );

        tParamNameCg = cgGetNamedParameter( m_vProg, "v2"          );
        cgGLSetParameterArray1f( tParamNameCg, 0, 24, e2           );

        tParamNameCg = cgGetNamedParameter( m_vProg, "vecView"     );
        cgGLSetParameter3dv(     tParamNameCg,        viewVec.xyzw );

        tParamNameCg = cgGetNamedParameter( m_vProg, "frontIndex"  );
        cgGLSetParameter1f(      tParamNameCg, static_cast<float>( nMaxIdx ) );

        tParamNameCg = cgGetNamedParameter( m_vProg, "dPlaneStart" );
        cgGLSetParameter1d( tParamNameCg,             dStartDist   );

        tParamNameCg = cgGetNamedParameter( m_vProg, "dPlaneIncr"  );
        cgGLSetParameter1d(      tParamNameCg,        sliceDistance );
#endif
    }
    else // glsl shaders
    {
        GLint tParamNameGL;
    
        tParamNameGL = glGetUniformLocationARB( glslShader, "vecVertices" );
        glUniform3fvARB( tParamNameGL,  8, shaderVertices                 );

        tParamNameGL = glGetUniformLocationARB( glslShader, "sequence"    );
        glUniform1fvARB( tParamNameGL, 64, sequence                       );

        tParamNameGL = glGetUniformLocationARB( glslShader, "v1"          );
        glUniform1fvARB( tParamNameGL, 24, e1                             );

        tParamNameGL = glGetUniformLocationARB( glslShader, "v2"          );
        glUniform1fvARB( tParamNameGL, 24, e2                             );

        tParamNameGL = glGetUniformLocationARB( glslShader, "vecView"     );
        glUniform3fARB( tParamNameGL, viewVec.x, viewVec.y, viewVec.z     );

        tParamNameGL = glGetUniformLocationARB(  glslShader, "dPlaneStart" );
        glUniform1fARB(     tParamNameGL,   dStartDist                     );

        tParamNameGL = glGetUniformLocationARB( glslShader, "frontIndex"  );
        glUniform1iARB( tParamNameGL, static_cast<GLint>( nMaxIdx )       );

        tParamNameGL = glGetUniformLocationARB( glslShader, "dPlaneIncr"  );
        glUniform1fARB( tParamNameGL, sliceDistance                        );
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


static void setCamera( const FrameData::Data &data, const VolumeScales &scales )
{
    glTranslatef(  data.translation.x, data.translation.y, data.translation.z );

    glMultMatrixf( data.rotation.ml );

    glScalef( scales.W, scales.H, scales.D );
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


static void renderSlices( const GLuint slicesListID )
{
    glCallList( slicesListID );
/*
    for( int s = 0; s < numberOfSlices; ++s )
        for( float l=0; l < 1.5; l++ )
        {
            tParamNameCg = cgGetNamedParameter( m_fProg, "lines" );
            cgGLSetParameter1f( tParamNameCg, l );
            glBegin( GL_POLYGON );
            for( int i = 0; i < 6; ++i )
                glVertex2i( i, numberOfSlices-1-s );
            glEnd();
        }
*/
}


void Channel::frameDraw( const uint32_t frameID )
{
    // Setup frustum
    eq::Channel::frameDraw( frameID );

    // Save ID and range for compositing
    _curFrData.lastRange    = getRange();
    _curFrData.frameID      = frameID;

    // Setup / load textures
    if( !_model->createTextures( _tex3D, _preintName, _curFrData.lastRange ) )
        return;

    const double sliceDistance = 3.6/ ( _model->getResolution() * 2 );

    // Setup camera and lighting
    const Pipe*      pipe      = static_cast<Pipe*>( getPipe( ));
    const FrameData& frameData = pipe->getFrameData();

    setCamera( frameData.data, _model->volScales );
    setLights();

    // Enable shaders
    eqCgShaders cgShaders  = pipe->getShaders();
    GLhandleARB glslShader = pipe->getShader();

    enableShaders( cgShaders, glslShader, _useCg );
    
    // Calculate and put necessary data to shaders 
    vmml::Matrix4d  modelviewM;     // modelview matrix
    vmml::Matrix3d  modelviewITM;   // modelview inversed transposed matrix
    _calcMVandITMV( modelviewM, modelviewITM );

    calcAndPutDataForSlicesClippingToShader
    (
        modelviewM, modelviewITM, sliceDistance, _curFrData.lastRange,
        cgShaders, glslShader, _useCg 
    );

    putTextureCoordinatesModifyersToShader
    (
        _model->TD,
        cgShaders, glslShader, _useCg 
    );

    // Fill volume data
    putVolumeDataToShader
    ( 
        _preintName, _tex3D, sliceDistance,
        cgShaders, glslShader, _useCg 
    );

    //Render slices
    glEnable(GL_BLEND);
#ifdef COMPOSE_MODE_NEW
    glBlendFuncSeparateEXT( GL_ONE, GL_SRC_ALPHA, GL_ZERO, GL_SRC_ALPHA );
#else
    glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
#endif

    renderSlices( _slicesListID );

    glDisable(GL_BLEND);

    checkError( "error during rendering " );

    disableShaders( cgShaders, glslShader, _useCg );
    
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
    
    if( _model )
    {
        vmml::Matrix4f scale( 
            _model->volScales.W, 0, 0, 0,
            0, _model->volScales.H, 0, 0,
            0, 0, _model->volScales.D, 0,
            0, 0,                   0, 1 );

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
        uint            nFrames   = frames.size();
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
        pvp += imagePVP;
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

    coveredPVP ^= getPixelViewport();

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
