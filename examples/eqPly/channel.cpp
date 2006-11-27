
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channel.h"

#include "frameData.h"
#include "initData.h"
#include "node.h"
#include "pipe.h"

using namespace std;
using namespace eqBase;

static float lightpos[] = { 0., 0., 1., 0. };

// #define DYNAMIC_NEAR 
#ifndef M_SQRT3
#  define M_SQRT3    1.7321f   /* sqrt(3) */
#endif
#ifndef M_SQRT3_2
#  define M_SQRT3_2  0.86603f  /* sqrt(3)/2 */
#endif

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

#ifdef DYNAMIC_NEAR // Code assumes the view is in the x-y plane
    
    //vmml::Vector3f viewTrans = _frameData->_data.translation * 
    const float near = MAX( 0.001f, 
                            -_frameData->_data.translation.z - M_SQRT3_2 );
    const float far  = near + M_SQRT3;
    setNearFar( near, far );
#endif

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

    const eq::Range& range = getRange();

    if( model )
    {
        vector<const Model::BBox*> bBoxVector;
        bBoxVector.push_back( model->getBBox( ) );
        while( !bBoxVector.empty( ) )
        {
            const Model::BBox *bbox = bBoxVector.back();
            bBoxVector.pop_back();

            // cull against 'completely out of range'
            if( bbox->range[0] >= range.end || bbox->range[1] < range.start )
                continue;

            const FrustumVisibility visibility = frustum.sphereVisibility(
                bbox->cullSphere.center.pos, bbox->cullSphere.radius );
            switch( visibility )
            {
                case FRUSTUM_VISIBILITY_FULL:
                {
                    const bool fullyInRange = (bbox->range[0] >= range.start && 
                                               bbox->range[1] <  range.end );
                    if( fullyInRange )
                    {
                        _drawBBox( bbox );
                        break;   
                    }
                    // partial range, fall through
                }
                case FRUSTUM_VISIBILITY_PARTIAL:
                    if( !bbox->children )
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
        glColor3f( 1.f, 1.f, 0.f );
        glNormal3f( 0.f, -1.f, 0.f );
        glBegin( GL_TRIANGLE_STRIP );
        glVertex3f(  .25f, 0.f,  .25f );
        glVertex3f(  .25f, 0.f, -.25f );
        glVertex3f( -.25f, 0.f,  .25f );
        glVertex3f( -.25f, 0.f, -.25f );
        glEnd();
        glFinish();
    }
}

void Channel::_drawBBox( const Model::BBox *bbox )
{
    Pipe*  pipe        = static_cast<Pipe*>( getPipe( ));
    GLuint displayList = pipe->getDisplayList( bbox );

    if( displayList != 0 )
    {
        glCallList( displayList );
        return;
    }

    displayList = pipe->newDisplayList( bbox );
    EQASSERT( displayList );

    glNewList( displayList, GL_COMPILE );
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

    glEndList();
    glCallList( displayList );
}

void Channel::_initFrustum( Frustumf& frustum )
{
    // apply frustum
    const eq::Frustum&   eqFrustum  = getFrustum();
    const vmml::Matrix4f projection = eqFrustum.computeMatrix();

    // apply rot + trans + head transform
    vmml::Matrix4f view( _frameData->_data.rotation );
    view.setTranslation( _frameData->_data.translation );

    const vmml::Matrix4f& mvm = getHeadTransform() * view;

    const eq::PixelViewport& pvp = getPixelViewport();

    frustum.initView( projection.ml, mvm.ml, pvp);
}
