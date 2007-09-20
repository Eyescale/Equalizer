
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


//Only for gluErrorString()
#ifdef __APPLE__
#include <GLUT/glut.h>
#else // linux & Win
#include <GL/glu.h>
#endif


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
    ,_useCg          ( true )
    ,_bgColor        ( 0.0f, 0.0f, 0.0f, 1.0f ) 
    ,_model          ( NULL )
    ,_slicesListID ( 0 )
{
    _curFrData.frameID = 0;
}


static void checkError( std::string msg ) 
{
    const GLenum error = glGetError();
    if (error != GL_NO_ERROR)
        EQERROR << msg << " GL Error: " << gluErrorString(error) << endl;
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
    _model = new Model( node->getInitData().getDataFilename() );

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


static void calcAndPutDataForPolygonsClippingToShader
(
    vmml::Matrix4d&     modelviewM,
    vmml::Matrix3d&     modelviewITM,
    double              sliceDistance,
    Range               range,
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


static void setLightParameters()
{
    GLfloat lightAmbient[]  = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat lightDiffuse[]  = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat lightSpecular[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat lightPosition[] = {0.0f, 0.0f, 1.0f, 0.0f};

    glLightfv( GL_LIGHT0, GL_AMBIENT,  lightAmbient  );
    glLightfv( GL_LIGHT0, GL_DIFFUSE,  lightDiffuse  );
    glLightfv( GL_LIGHT0, GL_SPECULAR, lightSpecular );
    glLightfv( GL_LIGHT0, GL_POSITION, lightPosition );
}


static void rotateLights()
{
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

void Channel::frameDraw( const uint32_t frameID )
{
    const Pipe*      pipe      = static_cast<Pipe*>( getPipe( ));
    const FrameData& frameData = pipe->getFrameData();

    _curFrData.lastRange    = getRange();  //save range for compositing
    _curFrData.frameID      = frameID;

    if( _model )
        _model->createTextures( _tex3D, _preintName, _curFrData.lastRange );

    applyBuffer();
    applyViewport();

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    applyFrustum();

    if( _model )
    {
        const uint32_t  numberOfSlices = _model->getResolution() * 2;
        double          sliceDistance  = 3.6/numberOfSlices;
        
        if( _prvNumberOfSlices != numberOfSlices )
        {
            createSlicesHexagonsList( numberOfSlices, _slicesListID );
            _prvNumberOfSlices = numberOfSlices;
        }

        setLightParameters();

        // Matrix World
        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();
        applyHeadTransform();

        glTranslatef(  frameData.data.translation.x,
                       frameData.data.translation.y,
                       frameData.data.translation.z );

        glMultMatrixf( frameData.data.rotation.ml );

        glScalef( 
            _model->volScales.W,
            _model->volScales.H, 
            _model->volScales.D  );

        rotateLights();

        // Enable shaders
        eqCgShaders cgShaders  = pipe->getShaders();
        GLhandleARB glslShader = pipe->getShader();
        
        if( _useCg )
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

        // Calculate and put necessary data to shaders 
        vmml::Matrix4d  modelviewM;     // modelview matrix
        vmml::Matrix3d  modelviewITM;   // modelview inversed transposed matrix
        _calcMVandITMV( modelviewM, modelviewITM );

        calcAndPutDataForPolygonsClippingToShader
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

        glCallList( _slicesListID );
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

        glDisable(GL_BLEND);

        checkError( "error during rendering " );

        //Disable shaders
        if( _useCg )
        {
#ifdef CG_INSTALLED
            cgShaders.cgVertex->unbind_and_disable();
            cgShaders.cgFragment->unbind_and_disable();
#endif
        }
        else
        {    
            glUseProgramObjectARB( NULL );
        }
    }
    else
    {
        glColor3f( 1.f, 1.f, 0.f );
        glNormal3f( 0.f, -1.f, 0.f );
        glBegin( GL_TRIANGLE_STRIP );
        glVertex3f(  .25f, 0.f,  .25f );
        glVertex3f(  .25f, 0.f, -.25f );
        glVertex3f( -.25f, 0.f,  .25f );
        glVertex3f( -.25f, 0.f, -.25f );
        glEnd();
    }
//    _drawLogo();
}

static bool cmpRangesDec(const Range& r1, const Range& r2)
{
    return r1.start < r2.start;
}

static bool cmpRangesInc(const Range& r1, const Range& r2)
{
    return r1.start > r2.start;
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

void Channel::arrangeFrames( vector<Range>& ranges )
{
    if( _perspective )
    {//perspective projection
        vmml::Matrix4d  modelviewM;     // modelview matrix
        vmml::Matrix3d  modelviewITM;   // modelview inversed transposed matrix
        _calcMVandITMV( modelviewM, modelviewITM );

        vmml::Vector3d norm = modelviewITM * vmml::Vector3d( 0.0, 0.0, 1.0 );
        norm.normalize();

        sort( ranges.begin(), ranges.end(), cmpRangesDec );

        // cos of angle between normal and vectors from center
        vector<double> dotVals;
        
        // of projection to the middle of slices' boundaries
        dotVals.reserve( ranges.size()+1 );

        for( uint i=0; i<ranges.size(); i++ )
        {
            double px = -1.0 + ranges[i].start*2.0;

            vmml::Vector4d pS = 
                modelviewM * vmml::Vector4d( 0.0, 0.0, px , 1.0 );
                
            dotVals.push_back( norm.dot( pS.getNormalizedVector3() ) );
        }

        vmml::Vector4d pS = modelviewM * vmml::Vector4d( 0.0, 0.0, 1.0, 1.0 );
        dotVals.push_back( norm.dot( pS.getNormalizedVector3() ) );

        //check if any slices need to be rendered in rederse orded
        int minPos = -1;
        for( uint i=0; i<dotVals.size()-1; i++ )
            if( dotVals[i] < 0 && dotVals[i+1] < 0 )
                minPos = static_cast<int>(i);

        if( minPos >= 0 )
        {
            uint          rangesNum = ranges.size();
            vector<Range> rangesTmp = ranges;

            //Copy slices that should be rendered first
            memcpy( 
                &ranges[0], 
                &rangesTmp[minPos+1],
                (rangesNum-minPos-1)*sizeof(Range)  );

            //Copy sliced that shouls be rendered last in reversed order
            for( int i=0; i<=minPos; i++ )
                ranges[ rangesNum-i-1 ] = rangesTmp[i];
        }
    }else
    {//parallel projection
        const bool orientation = _getFrameData().data.rotation.ml[10] < 0;
        sort(
            ranges.begin(), 
            ranges.end(), 
            orientation ? cmpRangesDec : cmpRangesInc );
    }
}


void IntersectViewports(        PixelViewport & pvp, 
                          const vector<Image*>& vecImages,
                          const vmml::Vector2i& offset      )
{
    if( vecImages.size() < 1 )
    {
        pvp.invalidate();
        return;
    }

    PixelViewport overalPVP = vecImages[0]->getPixelViewport() + offset;

    for( uint i=1; i<vecImages.size(); i++ )
        overalPVP += vecImages[i]->getPixelViewport() + offset;

    pvp ^= overalPVP;
}


void Channel::_clearPixelViewPorts( const vector<Image*>& vecImages, 
                                    const vmml::Vector2i& offset     )
{
    for( uint i=0; i<vecImages.size(); i++ )
        clearViewport( vecImages[i]->getPixelViewport() + offset );
}


#define SOLID_BG

void Channel::clearViewport( const PixelViewport &pvp )
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
    
    applyBuffer();
    applyViewport();
    setupAssemblyState();

    const vector<Frame*>& frames  = getInputFrames();

    vector<Frame*> unusedFrames = frames;

    for( vector<Frame*>::iterator i = unusedFrames.begin();
         i != unusedFrames.end(); ++i )
    {
        Frame* frame = *i;
        frame->waitReady( );
    }

    //All frames is ready

    //fill ranges - it should be supplied by server actualy 
    eq::PixelViewport curPVP = getPixelViewport();

    vector<Range> ranges;
    for( uint k=0; k<unusedFrames.size(); k++ )
    {
        Range curRange = unusedFrames[k]->getRange();

        // Add only DB related slices,
        // screen decomposition should be composed as is
        if( !curRange.isFull() )
        {
            ranges.push_back( curRange );
            IntersectViewports( curPVP, 
                                unusedFrames[k]->getImages(),
                                unusedFrames[k]->getOffset()  );
        }
    }

    //calculate correct frames sequence
    if( !composeOnly ) ranges.push_back( _curFrData.lastRange );
    arrangeFrames( ranges );

    //check if current frame in proper position, redback if not
    if( !composeOnly )
    {
#ifndef NDEBUG //DEBUG
        if(  _curFrData.lastRange == ranges.back() && 
            !getenv( "EQ_TAINT_CHANNELS" ) )
            ranges.pop_back();
        else
#else
#ifndef SOLID_BG
        if( _curFrData.lastRange == ranges.back() )
            ranges.pop_back();
        else
#endif //SOLID_BG
#endif //NDEBUG
            if( curPVP.hasArea() )
            {
                _curFrameImage.setFormat(    Frame::BUFFER_COLOR, GL_RGBA   );
                _curFrameImage.setType(      Frame::BUFFER_COLOR,
                                             GL_UNSIGNED_BYTE               );

                _curFrameImage.setPixelViewport(                   curPVP   );
                _curFrameImage.startReadback( Frame::BUFFER_COLOR, curPVP   );

                //clear part of a screen
#ifdef __APPLE__
                clearViewport( curPVP );
#endif
            }
    }


    while( !unusedFrames.empty() || !ranges.empty() )
    {
        if( !composeOnly && ( ranges.back() == _curFrData.lastRange ))
        {
            // current range equals to range of original frame
            if( curPVP.hasArea() )
            {
                vmml::Vector2i curOff( curPVP.x, curPVP.y );
                _curFrameImage.startAssemble( Frame::BUFFER_COLOR, curOff ); 
            }
            ranges.pop_back();
        }

        for( vector<Frame*>::iterator i  = unusedFrames.begin(); 
                                      i != unusedFrames.end();
                                      i ++                       )
        {
            Frame* frame = *i;
            Range curRange = frame->getRange();

            if( curRange.isFull() ) //2D screen element, asseble as is
            {
                _clearPixelViewPorts( frame->getImages(), frame->getOffset() );
                frame->startAssemble();
                unusedFrames.erase( i );
                break;
            }
            else
            {
                if( ranges.empty() ) // no ranges to put but some not       
                {                    // full-ranges frames -> error
                
                    EQERROR << "uncounted frame" << endl;
                    unusedFrames.erase( i );
                    break;
                }else
                    if( ranges.back() == curRange ) // current frame has 
                    {                               // proper range
                    
                        frame->startAssemble();
                        unusedFrames.erase( i );
                        ranges.pop_back();
                        break;
                    }
            }
        }
    }

    resetAssemblyState();
}

void Channel::setupAssemblyState()
{
    eq::Channel::setupAssemblyState();

    glEnable( GL_BLEND );

#ifdef COMPOSE_MODE_NEW
    glBlendFunc( GL_ONE, GL_SRC_ALPHA );
#else
    glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
#endif

}


void Channel::frameReadback( const uint32_t frameID )
{
    applyBuffer();
    applyViewport();
    setupAssemblyState();

    const vector<Frame*>& frames = getOutputFrames();

    for( vector<Frame*>::const_iterator iter  = frames.begin(); 
                                        iter != frames.end(); 
                                        iter ++                 )
    {
        //Drop depth buffer flag if present
        (*iter)->disableBuffer( Frame::BUFFER_DEPTH );

        (*iter)->startReadback();
    }

    resetAssemblyState();
}


void Channel::_drawLogo()
{
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
    glEnable( GL_LIGHTING );
    glEnable( GL_DEPTH_TEST );
}

}
