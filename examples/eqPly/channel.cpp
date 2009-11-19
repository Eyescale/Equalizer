
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   Copyright (c) 2007, Tobias Wolf <twolf@access.unizh.ch>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "channel.h"

#include "initData.h"
#include "config.h"
#include "configEvent.h"
#include "pipe.h"
#include "view.h"
#include "window.h"
#include "vertexBufferState.h"

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

Channel::Channel( eq::Window* parent )
             : eq::Channel( parent )
             , _model(0)
             , _modelID( EQ_ID_INVALID )
             , _accum ( 0 )
             , _jitterStep( 0 )
             , _totalSteps( 0 )
             , _subpixelStep( 0 )
             , _needsTransfer( false )
{
}

bool Channel::configInit( const uint32_t initID )
{
    if( !eq::Channel::configInit( initID ))
        return false;

    setNearFar( 0.1f, 10.0f );
    _model = 0;
    _modelID = EQ_ID_INVALID;

    return _configInitAccumBuffer();
}

bool Channel::_configInitAccumBuffer()
{
    // TODO OPT: Only alloc accum buffer for dest channels.
    const eq::PixelViewport& pvp = getPixelViewport();
    _accum = new eq::Accum( glewGetContext( ));
    EQASSERT( pvp.isValid( ));

    if( !_accum->init( pvp, getWindow()->getColorFormat( )))
    {
        setErrorMessage( "Accumulation buffer initialization failed." );
        EQERROR << "Accumulation buffer initialization failed." << std::endl;
        delete _accum;
        _accum = 0;
        return false;
    }
    // else

    _totalSteps = _accum->getMaxSteps();
    _jitterStep = _totalSteps - 1;
    return true;
}

bool Channel::configExit()
{
    delete _accum;
    _accum = 0;

    return true;
}

void Channel::frameClear( const uint32_t frameID )
{
    if( _accum->isInvalidNumSteps( ))
        return;

    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));

    const eq::View*  view      = getView();
    const FrameData& frameData = _getFrameData();
    if( view && frameData.getCurrentViewID() == view->getID( ))
        glClearColor( .4f, .4f, .4f, 1.0f );
#ifndef NDEBUG
    else if( getenv( "EQ_TAINT_CHANNELS" ))
    {
        const eq::Vector3ub color = getUniqueColor();
        glClearColor( color.r()/255.0f,
                      color.g()/255.0f,
                      color.b()/255.0f, 1.0f );
    }
#endif // NDEBUG
    else
        glClearColor( 0.f, 0.f, 0.f, 1.0f );

    EQ_GL_CALL( glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ));
}

void Channel::frameDraw( const uint32_t frameID )
{
    if( _accum->isInvalidNumSteps( ))
        return;

    const Model* model = _getModel();
    if( model )
        _updateNearFar( model->getBoundingSphere( ));

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

    const eq::Vector3f& translation = frameData.getCameraTranslation();

    glMultMatrixf( frameData.getCameraRotation().array );
    glTranslatef( translation.x(), translation.y(), translation.z() );
    glMultMatrixf( frameData.getModelRotation().array );

    if( frameData.getColorMode() == COLOR_DEMO )
    {
        const eq::Vector3ub color = getUniqueColor();
        glColor3ub( color.r(), color.g(), color.b() );
    }
    else
        glColor3f( .75f, .75f, .75f );

    if( model )
    {
        _drawModel( model );
    }
    else
    {
        glNormal3f( 0.f, -1.f, 0.f );
        glBegin( GL_TRIANGLE_STRIP );
        glVertex3f(  .25f, 0.f,  .25f );
        glVertex3f( -.25f, 0.f,  .25f );
        glVertex3f(  .25f, 0.f, -.25f );
        glVertex3f( -.25f, 0.f, -.25f );
        glEnd();
    }

    _subpixelStep = EQ_MAX( _subpixelStep, getSubPixel().size );
    _needsTransfer = true;
}

void Channel::frameAssemble( const uint32_t frameID )
{
    if( _accum->isInvalidNumSteps( ))
        return;

    if( getPixelViewport() != _currentPVP )
    {
        _needsTransfer = true;

        if( !_accum->usesFBO( ))
        {
            EQWARN << "Current viewport different from view viewport, ";
            EQWARN << "idle anti-aliasing not implemented." << std::endl;
            _jitterStep = 0;
        }

        eq::Channel::frameAssemble( frameID );
        return;
    }
    // else
    
    bool subPixelALL = true;
    const eq::FrameVector& frames = getInputFrames();

    for( eq::FrameVector::const_iterator i = frames.begin();
         i != frames.end(); ++i )
    {
        eq::Frame* frame = *i;
        const eq::SubPixel& curSubPixel = frame->getSubPixel();

        if( curSubPixel != eq::SubPixel::ALL )
            subPixelALL = false;

        _subpixelStep = EQ_MAX( _subpixelStep, frame->getSubPixel().size );
    }

    if( subPixelALL )
        _needsTransfer = true;
    else
        _needsTransfer = false;

    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));
    EQ_GL_CALL( setupAssemblyState( ));

	eq::Compositor::assembleFrames( getInputFrames(), this, _accum );

    EQ_GL_CALL( resetAssemblyState( ));
}

void Channel::frameReadback( const uint32_t frameID )
{
    if( _accum->isInvalidNumSteps( ))
        return;

    // OPT: Drop alpha channel from all frames during network transport
    const eq::FrameVector& frames = getOutputFrames();
    for( eq::FrameVector::const_iterator i = frames.begin(); 
         i != frames.end(); ++i )
    {
        (*i)->setAlphaUsage( false );
    }

    eq::Channel::frameReadback( frameID );
}

void Channel::frameStart( const uint32_t frameID,
                          const uint32_t frameNumber )
{
    const FrameData& frameData = _getFrameData();

    // ready for the next FSAA
    if( !frameData.isIdle( ))
    {
        _accum->clear();
        _jitterStep = _totalSteps - 1;
        _subpixelStep = 0;
    }

    eq::Channel::frameStart( frameID, frameNumber );
}

void Channel::frameViewStart( const uint32_t frameID )
{
    _currentPVP = getPixelViewport();
    eq::Channel::frameViewStart( frameID );
}

void Channel::frameFinish( const uint32_t frameID,
                           const uint32_t frameNumber )
{
    const FrameData& frameData = _getFrameData();

    if( frameData.isIdle() && _jitterStep > 0 )
    {
        if( _subpixelStep > _jitterStep )
            _jitterStep = 0;
        else
            _jitterStep -= _subpixelStep;
    }

    ConfigEvent event;
    event.data.type = ConfigEvent::IDLE_AA;
    event.jitter = _jitterStep;

    /* if _jitterStep == 0 and no more frames are requested,
     * the app will exit FSAA idle mode.
     */
    eq::Config* config = const_cast< eq::Config* >( getConfig( ));
    config->sendEvent( event );

    eq::Channel::frameFinish( frameID, frameNumber );
}

void Channel::frameViewFinish( const uint32_t frameID )
{
    const FrameData& frameData = _getFrameData();

    const eq::PixelViewport& pvp = getPixelViewport();
    bool isResized = _accum->resize( pvp.w, pvp.h );

    if( isResized )
    {
        _accum->clear();
        _jitterStep = _totalSteps - 1;
        _subpixelStep = 0;
        return;
    }
    //else

    if( frameData.isIdle() && _needsTransfer )
    {
        setupAssemblyState();

        if( !_accum->isInvalidNumSteps( ))
            _accum->accum();
        _accum->display();

        resetAssemblyState();
    }

    _drawOverlay();
    _drawHelp();

    if( frameData.useStatistics())
        EQ_GL_CALL( drawStatistics( ));
}

void Channel::applyFrustum() const
{
    const FrameData& frameData = _getFrameData();

    if( frameData.isIdle() && _jitterStep > 0 )
    {
        eq::Vector2f jitter = _getJitter();

        if( frameData.useOrtho( ))
        {
            eq::Frustumf ortho = getOrtho();
            ortho.apply_jitter( jitter );

            glOrtho( ortho.left(), ortho.right(),
                     ortho.bottom(), ortho.top(),
                     ortho.near_plane(), ortho.far_plane( ));
        }
        else
        {
            eq::Frustumf frustum = getFrustum();
            frustum.apply_jitter( jitter );

            glFrustum( frustum.left(), frustum.right(),
                       frustum.bottom(), frustum.top(),
                       frustum.near_plane(), frustum.far_plane( ));
        }
        return;
    }
    //else
    
    eq::Channel::applyFrustum();
}

const FrameData& Channel::_getFrameData() const
{
    const Pipe* pipe = static_cast<const Pipe*>( getPipe( ));
    return pipe->getFrameData();
}

eq::Vector2f Channel::_getJitter() const
{
    const eq::PixelViewport& pvp = getPixelViewport();
    float pvp_w = static_cast<float>( pvp.w );
    float pvp_h = static_cast<float>( pvp.h );
    float frustum_w = static_cast<float>(( getFrustum().get_width( )));
    float frustum_h = static_cast<float>(( getFrustum().get_height( )));

    float pixel_w = frustum_w / pvp_w;
    float pixel_h = frustum_h / pvp_h;

    uint32_t sampleSize = _getSampleSize();
    float subpixel_w = pixel_w / static_cast<float>( sampleSize );
    float subpixel_h = pixel_h / static_cast<float>( sampleSize );

    eq::Vector2i jitterStep = _getJitterStep();

    // Sample value randomly computed within the subpixel
    eq::base::RNG rng;
    float value_i = rng.get< float >() * subpixel_w
                    + static_cast<float>( jitterStep.x( )) * subpixel_w;

    float value_j = rng.get< float >() * subpixel_h
                    + static_cast<float>( jitterStep.y( )) * subpixel_h;

    return eq::Vector2f( value_i, value_j );
}

eq::Vector2i Channel::_getJitterStep() const
{
    uint32_t channelID  = getSubPixel().index;
    uint32_t idx = ( channelID * _totalSteps );
    idx += ( _jitterStep * primeNumberTable[ channelID ] ) % _totalSteps;

    uint32_t sampleSize = _getSampleSize();
    const int dx = idx % sampleSize;
    const int dy = idx / sampleSize;

    return eq::Vector2i( dx, dy );
}

const Model* Channel::_getModel()
{
    Config*     config = static_cast< Config* >( getConfig( ));
    const View* view   = static_cast< const View* >( getView( ));
    const FrameData& frameData = _getFrameData();
    EQASSERT( !view || dynamic_cast< const View* >( getView( )));

    const uint32_t id = view ? view->getModelID() : frameData.getModelID();
    if( id != _modelID )
    {
        _model = config->getModel( id );
        _modelID = id;
    }

    return _model;
}

void Channel::_drawModel( const Model* model )
{
    Window*            window    = static_cast<Window*>( getWindow() );
    VertexBufferState& state     = window->getState();
    const FrameData&   frameData = _getFrameData();
    const eq::Range&   range     = getRange();
    eq::FrustumCullerf culler;

    if( frameData.getColorMode() == COLOR_MODEL && model->hasColors( ))
        state.setColors( true );
    else
        state.setColors( false );

    _initFrustum( culler, model->getBoundingSphere( ));

    const eq::Pipe* pipe = getPipe();
    const GLuint program = state.getProgram( pipe );
    if( program != VertexBufferState::INVALID )
        glUseProgram( program );
    
    model->beginRendering( state );
    
    // start with root node
    std::vector< const mesh::VertexBufferBase* > candidates;
    candidates.push_back( model );

#ifndef NDEBUG
    size_t verticesRendered = 0;
    size_t verticesOverlap  = 0;
#endif

    while( !candidates.empty() )
    {
        const mesh::VertexBufferBase* treeNode = candidates.back();
        candidates.pop_back();
            
        // completely out of range check
        if( treeNode->getRange()[0] >= range.end || 
            treeNode->getRange()[1] < range.start )
            continue;
            
        // bounding sphere view frustum culling
        const vmml::Visibility visibility =
            culler.test_sphere( treeNode->getBoundingSphere( ));

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
                const mesh::VertexBufferBase* left  = treeNode->getLeft();
                const mesh::VertexBufferBase* right = treeNode->getRight();
            
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
        << "%" << std::endl;
#endif    
}

void Channel::_drawOverlay()
{
    // Draw the overlay logo
    const Window* window      = static_cast<Window*>( getWindow( ));
    GLuint        texture;
    eq::Vector2i  size;
        
    window->getLogoTexture( texture, size );
        
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    applyScreenFrustum();
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_LIGHTING );
    glColor3f( 1.0f, 1.0f, 1.0f );

#if 1
    // border
    const eq::PixelViewport& pvp = getPixelViewport();
    const eq::Viewport& vp = getViewport();
    const float w = pvp.w / vp.w - .5f;
    const float h = pvp.h / vp.h - .5f;

    glBegin( GL_LINE_LOOP );
    {
        glVertex3f( .5f, .5f, 0.f );
        glVertex3f(   w, .5f, 0.f );
        glVertex3f(   w,   h, 0.f );
        glVertex3f( .5f,   h, 0.f );
    } 
    glEnd();
#endif

    // logo
    if( texture )
    {
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        
        glEnable( GL_TEXTURE_RECTANGLE_ARB );
        glBindTexture( GL_TEXTURE_RECTANGLE_ARB, texture );
        glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER,
                         GL_LINEAR );
        glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, 
                         GL_LINEAR );
    
        glBegin( GL_TRIANGLE_STRIP ); {
            glTexCoord2f( 0.0f, 0.0f );
            glVertex3f( 5.0f, 5.0f, 0.0f );
            
            glTexCoord2f( size.x(), 0.0f );
            glVertex3f( size.x() + 5.0f, 5.0f, 0.0f );
            
            glTexCoord2f( 0.0f, size.y() );
            glVertex3f( 5.0f, size.y() + 5.0f, 0.0f );
            
            glTexCoord2f( size.x(), size.y() );
            glVertex3f( size.x() + 5.0f, size.y() + 5.0f, 0.0f );
        } glEnd();
    
        glDisable( GL_TEXTURE_RECTANGLE_ARB );
        glDisable( GL_BLEND );
    }
    glEnable( GL_LIGHTING );
    glEnable( GL_DEPTH_TEST );
}

void Channel::_drawHelp()
{
    const FrameData& frameData = _getFrameData();
    std::string message = frameData.getMessage();

    if( !frameData.showHelp() && message.empty( ))
        return;

    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));
    EQ_GL_CALL( setupAssemblyState( ));

    glDisable( GL_LIGHTING );
    glDisable( GL_DEPTH_TEST );

    glColor3f( 1.f, 1.f, 1.f );

    if( frameData.showHelp( ))
    {
        const eq::Window::Font* font = getWindow()->getSmallFont();
        std::string help = EqPly::getHelp();
        float y = 340.f;

        for( size_t pos = help.find( '\n' ); pos != std::string::npos;
             pos = help.find( '\n' ))
        {
            glRasterPos3f( 10.f, y, 0.99f );
            
            font->draw( help.substr( 0, pos ));
            help = help.substr( pos + 1 );
            y -= 16.f;
        }
        // last line
        glRasterPos3f( 10.f, y, 0.99f );
        font->draw( help );
    }

    if( !message.empty( ))
    {
        const eq::Window::Font* font = getWindow()->getMediumFont();

        const eq::Viewport& vp = getViewport();
        const eq::PixelViewport& pvp = getPixelViewport();

        const float width = pvp.w / vp.w;
        const float xOffset = vp.x * width;

        const float height = pvp.h / vp.h;
        const float yOffset = vp.y * height;
        const float yMiddle = 0.5f * height;
        float y = yMiddle - yOffset;

        for( size_t pos = message.find( '\n' ); pos != std::string::npos;
             pos = message.find( '\n' ))
        {
            glRasterPos3f( 10.f - xOffset, y, 0.99f );
            
            font->draw( message.substr( 0, pos ));
            message = message.substr( pos + 1 );
            y -= 22.f;
        }
        // last line
        glRasterPos3f( 10.f - xOffset, y, 0.99f );
        font->draw( message );
    }

    EQ_GL_CALL( resetAssemblyState( ));
}

void Channel::_updateNearFar( const mesh::BoundingSphere& boundingSphere )
{
    // compute dynamic near/far plane of whole model
    const FrameData& frameData = _getFrameData();
    const bool ortho = frameData.useOrtho();

    const eq::Matrix4f& rotation     = frameData.getCameraRotation();
    const eq::Matrix4f headTransform = getHeadTransform() * rotation;

    eq::Matrix4f modelInv;
    compute_inverse( headTransform, modelInv );

    const eq::Vector3f zero  = modelInv * eq::Vector3f::ZERO;
    eq::Vector3f       front = modelInv * eq::Vector3f( 0.0f, 0.0f, -1.0f );

    front -= zero;
    front.normalize();
    front *= boundingSphere.w();

    const eq::Vector3f center = boundingSphere.get_sub_vector< 3 >() + 
                        frameData.getCameraTranslation( ).get_sub_vector< 3 >();
    const eq::Vector3f nearPoint  = headTransform * ( center - front );
    const eq::Vector3f farPoint   = headTransform * ( center + front );

    if( ortho )
    {
        EQASSERT( fabs( farPoint.z() - nearPoint.z() ) > 
                  std::numeric_limits< float >::epsilon( ));
        setNearFar( -nearPoint.z(), -farPoint.z() );
    }
    else
    {
        // estimate minimal value of near plane based on frustum size
        const eq::Frustumf& frustum = getFrustum();
        const float width  = fabs( frustum.right() - frustum.left() );
        const float height = fabs( frustum.top() - frustum.bottom() );
        const float size   = EQ_MIN( width, height );
        const float minNear = frustum.near_plane() / size * .001f;

        const float zNear = EQ_MAX( minNear, -nearPoint.z() );
        const float zFar  = EQ_MAX( zNear * 2.f, -farPoint.z() );

        setNearFar( zNear, zFar );
    }
}

void Channel::_initFrustum( eq::FrustumCullerf& culler,
                            const mesh::BoundingSphere& boundingSphere )
{
    // setup frustum cull helper
    const FrameData& frameData = _getFrameData();
    const eq::Matrix4f& rotation     = frameData.getCameraRotation();
    const eq::Matrix4f& modelRotation = frameData.getModelRotation();
          eq::Matrix4f  translation   = eq::Matrix4f::IDENTITY;
    translation.set_translation( frameData.getCameraTranslation());

    const eq::Matrix4f modelView = 
        getHeadTransform() * rotation * translation * modelRotation;

    const bool ortho = frameData.useOrtho();
    const eq::Frustumf& frustum      = ortho ? getOrtho() : getFrustum();
    const eq::Matrix4f  projection   = ortho ? frustum.compute_ortho_matrix() :
                                               frustum.compute_matrix();
    culler.setup( projection * modelView );
}
}
