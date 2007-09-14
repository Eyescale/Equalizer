
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
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
: eq::Channel()
, _model(NULL)
,_vertexID(0)
{
    _curFrData.frameID = 0;
}


void checkError( std::string msg ) 
{
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
        EQERROR << msg << " GL Error: " << gluErrorString(error) << endl;
}

#ifdef CG_SHADERS
void createHexagonsList( int num, GLuint &listId )
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
#else
struct quad 
{
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

void createVertexArray( uint32_t numberOfSlices, GLuint &vertexID )
{
    vector<quad> pVertex;
    pVertex.resize( 4*numberOfSlices );

    //draw the slices
    for( uint32_t ii=0; ii<numberOfSlices; ii++ )
    {
        pVertex[4*ii+3].x =  1.8;
        pVertex[4*ii+3].y = -1.8;
        pVertex[4*ii+3].z =  (3.6*ii)/(numberOfSlices-1)-1.8;

        pVertex[4*ii+2].x =  1.8;
        pVertex[4*ii+2].y =  1.8;
        pVertex[4*ii+2].z =  (3.6*ii)/(numberOfSlices-1)-1.8;

        pVertex[4*ii+1].x = -1.8;
        pVertex[4*ii+1].y =  1.8;
        pVertex[4*ii+1].z =  (3.6*ii)/(numberOfSlices-1)-1.8;


        pVertex[4*ii  ].x = -1.8;
        pVertex[4*ii  ].y = -1.8;
        pVertex[4*ii  ].z =  (3.6*ii)/(numberOfSlices-1)-1.8;
    }

    glGenBuffers( 1, &vertexID );
    glBindBuffer( GL_ARRAY_BUFFER, vertexID );
    glBufferData( GL_ARRAY_BUFFER, 3*4*numberOfSlices*sizeof(GLfloat), &pVertex[0], GL_STATIC_DRAW );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    checkError( "creating vertex array" );
}
#endif //CG_SHADERS


bool Channel::configInit( const uint32_t initID )
{
    EQINFO << "Init channel initID " << initID << " ptr " << this << endl;

//chose projection type
    _perspective = true;

    if( _model )
        delete _model;

    Node* node  = (Node*)getNode();
    _model = new Model( node->getInitData().getDataFilename() );


    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

#ifndef DYNAMIC_NEAR_FAR
    setNearFar( 0.0001f, 10.0f );
#endif
    return true;
}


void Channel::applyFrustum() const
{
    const vmml::Frustumf& frustum = _curFrData.frustum;

    if( _perspective )
    {
        glFrustum( frustum.left, frustum.right, frustum.bottom, frustum.top,
                 frustum.nearPlane, frustum.farPlane );
    }else
    {
        glOrtho( frustum.left, frustum.right, frustum.bottom, frustum.top,
                 frustum.nearPlane, frustum.farPlane );
    }

    EQVERB << "Apply " << frustum << endl;
}

void Channel::frameClear( const uint32_t frameID )
{
    applyBuffer();
    applyViewport();

#ifdef COMPOSE_MODE_NEW
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
#else
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
#endif

    glClear( GL_COLOR_BUFFER_BIT );
}


//#define TEXTURE_ROTATION

void Channel::frameDraw( const uint32_t frameID )
{
    const Pipe*      pipe      = static_cast<Pipe*>( getPipe( ));
    const FrameData& frameData = pipe->getFrameData();

    _curFrData.lastRange    = getRange();  //save range for compositing
    _curFrData.frameID      = frameID;
    _curFrData.pvp          = getPixelViewport();

    if( _model )
        _model->createTextures( _tex3D, _preintName, _curFrData.lastRange );

    vmml::FrustumCullerf culler;
    _initFrustum( culler );

    applyBuffer();
    applyViewport();

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    applyFrustum();


    if( _model )
    {
        uint32_t numberOfSlices = _model->getResolution() * 2;
        if( _prvNumberOfSlices != numberOfSlices )
        {
#ifndef TEXTURE_ROTATION 
            createHexagonsList( numberOfSlices, _vertexID );
#else
            createVertexArray(  numberOfSlices, _vertexID );
#endif
            _prvNumberOfSlices = numberOfSlices;
        }

        GLfloat lightAmbient[]  = {0.2f, 0.2f, 0.2f, 1.0f};
        GLfloat lightDiffuse[]  = {0.8f, 0.8f, 0.8f, 1.0f};
        GLfloat lightSpecular[] = {0.8f, 0.8f, 0.8f, 1.0f};
        GLfloat lightPosition[] = {0.0f, 0.0f, 1.0f, 0.0f};

        //set light parameters
        glLightfv( GL_LIGHT0, GL_AMBIENT,  lightAmbient  );
        glLightfv( GL_LIGHT0, GL_DIFFUSE,  lightDiffuse  );
        glLightfv( GL_LIGHT0, GL_SPECULAR, lightSpecular );
        glLightfv( GL_LIGHT0, GL_POSITION, lightPosition );

#ifdef TEXTURE_ROTATION 
        // Rotate 3D texture
        glActiveTexture( GL_TEXTURE0 );
        glMatrixMode( GL_TEXTURE );
        glLoadIdentity();
        glMultMatrixf( frameData.data.rotation.ml );
#endif


/////////////////////////////////////////////////////////////////////////////        

        // Matrix World
        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();
        applyHeadTransform();

        glTranslatef(  frameData.data.translation.x,
                       frameData.data.translation.y,
                       frameData.data.translation.z );
#ifndef TEXTURE_ROTATION 
        glMultMatrixf( frameData.data.rotation.ml );
#endif
        glScalef( _model->volScales.W, _model->volScales.H, _model->volScales.D );

/*        glPushMatrix(); //Rotate lights
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
        checkError("");


        vmml::Matrix4d pMatrix      = _curFrData.modelviewM;
        vmml::Matrix4d matModelView = _curFrData.modelviewIM;

        vmml::Vector4d camPosition = matModelView * vmml::Vector4f( 0.0, 0.0, 0.0, 1.0 );
        camPosition[3] = 1.0;

        vmml::Vector4d viewVec( -pMatrix.ml[2], -pMatrix.ml[6], -pMatrix.ml[10], 0.0 );

        double m_dScaleFactorX = 1.0;
        double m_dScaleFactorY = 1.0;
        double m_dScaleFactorZ = 1.0;
        double m_dSliceDist    = 3.6/numberOfSlices;

        vmml::Vector4d scaledViewVec( viewVec.x*m_dScaleFactorX, viewVec.y*m_dScaleFactorY, viewVec.z*m_dScaleFactorZ, 0.0 ) ;

        double zRs = -1+2*_curFrData.lastRange.start;
        double zRe = -1+2*_curFrData.lastRange.end;

        vmml::Vector4d m_pVertices[8];
        m_pVertices[0] = vmml::Vector4f(-1.0,-1.0,zRs, 1.0);
        m_pVertices[1] = vmml::Vector4f( 1.0,-1.0,zRs, 1.0);
        m_pVertices[2] = vmml::Vector4f(-1.0, 1.0,zRs, 1.0);
        m_pVertices[3] = vmml::Vector4f( 1.0, 1.0,zRs, 1.0);

        m_pVertices[4] = vmml::Vector4f(-1.0,-1.0,zRe, 1.0);
        m_pVertices[5] = vmml::Vector4f( 1.0,-1.0,zRe, 1.0);
        m_pVertices[6] = vmml::Vector4f(-1.0, 1.0,zRe, 1.0);
        m_pVertices[7] = vmml::Vector4f( 1.0, 1.0,zRe, 1.0);


        float dMaxDist = scaledViewVec.dot( m_pVertices[0] );
        int nMaxIdx = 0;
        for( int i = 1; i < 8; i++ )
        {
            float dist = scaledViewVec.dot( m_pVertices[i] );
            if ( dist > dMaxDist)
            {
                dMaxDist = dist;
                nMaxIdx = i;
            }
        }

        //rendering

        // Enable shader
#ifdef CG_SHADERS
        eqCgShaders shaders = pipe->getShaders();
        shaders.cgVertex->use();
        shaders.cgFragment->use();

        CGprogram m_vProg = shaders.cgVertex->get_program();
        CGprogram m_fProg = shaders.cgFragment->get_program();
#else
        GLhandleARB shader = pipe->getShader();
        glUseProgramObjectARB( shader );
#endif


#ifndef TEXTURE_ROTATION 
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

        float vertices[24];
        for(int i = 0; i < 8; ++i)
        {
            vertices[3*i]   = static_cast<double>( m_dScaleFactorX * m_pVertices[i][0] );
            vertices[3*i+1] = static_cast<double>( m_dScaleFactorY * m_pVertices[i][1] );
            vertices[3*i+2] = static_cast<double>( m_dScaleFactorZ * m_pVertices[i][2] );
        }

#ifdef CG_SHADERS
        cgGLSetParameterArray3f( cgGetNamedParameter( m_vProg, "vecVertices" ), 0,  8, vertices );
        cgGLSetParameterArray1f( cgGetNamedParameter( m_vProg, "sequence"    ), 0, 64, sequence );
        cgGLSetParameterArray1f( cgGetNamedParameter( m_vProg, "v1"          ), 0, 24, e1       );
        cgGLSetParameterArray1f( cgGetNamedParameter( m_vProg, "v2"          ), 0, 24, e2       );

        cgGLSetParameter3dv(     cgGetNamedParameter( m_vProg, "vecView"     ), viewVec.xyzw   );
        cgGLSetParameter1f(      cgGetNamedParameter( m_vProg, "frontIndex"  ), float(nMaxIdx) );

        cgGLSetParameter1d(      cgGetNamedParameter( m_vProg, "dPlaneIncr"  ), m_dSliceDist   );
#else
        glUniform3fvARB( glGetUniformLocationARB( shader, "vecVertices"),  8, vertices );
        glUniform1ivARB( glGetUniformLocationARB( shader, "sequence"   ), 64, sequence );
        glUniform1ivARB( glGetUniformLocationARB( shader, "v1"         ), 24, e1       );
        glUniform1ivARB( glGetUniformLocationARB( shader, "v2"         ), 24, e2       );

        glUniform3fARB(  glGetUniformLocationARB( shader, "vecView"    ), viewVec.x, viewVec.y, viewVec.z );
        const GLint frontInd = static_cast<GLint>(nMaxIdx);
        glUniform1iARB(  glGetUniformLocationARB( shader, "frontIndex" ), frontInd     );
        glUniform1fARB(  glGetUniformLocationARB( shader, "dPlaneIncr" ), m_dSliceDist );
#endif //CG_SHADERS

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

        int nMinIdx = 0;
        double dMinDist = (camPosition - m_pVertices[0]).length();

        double dDist;
        for(int v = 1; v < 8; v++)
        {
            dDist = ( camPosition - m_pVertices[v] ).length();
            if (dDist < dMinDist)
            {
                dMinDist = dDist;
                nMinIdx = v;
            }
        }

        double dStartDist   = viewVec.dot( m_pVertices[nSequence[nMaxIdx][0]] );
        double dS           = ceil( dStartDist/m_dSliceDist);
        dStartDist          = dS * m_dSliceDist;

#ifdef CG_SHADERS
        cgGLSetParameter1d(cgGetNamedParameter(  m_vProg, "dPlaneStart" ), dStartDist );

        cgGLSetParameter1d(cgGetNamedParameter(  m_vProg, "W"  ), _model->TD.W  );
        cgGLSetParameter1d(cgGetNamedParameter(  m_vProg, "H"  ), _model->TD.H  );
        cgGLSetParameter1d(cgGetNamedParameter(  m_vProg, "D"  ), _model->TD.D  );
        cgGLSetParameter1d(cgGetNamedParameter(  m_vProg, "Do" ), _model->TD.Do );
        cgGLSetParameter1d(cgGetNamedParameter(  m_vProg, "Db" ), _model->TD.Db );

#else
        glUniform1fARB( glGetUniformLocationARB(  shader, "dPlaneStart" ), dStartDist );
#endif
#endif //TEXTURE_ROTATION 


        // Fill volume data
        {
            glActiveTexture( GL_TEXTURE1 );
            glBindTexture( GL_TEXTURE_2D, _preintName ); //preintegrated values
#ifdef CG_SHADERS
            cgGLSetTextureParameter(    cgGetNamedParameter( m_fProg,"preInt" ), _preintName    );
            cgGLEnableTextureParameter( cgGetNamedParameter( m_fProg,"preInt" )                 );
#else
            glUniform1iARB( glGetUniformLocationARB( shader, "preInt"        ), 1              ); //f-shader
#endif

            glActiveTexture( GL_TEXTURE0 ); // Activate last because it has to be the active texture
            glBindTexture( GL_TEXTURE_3D, _tex3D ); //gx, gy, gz, val
            glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
#ifdef CG_SHADERS
            cgGLSetTextureParameter(    cgGetNamedParameter( m_fProg, "volume" ), _tex3D        );
            cgGLEnableTextureParameter( cgGetNamedParameter( m_fProg, "volume" )                );

            cgGLSetParameter1f(  cgGetNamedParameter( m_vProg, "sliceDistance" ), m_dSliceDist  );
            cgGLSetParameter1f(  cgGetNamedParameter( m_fProg, "shininess"     ), 20.0f         );
#else
            glUniform1iARB( glGetUniformLocationARB(  shader,  "volume"        ), 0             ); //f-shader

            glUniform1fARB( glGetUniformLocationARB(  shader,  "sliceDistance" ), m_dSliceDist  ); //v-shader
            glUniform1fARB( glGetUniformLocationARB(  shader,  "shininess"     ), 20.0f         ); //f-shader
#endif
        }

        //Render slices
        glEnable(GL_BLEND);
#ifdef COMPOSE_MODE_NEW
        glBlendFuncSeparateEXT( GL_ONE, GL_SRC_ALPHA, GL_ZERO, GL_SRC_ALPHA );
#else
        glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
#endif

#ifndef TEXTURE_ROTATION 

        glCallList( _vertexID );

/*  
        for( int s = 0; s < numberOfSlices; ++s )
        {
            cgGLSetParameter1f(  cgGetNamedParameter( m_fProg, "lines" ), 0.0f );
            glBegin( GL_POLYGON );
            for( int i = 0; i < 6; ++i )
                glVertex2i( i, numberOfSlices-1-s );
            glEnd();

            cgGLSetParameter1f(  cgGetNamedParameter( m_fProg, "lines" ), 1.0f );
            glBegin( GL_LINE_LOOP );
            for( int i = 0; i < 6; ++i )
                glVertex2i( i, numberOfSlices-1-s );
            glEnd();
        }
*/

#else
        // Draw slices from vertex array
        glEnableClientState( GL_VERTEX_ARRAY );
        glBindBuffer( GL_ARRAY_BUFFER, _vertexID );
        glVertexPointer( 3, GL_FLOAT, 0, 0 );
        glDrawArrays( GL_QUADS, 0, 4*numberOfSlices );
        glDisableClientState( GL_VERTEX_ARRAY );
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
#endif

        glDisable(GL_BLEND);

        checkError( "error during rendering " );

        //Disable shader
#ifdef CG_SHADERS
        shaders.cgVertex->unbind_and_disable();
        shaders.cgFragment->unbind_and_disable();
#else
        glUseProgramObjectARB( NULL );
#endif

/**/
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

bool cmpRangesDec(const Range& r1, const Range& r2)
{
    return r1.start < r2.start;
}

bool cmpRangesInc(const Range& r1, const Range& r2)
{
    return r1.start > r2.start;
}

const FrameData& Channel::_getFrameData() const
{
    const Pipe*      pipe      = static_cast<Pipe*>( getPipe( ));
    return pipe->getFrameData();
}

void Channel::_calcMVandITMV( vmml::Matrix4d& modelviewM, vmml::Matrix3d& modelviewITM ) const
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

void Channel::arrangeFrames( vector<Range>& ranges, bool composeOnly )
{
    if( _perspective )
    {//perspective projection
        vmml::Matrix4d  modelviewM;     // modelview matrix
        vmml::Matrix3d  modelviewITM;   // modelview inversed transposed matrix
        if( !composeOnly )
        {
            modelviewM      = _curFrData.modelviewM;
            modelviewITM    = _curFrData.modelviewITM;
        }else
            _calcMVandITMV( modelviewM, modelviewITM );

        vmml::Vector3d norm = modelviewITM * vmml::Vector3d( 0.0, 0.0, 1.0 );
        norm.normalize();

        sort( ranges.begin(), ranges.end(), cmpRangesDec );

        vector<double> dotVals;             // cos of angle between normal and vectors from center 
        dotVals.reserve( ranges.size()+1 ); // of projection to the middle of slices' boundaries

        for( uint i=0; i<ranges.size(); i++ )
        {
            double px = -1.0 + ranges[i].start*2.0;

            vmml::Vector4d pS = modelviewM * vmml::Vector4d( 0.0, 0.0, px , 1.0 );
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
            memcpy( &ranges[0], &rangesTmp[minPos+1], (rangesNum-minPos-1)*sizeof(Range) );

            //Copy sliced that shouls be rendered last in reversed order
            for( int i=0; i<=minPos; i++ )
                ranges[ rangesNum-i-1 ] = rangesTmp[i];
        }
    }else
    {//parallel projection
        sort( ranges.begin(), ranges.end(), _getFrameData().data.rotation.ml[10]<0 ? cmpRangesDec : cmpRangesInc );
    }
}


void IntersectViewports( PixelViewport &pvp, const vector<Image*>& vecImages, const vmml::Vector2i& offset )
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

void Channel::_clearPixelViewPorts( const vector<Image*> &vecImages, const vmml::Vector2i& offset )
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
        glClearColor( color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, 1.0f );
    }
#else 
#ifdef SOLID_BG
#ifdef COMPOSE_MODE_NEW
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
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
    const bool composeOnly  = frameID != _curFrData.frameID;
    
    applyBuffer();
    applyViewport();
    setupAssemblyState();

    Pipe*                 pipe    = static_cast<eVolve::Pipe*>( getPipe() );
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
    eq::PixelViewport curPVP = composeOnly ? getPixelViewport() : _curFrData.pvp;

    vector<Range> ranges;
    for( uint k=0; k<unusedFrames.size(); k++ )
    {
        Range curRange = unusedFrames[k]->getRange();

        if( !curRange.isFull() ) // Add only DB related slices, screen decomposition should be composed as is
        {
            ranges.push_back( curRange );
            IntersectViewports( curPVP, unusedFrames[k]->getImages(), unusedFrames[k]->getOffset() );
        }
    }

    //calculate correct frames sequence
    if( !composeOnly ) ranges.push_back( _curFrData.lastRange );
    arrangeFrames( ranges, composeOnly );

    //check if current frame in proper position, redback if not
    if( !composeOnly )
    {
#ifndef NDEBUG
        if( _curFrData.lastRange == ranges.back() && !getenv( "EQ_TAINT_CHANNELS" ) )
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
                _curFrameImage.setFormat(        Frame::BUFFER_COLOR, GL_RGBA          );
                _curFrameImage.setType(          Frame::BUFFER_COLOR, GL_UNSIGNED_BYTE );

                _curFrameImage.setPixelViewport(                      curPVP           );
                _curFrameImage.startReadback(    Frame::BUFFER_COLOR, curPVP           );

                //clear part of a screen
#ifdef __APPLE__
                clearViewport( curPVP );
#endif
            }
    }


    while( !unusedFrames.empty() )
    {
        for( vector<Frame*>::iterator i = unusedFrames.begin(); i != unusedFrames.end(); ++i )
        {
            bool foundMatchedFrame = false;
            Frame* frame = *i;
            Range curRange = frame->getRange();

            if( curRange.isFull() ) //2D screen element, asseble as is
            {
                _clearPixelViewPorts( frame->getImages(), frame->getOffset() );
                frame->startAssemble();
                unusedFrames.erase( i );
                foundMatchedFrame = true;
            }
            else
            {
                if( ranges.empty() ) // no ranges to put but some not full-ranges frames -> error
                {
                    EQERROR << "uncounted frame" << endl;
                    unusedFrames.erase( i );
                    break;
                }else
                    if( ranges.back() == curRange ) // current frame has proper range
                    {
                        frame->startAssemble();
                        unusedFrames.erase( i );
                        ranges.pop_back();
                        foundMatchedFrame = true;
                    }
            }

            if( !composeOnly && ( ranges.back() == _curFrData.lastRange )) // current range equals to range of original frame
            {
                if( curPVP.hasArea() )
                    _curFrameImage.startAssemble( Frame::BUFFER_COLOR, vmml::Vector2i( curPVP.x, curPVP.y ) );

                ranges.pop_back();
                foundMatchedFrame = true;
            }

            if( foundMatchedFrame )
                break;
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

    for( vector<Frame*>::const_iterator iter = frames.begin(); iter != frames.end(); ++iter )
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

void Channel::_initFrustum( vmml::FrustumCullerf& culler )
{
    const Pipe*      pipe      = static_cast<Pipe*>( getPipe( ));
    const FrameData& frameData = pipe->getFrameData();

    vmml::Matrix4f view( frameData.data.rotation );

    if( _model )
    {
        vmml::Matrix4f scale( 
            _model->volScales.W, 0, 0, 0, 
            0, _model->volScales.H, 0, 0,
            0, 0, _model->volScales.D, 0,
            0, 0,                   0, 1 );

        view = scale * view;
    }

    view.setTranslation( frameData.data.translation );

    const vmml::Frustumf&  frustum       = getFrustum();
    const vmml::Matrix4f&  headTransform = getHeadTransform();
    const vmml::Matrix4f   modelView     = headTransform * view;

#ifdef DYNAMIC_NEAR_FAR
    vmml::Matrix4f modelInv;
    headTransform.getInverse( modelInv );

    const vmml::Vector3f zero  = modelInv * vmml::Vector3f( 0.0f, 0.0f,  0.0f );
    vmml::Vector3f       front = modelInv * vmml::Vector3f( 0.0f, 0.0f, -1.0f );
    front -= zero;
    front.normalise();
    EQINFO << getName()  << " front " << front << endl;
    front.scale( M_SQRT3_2 ); // bounding sphere size of unit-sized cube

    const vmml::Vector3f center( frameData.data.translation );
    const vmml::Vector3f near  = headTransform * ( center - front );
    const vmml::Vector3f far   = headTransform * ( center + front );
    const float          zNear = MAX( 0.0001f, -near.z );
    const float          zFar  = MAX( 0.0002f, -far.z );

    EQINFO << getName() << " center:    " << headTransform * center << endl;
    EQINFO << getName() << " near, far: " << near  << " " << far << endl;
    EQINFO << getName() << " near, far: " << zNear << " " << zFar << endl;
    setNearFar( zNear, zFar );
#endif

    vmml::Matrix4f projection;
    if( _perspective )
    {
        projection         = frustum.computeMatrix();
        _curFrData.frustum = frustum;

    }else
    {
        vmml::Frustumf  f = frustum;
        double zc = ( 1.0 + f.farPlane/f.nearPlane ) / 3.0;
        f.left   *= zc;
        f.right  *= zc;
        f.top    *= zc;
        f.bottom *= zc;

        projection         = f.computeOrthoMatrix();
        _curFrData.frustum = f;
    }

    _curFrData.modelviewM.set( modelView.ml );

    //calculate inverse transposed matrix
    _curFrData.modelviewM.getInverse( _curFrData.modelviewIM );
    _curFrData.modelviewITM = _curFrData.modelviewIM.getTransposed();

    culler.setup( projection * modelView );
}

}
