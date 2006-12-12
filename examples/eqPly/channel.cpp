
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

//#define DYNAMIC_NEAR 
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
    Frustumf frustum;
    _initFrustum( frustum );

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

    Node*            node  = (Node*)getNode();
    const Model*     model = node->getModel();
    const eq::Range& range = getRange();

    if( !range.isFull( )) // Color DB-patches
    {
        stde::hash<const char*> hasher;
        unsigned  seed  = (unsigned)(long long)this + hasher(getName().c_str());
        const int color = rand_r( &seed );
    
        glColor3f( (color&0xff) / 255., ((color>>8) & 0xff) / 255.,
                   ((color>>16) & 0xff) / 255. );
    }

    if( model )
    {
        vector<const Model::BBox*> candidates;
        candidates.push_back( model->getBBox( ) );

        while( !candidates.empty( ) )
        {
            const Model::BBox *bbox = candidates.back();
            candidates.pop_back();

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
                        model->traverseBBox( bbox, 0, _drawBBoxCB, 0, this );
                        break;   
                    }
                    // partial range, fall through
                }
                case FRUSTUM_VISIBILITY_PARTIAL:
                    if( !bbox->children )
                    {
                        if( bbox->range[0] >= range.start )
                            model->traverseBBox( bbox, 0, _drawBBoxCB, 0, this);
                        // else drop, will be drawn by 'previous' channel
                    }
                    else
                        for( int i=0; i<8; i++ )
                            candidates.push_back( &bbox->children[i] );
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

void Channel::_drawBBoxCB( Model::BBox *bbox, void *userData )
{
    Channel* channel = static_cast<Channel*>( userData );
    channel->_drawBBox( bbox );
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

    const size_t     nFaces = bbox->nFaces;
    const eq::Range& range  = getRange();
    const bool       color  = range.isFull(); // Use color only if not DB

    glNewList( displayList, GL_COMPILE );
    glBegin( GL_TRIANGLES );

    if( color )
        for( size_t i=0; i<nFaces; ++i )
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
    else
        for( size_t i=0; i<nFaces; ++i )
        {
            const NormalFace<ColorVertex> &face = bbox->faces[i];
            
            glNormal3fv( face.normal );
            glVertex3fv( face.vertices[0].pos );
            
            glNormal3fv( face.normal ); 
            glVertex3fv( face.vertices[1].pos );
            
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
#ifdef DYNAMIC_NEAR
    vmml::Frustumf oldFrustum = getFrustum();

    //oldFrustum.adjustNear( 1.0f );
    const vmml::Matrix4f  oldProj = oldFrustum.computeMatrix();
    const vmml::Vector3f& center = 
        oldProj * getHeadTransform() * -_frameData->_data.translation;

    const float near = MAX( 0.001f,        center.z - M_SQRT3_2 );
    const float far  = MAX( near + 0.001f, center.z + M_SQRT3_2 );
    EQINFO << "Model Z position: " << center.z << " near, far: " << near << " " 
           << far << endl;
    setNearFar( near, far );
#endif

    const vmml::Frustumf& eqFrustum  = getFrustum();
    const vmml::Matrix4f  projection = eqFrustum.computeMatrix();

    // apply rot + trans + head transform
    vmml::Matrix4f view( _frameData->_data.rotation );
    view.setTranslation( _frameData->_data.translation );

    const vmml::Matrix4f& mvm = getHeadTransform() * view;


    const eq::PixelViewport& pvp = getPixelViewport();

    frustum.initView( projection.ml, mvm.ml, pvp);

}
