
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
   All rights reserved. */

#include "channel.h"

#include "frameData.h"
#include "initData.h"
#include "config.h"
#include "pipe.h"
#include "view.h"
#include "window.h"
#include "vertexBufferState.h"

using namespace eq::base;
using namespace std;
using namespace mesh;

// light parameters
static GLfloat lightPosition[] = {0.0f, 0.0f, 1.0f, 0.0f};
static GLfloat lightAmbient[]  = {0.1f, 0.1f, 0.1f, 1.0f};
static GLfloat lightDiffuse[]  = {0.8f, 0.8f, 0.8f, 1.0f};
static GLfloat lightSpecular[] = {0.8f, 0.8f, 0.8f, 1.0f};

// material properties
static GLfloat materialAmbient[]  = {0.2f, 0.2f, 0.2f, 1.0f};
static GLfloat materialDiffuse[]  = {0.8f, 0.8f, 0.8f, 1.0f};
static GLfloat materialSpecular[] = {0.5f, 0.5f, 0.5f, 1.0f};
static GLint  materialShininess   = 64;

#ifndef M_SQRT3_2
#  define M_SQRT3_2  0.86603f  /* sqrt(3)/2 */
#endif

namespace eqPly
{
bool Channel::configInit( const uint32_t initID )
{
    if( !eq::Channel::configInit( initID ))
        return false;

    setNearFar( 0.1f, 10.0f );
    return true;
}

void Channel::frameClear( const uint32_t frameID )
{
    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));

    const eq::View*  view      = getView();
    const FrameData& frameData = _getFrameData();
    if( view && frameData.getCurrentViewID() == view->getID( ))
        glClearColor( .4f, .4f, .4f, 1.0f );
#ifndef NDEBUG
    else if( getenv( "EQ_TAINT_CHANNELS" ))
    {
        const vmml::Vector3ub color = getUniqueColor();
        glClearColor( color.r/255.0f, color.g/255.0f, color.b/255.0f, 1.0f );
    }
#endif // DEBUG
    else
        glClearColor( 0.f, 0.f, 0.f, 1.0f );

    EQ_GL_CALL( glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ));
}

void Channel::frameDraw( const uint32_t frameID )
{
    // Setup OpenGL state
    eq::Channel::frameDraw( frameID );

    glLightfv( GL_LIGHT0, GL_POSITION, lightPosition );
    glLightfv( GL_LIGHT0, GL_AMBIENT,  lightAmbient  );
    glLightfv( GL_LIGHT0, GL_DIFFUSE,  lightDiffuse  );
    glLightfv( GL_LIGHT0, GL_SPECULAR, lightSpecular );

    glMaterialfv( GL_FRONT, GL_AMBIENT,   materialAmbient );
    glMaterialfv( GL_FRONT, GL_DIFFUSE,   materialDiffuse );
    glMaterialfv( GL_FRONT, GL_SPECULAR,  materialSpecular );
    glMateriali(  GL_FRONT, GL_SHININESS, materialShininess );

    const FrameData& frameData = _getFrameData();
    glPolygonMode( GL_FRONT_AND_BACK, 
                   frameData.useWireframe() ? GL_LINE : GL_FILL );

    const vmml::Vector3f& translation = frameData.getCameraTranslation();
    glTranslatef( translation.x, translation.y, translation.z );
    glMultMatrixf( frameData.getCameraRotation().ml );

    const Model*     model  = _getModel();
    const eq::Range& range  = getRange();

    if( !frameData.useColor( ))
    {
        glColor3f( .75f, .75f, .75f );
    }
    else if( range != eq::Range::ALL ) // Color DB-patches
    {
        const vmml::Vector3ub color = getUniqueColor();
        glColor3ub( color.r, color.g, color.b );
    }
    else if( model && !model->hasColors( ))
    {
        glColor3f( .75f, .75f, .75f );
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
        glVertex3f( -.25f, 0.f,  .25f );
        glVertex3f(  .25f, 0.f, -.25f );
        glVertex3f( -.25f, 0.f, -.25f );
        glEnd();
    }

    if( range == eq::Range::ALL )
        _drawLogo();

#ifndef NDEBUG
    outlineViewport();
#endif
}

void Channel::frameAssemble( const uint32_t frameID )
{
    eq::Channel::frameAssemble( frameID );
    _drawLogo();
}

const FrameData& Channel::_getFrameData() const
{
    const Pipe* pipe = static_cast<const Pipe*>( getPipe( ));
    return pipe->getFrameData();
}

void Channel::applyFrustum() const
{
    const FrameData& frameData = _getFrameData();

    if( frameData.useOrtho( ))
        eq::Channel::applyOrtho();
    else
        eq::Channel::applyFrustum();
}

const Model* Channel::_getModel()
{
    Config*     config = static_cast< Config* >( getConfig( ));
    const View* view   = static_cast< const View* >( getView( ));
    EQASSERT( !view || dynamic_cast< const View* >( getView( )));

    if( view )
        return config->getModel( view->getModelID( ));

    const FrameData& frameData = _getFrameData();
    return config->getModel( frameData.getModelID( ));
}

void Channel::_drawModel( const Model* model )
{
    Window*              window    = static_cast<Window*>( getWindow() );
    VertexBufferState&   state     = window->getState();
    const FrameData&     frameData = _getFrameData();
    const eq::Range&     range     = getRange();
    vmml::FrustumCullerf culler;

    state.setColors( frameData.useColor() && range == eq::Range::ALL && 
                     model->hasColors() );
    _initFrustum( culler, model->getBoundingSphere( ));

    const eq::Pipe* pipe = getPipe();
    const GLuint program = state.getProgram( pipe );
    if( program != VertexBufferState::INVALID )
        glUseProgram( program );
    
    model->beginRendering( state );
    
    // start with root node
    vector< const VertexBufferBase* > candidates;
    candidates.push_back( model );

#ifndef NDEBUG
    size_t verticesRendered = 0;
    size_t verticesOverlap  = 0;
#endif

    while( !candidates.empty() )
    {
        const VertexBufferBase* treeNode = candidates.back();
        candidates.pop_back();
            
        // completely out of range check
        if( treeNode->getRange()[0] >= range.end || 
            treeNode->getRange()[1] < range.start )
            continue;
            
        // bounding sphere view frustum culling
        const vmml::Visibility visibility = 
            culler.testSphere( treeNode->getBoundingSphere( ));

        switch( visibility )
        {
            case vmml::VISIBILITY_FULL:
                // if fully visible and fully in range, render it
                if( range == eq::Range::ALL || 
                    ( treeNode->getRange()[0] >= range.start && 
                      treeNode->getRange()[1] < range.end ))
                {
                    treeNode->render( state );
                    //treeNode->renderBoundingSphere( state );
#ifndef NDEBUG
                    verticesRendered += treeNode->getNumberOfVertices();
#endif
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
                    {
                        treeNode->render( state );
                        //treeNode->renderBoundingSphere( state );
#ifndef NDEBUG
                        verticesRendered += treeNode->getNumberOfVertices();
                        if( visibility == vmml::VISIBILITY_PARTIAL )
                            verticesOverlap  += treeNode->getNumberOfVertices();
#endif
                    }
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
    
    if( program != VertexBufferState::INVALID )
        glUseProgram( 0 );

#ifndef NDEBUG
    const size_t verticesTotal = model->getNumberOfVertices();
    EQLOG( LOG_CULL ) 
        << getName() << " rendered " << verticesRendered * 100 / verticesTotal
        << "% of model, overlap <= " << verticesOverlap * 100 / verticesTotal
        << "%" << endl;
#endif    
}

void Channel::_drawLogo()
{
    const Window*  window      = static_cast<Window*>( getWindow( ));
    GLuint         texture;
    vmml::Vector2i size;
        
    window->getLogoTexture( texture, size );
    if( !texture )
        return;
        
    const eq::Zoom& zoom = getZoom();
        
    const float newX = size.x * zoom.x;
    const float newY = size.y * zoom.y;
        
    const float delta = 5.0f * zoom.x ;
        
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    applyScreenFrustum();
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
        glVertex3f( delta, delta, 0.0f );
        
        glTexCoord2f( size.x, 0.0f );
        glVertex3f( newX + delta, delta, 0.0f );
        
        glTexCoord2f( 0.0f, size.y );
        glVertex3f( delta, newY + delta, 0.0f );
        
        glTexCoord2f( size.x, size.y );
        glVertex3f( newX + delta, newY + delta, 0.0f );
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
    const FrameData& frameData = _getFrameData();

    vmml::Matrix4f view( frameData.getCameraRotation( ));
    view.setTranslation( frameData.getCameraTranslation( ));

    const vmml::Frustumf&  frustum       = getFrustum();
    const vmml::Matrix4f&  headTransform = getHeadTransform();
    const vmml::Matrix4f   modelView     = headTransform * view;
    const vmml::Matrix4f   projection    = frameData.useOrtho() ?
                                               frustum.computeOrthoMatrix() :
                                               frustum.computeMatrix();

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
                             vmml::Vector3f( frameData.getCameraTranslation( ));
    const vmml::Vector3f nearPoint  = headTransform * ( center - front );
    const vmml::Vector3f farPoint   = headTransform * ( center + front );

    if( frameData.useOrtho( ))
    {
        EQASSERT( fabs( farPoint.z - nearPoint.z ) > 
                  numeric_limits< float >::epsilon( ));
        setNearFar( -nearPoint.z, -farPoint.z );
    }
    else
    {
        const float zNear = EQ_MAX( 0.0001f, -nearPoint.z );
        const float zFar  = EQ_MAX( 0.0002f, -farPoint.z );

        setNearFar( zNear, zFar );
    }
}
}
