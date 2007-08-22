
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


#include <GLUT/glut.h> //Only for gluErrorString()


//#define DYNAMIC_NEAR_FAR 
#ifndef M_SQRT3
#  define M_SQRT3    1.7321f   /* sqrt(3) */
#endif
#ifndef M_SQRT3_2
#  define M_SQRT3_2  0.86603f  /* sqrt(3)/2 */
#endif

namespace eqVol
{

using namespace eqBase;
using namespace std;
using namespace hlpFuncs;

	
GLuint createGxGyGzATexture( const uint8_t *volume, uint32_t w, uint32_t h, uint32_t d, const eq::Range& range )
{
	EQINFO << "Precalculating gradients..." << endl;
	
    int wh = w*h;

	vector<uint8_t> GxGyGzA;
	GxGyGzA.resize( wh*d*4 );
	memset( &GxGyGzA[0], 0, wh*d*4 );
	
	uint32_t start = 1;//static_cast<uint32_t>( clip<int32_t>( d*range.start-2, 1, d-1 ) );
	uint32_t end   = d-1;//static_cast<uint32_t>( clip<int32_t>( d*range.end  +2, 1, d-1 ) );
	
	for( uint32_t z=start; z<end; z++ )
    {
        int zwh = z*wh;

        const uint8_t *curPz = volume + zwh;

		for( uint32_t y=1; y<h-1; y++ )
        {
            int zwh_y = zwh + y*w;
            const uint8_t * curPy = curPz + y*w ;
			for( uint32_t x=1; x<w-1; x++ )
            {
                const uint8_t * curP = curPy +  x;
                const uint8_t * prvP = curP  - wh;
                const uint8_t * nxtP = curP  + wh;
				int32_t gx = 
                      nxtP[  1+w ]+ 3*curP[  1+w ]+   prvP[  1+w ]+
                    3*nxtP[  1   ]+ 6*curP[  1   ]+ 3*prvP[  1   ]+
                      nxtP[  1-w ]+ 3*curP[  1-w ]+   prvP[  1-w ]-
				
                      nxtP[ -1+w ]- 3*curP[ -1+w ]-   prvP[ -1+w ]-
                    3*nxtP[ -1   ]- 6*curP[ -1   ]- 3*prvP[ -1   ]-
                      nxtP[ -1-w ]- 3*curP[ -1-w ]-   prvP[ -1-w ];
				
				int32_t gy = 
                      nxtP[  1+w ]+ 3*curP[  1+w ]+   prvP[  1+w ]+
					3*nxtP[    w ]+ 6*curP[    w ]+ 3*prvP[    w ]+
					  nxtP[ -1+w ]+ 3*curP[ -1+w ]+   prvP[ -1+w ]-
					
				 	  nxtP[  1-w ]- 3*curP[  1-w ]-   prvP[  1-w ]-
					3*nxtP[   -w ]- 6*curP[   -w ]- 3*prvP[   -w ]-
                      nxtP[ -1-w ]- 3*curP[ -1-w ]-   prvP[ -1-w ];
				
				int32_t gz = 
                      prvP[  1+w ]+ 3*prvP[  1   ]+   prvP[  1-w ]+
                    3*prvP[    w ]+ 6*prvP[  0   ]+ 3*prvP[   -w ]+
					  prvP[ -1+w ]+ 3*prvP[ -1   ]+   prvP[ -1-w ]-
					
                      nxtP[  1+w ]- 3*nxtP[  1   ]-   nxtP[  1-w ]-
					3*nxtP[   +w ]- 6*nxtP[  0   ]- 3*nxtP[   -w ]-
					  nxtP[ -1+w ]- 3*nxtP[ -1   ]-   nxtP[ -1-w ];
				
				
				int32_t length = sqrt( (gx*gx+gy*gy+gz*gz) )+1;
				
				gx = ( gx*255/length + 255 )/2; 
				gy = ( gy*255/length + 255 )/2;
				gz = ( gz*255/length + 255 )/2;
				
				GxGyGzA[(zwh_y + x)*4   ] = (uint8_t)(gx);
				GxGyGzA[(zwh_y + x)*4 +1] = (uint8_t)(gy);
				GxGyGzA[(zwh_y + x)*4 +2] = (uint8_t)(gz);	
				GxGyGzA[(zwh_y + x)*4 +3] = curP[0];
			}
		}
	}
	
	EQINFO << "Done precalculating gradients." << endl;
	
	GLuint tex3D;
    glGenTextures( 1, &tex3D ); 	
	glBindTexture(GL_TEXTURE_3D, tex3D);
    
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R    , GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR        );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR        );

    glTexImage3D( GL_TEXTURE_3D, 0, GL_RGBA, w, h, d, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)(&GxGyGzA[0]) );

	return tex3D;
}

void checkError( std::string msg ) 
{
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
		EQERROR << msg << " GL Error: " << gluErrorString(error) << endl;
}

//#define COMPOSE_MODE_NEW
void createPreintegrationTable( const uint8_t *Table, GLuint &preintName )
{
	EQINFO << "Calculating preintegration table..." << endl;
			
	double rInt[256]; rInt[0] = 0.;
	double gInt[256]; gInt[0] = 0.;
	double bInt[256]; bInt[0] = 0.;
	double aInt[256]; aInt[0] = 0.;
	
	for( int i=1; i<256; i++ )
	{
		double tauc = ( Table[(i-1)*4+3] + Table[i*4+3] ) / 2.;
		
		rInt[i] = rInt[i-1] + ( Table[(i-1)*4+0] + Table[i*4+0] )/2.*tauc/255.;
		gInt[i] = gInt[i-1] + ( Table[(i-1)*4+1] + Table[i*4+1] )/2.*tauc/255.;
		bInt[i] = bInt[i-1] + ( Table[(i-1)*4+2] + Table[i*4+2] )/2.*tauc/255.;
		aInt[i] = aInt[i-1] + tauc;
	}

	vector<GLubyte> lookupI;
	lookupI.resize(256*256*4);	
	GLubyte* lookupImg = &lookupI[0];	// Preint Texture	
	int lookupindex=0;

	for( int sb=0; sb<256; sb++ )
	{
		for( int sf=0; sf<256; sf++ )
		{
			int smin, smax;
			if( sb<sf ) { smin=sb; smax=sf; }
			else        { smin=sf; smax=sb; }
			
			int rcol, gcol, bcol, acol;
			if( smax != smin )
			{
				double factor = 1. / (double)(smax-smin);
				rcol = (rInt[smax]-rInt[smin])*factor;
				gcol = (gInt[smax]-gInt[smin])*factor;
				bcol = (bInt[smax]-bInt[smin])*factor;
#ifdef COMPOSE_MODE_NEW
				acol = 256.*(exp(-(aInt[smax]-aInt[smin])*factor/255.));
#else
				acol = 256.*(1.0-exp(-(aInt[smax]-aInt[smin])*factor/255.));
#endif
			} else
			{
				double factor = 1./255.;
				rcol = Table[smin*4+0]*Table[smin*4+3]*factor;
				gcol = Table[smin*4+1]*Table[smin*4+3]*factor;
				bcol = Table[smin*4+2]*Table[smin*4+3]*factor;
#ifdef COMPOSE_MODE_NEW
				acol = 256.*(exp(-Table[smin*4+3]*1./255.));
#else
				acol = 256.*(1.0-exp(-Table[smin*4+3]*1./255.));
#endif
			}
			
			lookupImg[lookupindex++] = clip( rcol, 0, 255 );//min( rcol, 255 );	
			lookupImg[lookupindex++] = clip( gcol, 0, 255 );//min( gcol, 255 );
			lookupImg[lookupindex++] = clip( bcol, 0, 255 );//min( bcol, 255 );
			lookupImg[lookupindex++] = clip( acol, 0, 255 );//min( acol, 255 );
		}
	}
	
	
	glGenTextures( 1, &preintName );
	glBindTexture( GL_TEXTURE_2D, preintName );
	glTexImage2D(  GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, lookupImg );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR        );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR        );
}

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
	
bool Channel::configInit( const uint32_t initID )
{
    EQINFO << "Init channel initID " << initID << " ptr " << this << endl;

//chose projection type
	_perspective = true;
	
// Init model-dependent stuff
    Node*            node  = (Node*)getNode();
    const Model*     model = node->getModel();
	
	EQASSERT( model );
	
	//setup the 3D textures
	glGenTextures( 1, &_tex3D );
	glGenTextures( 1, &_tex1D );

	_tex3D = 0;
  
	_slices = model->GetWidth() *2;
	createVertexArray( _slices, _vertexID );

	createPreintegrationTable( model->GetTransferFunc(), _preintName );
    
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

//Init shader
//	if( !eqShader::loadShaders("./examples/eqVol/vshader.oglsl", "./examples/eqVol/fshader.oglsl", _shader) )
//		return false;
		
//	glUseProgramObjectARB( NULL );
	
	
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
//	vmml::Matrix4f rotation( 
//		-0.9687  , -0.072722, 0.23743 , 0,
//	 	 0.23553 ,  0.33545 , 0.97129 , 0,
//		-0.078602,  0.9968  ,-0.015379, 0,
//		 0.0     ,  0.0     , 0.0     , 1 );
		
    vmml::FrustumCullerf culler;
    _initFrustum( culler ); 

    applyBuffer();
    applyViewport();
            
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    applyFrustum();

    const Pipe*      pipe      = static_cast<Pipe*>( getPipe( ));
    const FrameData& frameData = pipe->getFrameData();

	_curFrData.needInverse = frameData.data.rotation.ml[10] < 0; //check if z coordinate of vector (0,0,1) after rotation is < 0
	_curFrData.lastRange   = getRange();						 //save range for compositing

    Node*            node  = (Node*)getNode();
    const Model*     model = node->getModel();

	if( _tex3D==0 )
		_tex3D = createGxGyGzATexture( model->GetData(), model->GetWidth(), model->GetHeight(), model->GetDepth(), _curFrData.lastRange );

   	if( model )
    {
		int numberOfSlices = _slices;

		GLfloat lightAmbient[]  = {0.2f, 0.2f, 0.2f, 1.0f};
		GLfloat lightDiffuse[]  = {0.8f, 0.8f, 0.8f, 1.0f};
		GLfloat lightSpecular[] = {0.8f, 0.8f, 0.8f, 1.0f};
		GLfloat lightPosition[] = {0.0f, 0.0f, 1.0f, 0.0f}; 

		
//		glClearColor( 1.0f, 1.0f, 1.0f, 1.0f ); 

//		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		//set light parameters
		glLightfv( GL_LIGHT0, GL_AMBIENT,  lightAmbient  );
		glLightfv( GL_LIGHT0, GL_DIFFUSE,  lightDiffuse  );
//		glLightfv( GL_LIGHT0, GL_SPECULAR, lightSpecular );
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

/*		glPushMatrix(); //Rotate lights
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

//		double dXdist = 2.0*m_dScaleFactorX;
//		double dYdist = 2.0*m_dScaleFactorY;
//		double dZdist = 2.0*m_dScaleFactorZ;
//		int nMaxNumSlices = int( sqrt( dXdist*dXdist + dYdist*dYdist + dZdist*dZdist ) / m_dSliceDist ) + 1;
		

		//rendering

		// Enable shader
//		glUseProgramObjectARB( _shader );
		eqCgShaders shaders = pipe->getShaders();
		shaders.cgVertex->use();
		shaders.cgFragment->use();

		CGprogram m_vProg = shaders.cgVertex->get_program();
		CGprogram m_fProg = shaders.cgFragment->get_program();
		{
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
			for(int i = 0; i < 8; ++i) {
				vertices[3*i]   = double(m_dScaleFactorX*m_pVertices[i][0]);
				vertices[3*i+1] = double(m_dScaleFactorY*m_pVertices[i][1]);
				vertices[3*i+2] = double(m_dScaleFactorZ*m_pVertices[i][2]);
			}
			
#ifndef TEXTURE_ROTATION 
			cgGLSetParameterArray3f( cgGetNamedParameter( m_vProg, "vecVertices" ), 0,  8, vertices );
			cgGLSetParameterArray1f( cgGetNamedParameter( m_vProg, "sequence"    ), 0, 64, sequence );
			cgGLSetParameterArray1f( cgGetNamedParameter( m_vProg, "v1"          ), 0, 24, e1       );
			cgGLSetParameterArray1f( cgGetNamedParameter( m_vProg, "v2"          ), 0, 24, e2       );
#endif			
//			glUniform3fvARB( glGetUniformLocationARB( _shader, "vecVertices"),  8, vertices );
//			glUniform1ivARB( glGetUniformLocationARB( _shader, "sequence"   ), 64, sequence );
//			glUniform1ivARB( glGetUniformLocationARB( _shader, "v1"         ), 24, e1       );
//			glUniform1ivARB( glGetUniformLocationARB( _shader, "v2"         ), 24, e2       );
		}
		

#ifndef TEXTURE_ROTATION 
		cgGLSetParameter3dv( cgGetNamedParameter( m_vProg, "vecView"    ), viewVec.xyzw   );
		cgGLSetParameter1f(  cgGetNamedParameter( m_vProg, "frontIndex" ), float(nMaxIdx) );


		cgGLSetParameter1d(  cgGetNamedParameter( m_vProg, "dPlaneIncr" ), m_dSliceDist   );  
#endif
//		glUniform3fARB( glGetUniformLocationARB( _shader, "vecView"    ), viewVec.x, viewVec.y, viewVec.z );
//		const GLint frontInd = static_cast<GLint>(nMaxIdx);
//		glUniform1iARB( glGetUniformLocationARB( _shader, "frontIndex" ), frontInd );
//		glUniform1fARB( glGetUniformLocationARB( _shader, "dPlaneIncr" ), m_dSliceDist   );
		
		{
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
			for(int v = 1; v < 8; v++) {
				dDist = (camPosition - m_pVertices[v]).length();
				if (dDist < dMinDist) {
					dMinDist = dDist;
					nMinIdx = v;
				}
			}

			double dStartDist   = viewVec.dot( m_pVertices[nSequence[nMaxIdx][0]] );
			double dEndDist     = viewVec.dot( m_pVertices[nMaxIdx]               );
			double dS           = ceil( dStartDist/m_dSliceDist);
			dStartDist          = dS * m_dSliceDist;

			int nNumSlices = int((dEndDist-dStartDist)/m_dSliceDist)+1; 

#ifndef TEXTURE_ROTATION 
			cgGLSetParameter1d(cgGetNamedParameter( m_vProg,"dPlaneStart"),dStartDist); 
#endif
//			glUniform1fARB( glGetUniformLocationARB( _shader, "dPlaneStart" ), dStartDist   );


			// Fill volume data
			{
				glActiveTexture( GL_TEXTURE1 );
				glBindTexture( GL_TEXTURE_2D, _preintName ); //preintegrated values
//				glUniform1iARB( glGetUniformLocationARB( _shader, "preInt"        ), 1                  ); //f-shader
				cgGLSetTextureParameter(    cgGetNamedParameter( m_fProg,"preInt" ), _preintName );
				cgGLEnableTextureParameter( cgGetNamedParameter( m_fProg,"preInt" )              );


				glActiveTexture( GL_TEXTURE0 ); // Activate last because it has to be the active texture
				glBindTexture( GL_TEXTURE_3D, _tex3D ); //gx, gy, gz, val
				glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
//				glUniform1iARB( glGetUniformLocationARB( _shader, "volume"        ), 0                  ); //f-shader
				cgGLSetTextureParameter(    cgGetNamedParameter( m_fProg,"volume" ), _tex3D );
				cgGLEnableTextureParameter( cgGetNamedParameter( m_fProg,"volume" )         );


				cgGLSetParameter1f(  cgGetNamedParameter( m_vProg, "sliceDistance" ), m_dSliceDist );
				cgGLSetParameter1f(  cgGetNamedParameter( m_fProg, "shininess" ), 20.0f );
				
//				glUniform1fARB( glGetUniformLocationARB( _shader, "sliceDistance" ), 3.6/numberOfSlices ); //v-shader
//				glUniform1fARB( glGetUniformLocationARB( _shader, "shininess"     ), 20.0f              ); //f-shader
			}

			//Render slices
			glEnable(GL_BLEND);
#ifdef COMPOSE_MODE_NEW
//			glBlendFuncSeparateEXT( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA );
			glBlendFuncSeparateEXT( GL_ONE, GL_SRC_ALPHA, GL_ZERO, GL_SRC_ALPHA );
//			glBlendFuncSeparateEXT( GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA );
#else
			glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
#endif

#ifndef TEXTURE_ROTATION 
			cgGLSetParameter1f(  cgGetNamedParameter( m_fProg, "lines" ), 0.0f );
			for( int s = 0; s < nNumSlices; ++s )
			{

				glBegin( GL_POLYGON );
				for( int i = 0; i < 6; ++i )
					glVertex2i( i, nNumSlices-1-s );
				glEnd();

/*				cgGLSetParameter1f(  cgGetNamedParameter( m_fProg, "lines" ), 1.0f );

				glBegin( GL_LINE_LOOP );
				for( int i = 0; i < 6; ++i )
					glVertex2i( i, nNumSlices-1-s );
			
				glEnd();
*/				
			}
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

			//disable shader
//			glUseProgramObjectARB( NULL );

			checkError( "error during rendering " );			
		}

	//Disable shader
		shaders.cgVertex->turnOff();
		shaders.cgFragment->turnOff();
		
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

void Channel::arrangeFrames( vector<Range>& ranges )
{
	ranges.push_back( _curFrData.lastRange );

	if( _perspective )
	{//perspective projection
		vmml::Vector3d norm = _curFrData.modelviewITM * vmml::Vector3d( 0.0, 0.0, 1.0 );
		norm.normalize();
		
		sort( ranges.begin(), ranges.end(), cmpRangesDec );

		vector<double> dotVals;  	        // cos of angle between normal and vectors from center 
		dotVals.reserve( ranges.size()+1 ); // of projection to the middle of slices' boundaries
	
		for( uint i=0; i<ranges.size(); i++ )
		{
			double px = -1.0 + ranges[i].start*2.0;

			vmml::Vector4d pS = _curFrData.modelviewM * vmml::Vector4d( 0.0, 0.0, px , 1.0 );
			dotVals.push_back( norm.dot( pS.getNormalizedVector3() ) );
		}

		vmml::Vector4d pS = _curFrData.modelviewM * vmml::Vector4d( 0.0, 0.0, 1.0, 1.0 );
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
		sort( ranges.begin(), ranges.end(), _curFrData.needInverse ? cmpRangesDec : cmpRangesInc );
	}
}


void IntersectViewports( PixelViewport &pvp, const vector<PixelViewport> &pvpVec )
{
	if( pvpVec.size() < 1 )
	{
		pvp.invalidate();
		return;
	}
	
	PixelViewport overalPVP = pvpVec[0];

	for( uint i=1; i<pvpVec.size(); i++ )
		overalPVP += pvpVec[i];

	pvp ^= overalPVP;	
}

void Channel::_clearPixelViewPorts( const vector<eq::PixelViewport> &pvpVec )
{
	for( uint i=0; i<pvpVec.size(); i++ )
		clearViewport( pvpVec[i] );
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
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
#endif //SOLID_BG
#endif //NDEBUG

    glClear( GL_COLOR_BUFFER_BIT );

	applyViewport();
}
	

void Channel::frameAssemble( const uint32_t frameID )
{
    applyBuffer();
    applyViewport();
    setupAssemblyState();

    Pipe*                 pipe    = static_cast<eqVol::Pipe*>( getPipe() );
    const vector<Frame*>& frames  = getInputFrames();
    Monitor<uint32_t>     monitor;

    for( vector<Frame*>::const_iterator i = frames.begin();
         i != frames.end(); ++i )
    {
        Frame* frame = *i;
        frame->addListener( monitor );
    }

    uint32_t       nUsedFrames  = 0;
    vector<Frame*> unusedFrames = frames;

    while( !unusedFrames.empty() )
    {
        StatEvent event( StatEvent::CHANNEL_WAIT_FRAME, this, 
                         pipe->getFrameTime( ));

        monitor.waitGE( ++nUsedFrames );
        if( getIAttribute( IATTR_HINT_STATISTICS ))
            pipe->addStatEvent( event );

		uint32_t nFr = frames.size();
        for( vector<Frame*>::iterator i = unusedFrames.begin();
             i != unusedFrames.end(); ++i )
        {
            Frame* frame = *i;
            if( frame->isReady( ))
				nFr--;
        }

		if( nFr == 0 ) //All frames is ready
		{
			//fill ranges - it should be supplied by server actualy 
			eq::PixelViewport curPVP = getPixelViewport();
			
			vector<Range> ranges;
			for( uint k=0; k<unusedFrames.size(); k++ )
			{
				Range curRange = unusedFrames[k]->getRange();
				
				if( !curRange.isFull() ) // Add only DB related slices, screen decomposition should be composed as is
				{
					ranges.push_back( curRange );
					IntersectViewports( curPVP, unusedFrames[k]->getPixelViewports() ); 
				}
			}
			
			//calculate correct frames sequence
			arrangeFrames( ranges );

			//check if current frame in proper position, redback if not
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
					clearViewport( curPVP );
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
						_clearPixelViewPorts( frame->getPixelViewports() );
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

					if( ranges.back() == _curFrData.lastRange ) // current range equals to range of original frame
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
		}
    }

    for( vector<Frame*>::const_iterator i = frames.begin(); i != frames.end();
         ++i )
    {
        Frame* frame = *i;
        frame->syncAssemble();
        frame->removeListener( monitor );
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

//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
		(*iter)->setDataBuffers( ( (*iter)->getDataBuffers() )&( ~Frame::BUFFER_DEPTH ) );
		
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
    glBegin( GL_TRIANGLE_STRIP ); {
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
//	_curFrData.projectionM = projection;

	//calculate inverse transposed matrix
	_curFrData.modelviewM.getInverse( _curFrData.modelviewIM );
	_curFrData.modelviewITM = (_curFrData.modelviewIM.getTransposed()).getMainSubmatrix();
	

    culler.setup( projection * modelView );
}
}
