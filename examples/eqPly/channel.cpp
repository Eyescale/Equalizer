
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channel.h"

#include "frameData.h"
#include "initData.h"
#include "node.h"

using namespace std;
using namespace eqBase;

static float lightpos[] = { 0., 0., 1., 0. };

bool Channel::init( const uint32_t initID )
{
    EQINFO << "Init channel initID " << initID << " ptr " << this << endl;
    eq::Config* config = getConfig();

    _initData  = (InitData*)config->getObject( initID );
    _frameData = _initData->getFrameData();
    
    return true;
}

bool Channel::exit()
{
    _initData  = NULL;
    _frameData = NULL;
    EQINFO << "Exit " << this << endl;
    return eq::Channel::exit();
}

void Channel::draw( const uint32_t frameID )
{
    applyBuffer();
    applyViewport();
            
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    applyFrustum();
    
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    applyHeadTransform();

    glLightfv( GL_LIGHT0, GL_POSITION, lightpos );

    glTranslatef( _frameData->_data.translation.x,
                  _frameData->_data.translation.y,
                  _frameData->_data.translation.z );
    glMultMatrixf( _frameData->_data.rotation.ml );

    Node*        node  = (Node*)getNode();
    const Model* model = node->getModel();

    Frustumf frustum;
    _initFrustum( frustum );
    const size_t  bboxThreshold = 64;

    if( model )
    {
        vector<const Model::BBox*> bBoxVector;
        bBoxVector.push_back( model->getBBox( ) );
        while( !bBoxVector.empty( ) )
        {
            const Model::BBox *bbox = bBoxVector.back();
            bBoxVector.pop_back();

            const FrustumVisibility visibility = frustum.sphereVisibility(
                bbox->cullSphere.center.pos, bbox->cullSphere.radius );
            switch( visibility )
            {
                case FRUSTUM_VISIBILITY_FULL:
                    _drawBBox( bbox );
                    break;   
                case FRUSTUM_VISIBILITY_PARTIAL:
                    if( !bbox->children || bbox->nFaces < bboxThreshold )
                        _drawBBox( bbox );
                    else
                        for( int i=0; i<8; i++ )
                            bBoxVector.push_back( &bbox->children[i] );
                    break;
                case FRUSTUM_VISIBILITY_NULL:
                    break;
            }
        }
    }
    else
    {
        glColor3f( 1, 1, 0 );
        glBegin( GL_TRIANGLE_STRIP );
        glVertex3f( -.25, -.25, -.25 );
        glVertex3f( -.25,  .25, -.25 );
        glVertex3f(  .25, -.25, -.25 );
        glVertex3f(  .25,  .25, -.25 );
        glEnd();
        glFinish();
    }
}

void Channel::_drawBBox( const Model::BBox *bbox )
{
    const size_t nFaces = bbox->nFaces;
            
    glBegin( GL_TRIANGLES );    
    for( size_t i=0; i<nFaces; i++ )
    {
        const NormalFace<ColorVertex> &face = bbox->faces[i];
        
        glColor3fv(  face.vertices[0].color );
        glNormal3fv( face.normal );
        glVertex3fv( face.vertices[0].pos );
        
        glColor3fv(  face.vertices[1].color );
        glNormal3fv( face.normal ); 
        glVertex3fv( face.vertices[1].pos );
        
        glColor3fv(  face.vertices[2].color );
        glNormal3fv( face.normal ); 
        glVertex3fv( face.vertices[2].pos );
    }
    glEnd();
}

void Channel::_initFrustum( Frustumf& frustum )
{
    // apply frustum
    const eq::Frustum& channelFrustum = getFrustum();
    float proj[16];
    channelFrustum.computeMatrix( proj );

    // apply rot + trans + head transform
    vmml::Matrix4f modelview( _frameData->_data.rotation );

    modelview.setTranslation( _frameData->_data.translation );
    modelview *= getHeadTransform();

    const eq::PixelViewport& pvp = getPixelViewport();

    frustum.initView( proj, modelview.ml, pvp);
}
