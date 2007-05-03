
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channel.h"

#include "frameData.h"
#include "initData.h"
#include "node.h"
#include "pipe.h"
#include "window.h"

using namespace eqBase;
using namespace std;

static float lightpos[] = { 0.0f, 0.0f, 1.0f, 0.0f };

//#define DYNAMIC_NEAR_FAR 
#ifndef M_SQRT3
#  define M_SQRT3    1.7321f   /* sqrt(3) */
#endif
#ifndef M_SQRT3_2
#  define M_SQRT3_2  0.86603f  /* sqrt(3)/2 */
#endif

namespace eqPly
{
bool Channel::configInit( const uint32_t initID )
{
    EQINFO << "Init channel initID " << initID << " ptr " << this << endl;

#ifndef DYNAMIC_NEAR_FAR
    setNearFar( 0.0001f, 10.0f );
#endif
    return true;
}

void Channel::frameDraw( const uint32_t frameID )
{
    vmml::FrustumCullerf culler;
    _initFrustum( culler );

    applyBuffer();
    applyViewport();
            
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    applyFrustum();

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    applyHeadTransform();

    glLightfv( GL_LIGHT0, GL_POSITION, lightpos );

    const Pipe*      pipe      = static_cast<Pipe*>( getPipe( ));
    const FrameData& frameData = pipe->getFrameData();

    glTranslatef( frameData.data.translation.x,
                  frameData.data.translation.y,
                  frameData.data.translation.z );
    glMultMatrixf( frameData.data.rotation.ml );

    Node*            node  = (Node*)getNode();
    const Model*     model = node->getModel();
    const eq::Range& range = getRange();

    if( !frameData.data.color )
    {
        glColor3f( 1.0f, 1.0f, 1.0f );
    }
    else if( !range.isFull( )) // Color DB-patches
    {
        uint32_t value = reinterpret_cast< size_t >( this ) & 0xffffffffu;
        uint8_t  red   = 0;
        uint8_t  green = 0;
        uint8_t  blue  = 0;

        for( unsigned i=0; i<8; ++i )
        {
            red   |= ( value&1 << (7-i) ); value >>= 1;
            green |= ( value&1 << (7-i) ); value >>= 1;
            blue  |= ( value&1 << (7-i) ); value >>= 1;
        }

        glColor3ub( red, green, blue );
    }

    if( model )
    {
        vector<const Model::BBox*> candidates;
        candidates.push_back( model->getBBox( ));

        while( !candidates.empty( ) )
        {
            const Model::BBox *bbox = candidates.back();
            candidates.pop_back();

            // cull against 'completely out of range'
            if( bbox->range[0] >= range.end || bbox->range[1] < range.start )
                continue;

            const vmml::Visibility visibility = culler.testSphere( 
                bbox->cullSphere );

            switch( visibility )
            {
                case vmml::VISIBILITY_FULL:
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
                case vmml::VISIBILITY_PARTIAL:
                    if( !bbox->children )
                    {
                        if( bbox->range[0] >= range.start )
                            model->traverseBBox( bbox, 0, _drawBBoxCB, 0, this);
                        // else drop, to be drawn by 'previous' channel
                    }
                    else
                        for( int i=0; i<8; i++ )
                            candidates.push_back( &bbox->children[i] );
                    break;
                case vmml::VISIBILITY_NONE:
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
    Window*          window      = static_cast<Window*>( getWindow( ));
    GLuint           displayList = window->getDisplayList( bbox );

    if( displayList )
    {
        glCallList( displayList );
        return;
    }

    displayList = window->newDisplayList( bbox );
    EQASSERT( displayList );

    Pipe*            pipe      = static_cast<Pipe*>( getPipe( ));
    const FrameData& frameData = pipe->getFrameData();
    const size_t     nFaces    = bbox->nFaces;
    const eq::Range& range     = getRange();
    const bool       color     = frameData.data.color && range.isFull();

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
    EQINFO << getName() << " near, far: " << near << " " << far << endl;
    EQINFO << getName() << " near, far: " << zNear << " " << zFar << endl;
    setNearFar( zNear, zFar );
#endif

    const vmml::Matrix4f     projection = frustum.computeMatrix();
    //const eq::PixelViewport& pvp        = getPixelViewport();

    culler.setup( projection * modelView );
}
}
