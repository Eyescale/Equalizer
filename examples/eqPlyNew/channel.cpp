
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "channel.h"

#include "frameData.h"
#include "initData.h"
#include "node.h"
#include "pipe.h"
#include "window.h"
#include "vertexBufferState.h"

using namespace eqBase;
using namespace std;
using namespace mesh;

static float lightpos[] = { 0.0f, 0.0f, 1.0f, 0.0f };

#ifndef M_SQRT3_2
#  define M_SQRT3_2  0.86603f  /* sqrt(3)/2 */
#endif

namespace eqPly
{
bool Channel::configInit( const uint32_t initID )
{
    EQINFO << "Init channel initID " << initID << " ptr " << this << endl;

    setNearFar( 0.1f, 10.0f );
    return true;
}

void Channel::frameDraw( const uint32_t frameID )
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

    const Pipe*      pipe      = static_cast<Pipe*>( getPipe( ));
    const FrameData& frameData = pipe->getFrameData();

    glTranslatef( frameData.data.translation.x,
                  frameData.data.translation.y,
                  frameData.data.translation.z );
    glMultMatrixf( frameData.data.rotation.ml );

    Node*            node  = (Node*)getNode();
    const Model*     model = node->getModel();
    const eq::Range& range = getRange();

    if( !range.isFull( )) // Color DB-patches
    {
        const vmml::Vector3ub color = getUniqueColor();
        glColor3ub( color.r, color.g, color.b );
    }
    else if( !frameData.data.color || !model->hasColors() )
    {
        glColor3f( 1.0f, 1.0f, 1.0f );
    }
    
    if( model )
    {
        _drawModel( model );
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

    const eq::Viewport& vp = getViewport();
    if( range.isFull() && vp.isFullScreen( ))
        _drawLogo();
}

void Channel::frameAssemble( const uint32_t frameID )
{
    eq::Channel::frameAssemble( frameID );
    _drawLogo();
}

void Channel::_drawModel( const Model* model )
{
    Window*                  window    = static_cast<Window*>( getWindow() );
    mesh::VertexBufferState& state     = window->getState();

    const Pipe*              pipe      = static_cast<Pipe*>( getPipe( ));
    const FrameData&         frameData = pipe->getFrameData();

    const eq::Range&         range     = getRange();
    vmml::FrustumCullerf     culler;

    state.setColors( frameData.data.color && 
                     range.isFull() && 
                     model->hasColors() );
    _initFrustum( culler, model->getBoundingSphere( ));

    model->beginRendering( state );
        
    // start with root node
    vector< const VertexBufferBase* > candidates;
    candidates.push_back( model );
        
    while( !candidates.empty() )
    {
        const VertexBufferBase* treeNode = candidates.back();
        candidates.pop_back();
            
        // completely out of range check
        if( treeNode->getRange()[0] >= range.end || 
            treeNode->getRange()[1] < range.start )
            continue;
            
        // bounding sphere view frustum culling
        switch( culler.testSphere( treeNode->getBoundingSphere() ) )
        {
            case vmml::VISIBILITY_FULL:
                // if fully visible and fully in range, render it
                if( treeNode->getRange()[0] >= range.start && 
                    treeNode->getRange()[1] < range.end )
                {
                    treeNode->render( state );
                    break;
                }
                // partial range, fall through to partial visibility
            case vmml::VISIBILITY_PARTIAL:
            {
                const VertexBufferBase* left  = treeNode->getLeft();
                const VertexBufferBase* right = treeNode->getRight();
            
                if( !left && !right )
                {
                    if( treeNode->getRange()[0] >= range.start )
                        treeNode->render( state );
                    // else drop, to be drawn by 'previous' channel
                }
                else
                {
                    if( left )
                        candidates.push_back( left );
                    if( right )
                        candidates.push_back( right );
                }
                break;
            }
            case vmml::VISIBILITY_NONE:
                // do nothing
                break;
        }
    }
        
    model->endRendering( state );
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

void Channel::_initFrustum( vmml::FrustumCullerf& culler,
                            const vmml::Vector4f& boundingSphere )
{
    // setup frustum cull helper
    const Pipe*      pipe      = static_cast<Pipe*>( getPipe( ));
    const FrameData& frameData = pipe->getFrameData();

    vmml::Matrix4f view( frameData.data.rotation );
    view.setTranslation( frameData.data.translation );

    const vmml::Frustumf&  frustum       = getFrustum();
    const vmml::Matrix4f&  headTransform = getHeadTransform();
    const vmml::Matrix4f   modelView     = headTransform * view;
    const vmml::Matrix4f   projection    = frustum.computeMatrix();

    culler.setup( projection * modelView );

    // compute dynamic near/far plane of whole model
    vmml::Matrix4f modelInv;
    headTransform.getInverse( modelInv );

    const vmml::Vector3f zero  = modelInv * vmml::Vector3f::ZERO;
    vmml::Vector3f       front = modelInv * vmml::Vector3f( 0.0f, 0.0f, -1.0f );

    front -= zero;
    front.normalize();
    front.scale( boundingSphere.radius );

    const vmml::Vector3f center = vmml::Vector3f( boundingSphere.xyzw ) + 
                                  vmml::Vector3f( frameData.data.translation );
    const vmml::Vector3f nearPoint  = headTransform * ( center - front );
    const vmml::Vector3f farPoint   = headTransform * ( center + front );
    const float          zNear = MAX( 0.0001f, -nearPoint.z );
    const float          zFar  = MAX( 0.0002f, -farPoint.z );

    setNearFar( zNear, zFar );
}
}
