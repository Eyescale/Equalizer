
/* Copyright (c) 2006-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
 *                          Tobias Wolf <twolf@access.unizh.ch>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
        , _frameRestart( 0 )
{
}

bool Channel::configInit( const eq::uint128_t& initID )
{
    if( !eq::Channel::configInit( initID ))
        return false;

    setNearFar( 0.1f, 10.0f );
    _model = 0;
    _modelID = 0;
    return true;
}

bool Channel::configExit()
{
    for( size_t i = 0; i < eq::NUM_EYES; ++i )
    {
        delete _accum[ i ].buffer;
        _accum[ i ].buffer = 0;
    }

    return eq::Channel::configExit();
}

void Channel::frameClear( const eq::uint128_t& /*frameID*/ )
{
    if( stopRendering( ))
        return;

    _initJitter();
    resetRegions();

    const FrameData& frameData = _getFrameData();
    const int32_t eyeIndex = lunchbox::getIndexOfLastBit( getEye() );
    if( _isDone() && !_accum[ eyeIndex ].transfer )
        return;

    applyBuffer();
    applyViewport();

    const eq::View* view = getView();
    if( view && frameData.getCurrentViewID() == view->getID( ))
        glClearColor( 1.f, 1.f, 1.f, 0.f );
#ifndef NDEBUG
    else if( getenv( "EQ_TAINT_CHANNELS" ))
    {
        const eq::Vector3ub color = getUniqueColor();
        glClearColor( color.r()/255.f, color.g()/255.f, color.b()/255.f, 0.f );
    }
#endif // NDEBUG
    else
        glClearColor( 0.f, 0.f, 0.f, 0.0f );

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void Channel::frameDraw( const eq::uint128_t& frameID )
{
    if( stopRendering( ))
        return;

    _initJitter();
    if( _isDone( ))
        return;

    Window* window = static_cast< Window* >( getWindow( ));
    VertexBufferState& state = window->getState();
    const Model* oldModel = _model;
    const Model* model = _getModel();

    if( oldModel != model )
        state.setFrustumCulling( false ); // create all display lists/VBOs

    if( model )
        _updateNearFar( model->getBoundingSphere( ));

    eq::Channel::frameDraw( frameID ); // Setup OpenGL state

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

    const eq::Vector3f& position = frameData.getCameraPosition();

    glMultMatrixf( frameData.getCameraRotation().array );
    glTranslatef( position.x(), position.y(), position.z() );
    glMultMatrixf( frameData.getModelRotation().array );

    if( frameData.getColorMode() == COLOR_DEMO )
    {
        const eq::Vector3ub color = getUniqueColor();
        glColor3ub( color.r(), color.g(), color.b() );
    }
    else
        glColor3f( .75f, .75f, .75f );

    if( model )
        _drawModel( model );
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

    state.setFrustumCulling( true );
    Accum& accum = _accum[ lunchbox::getIndexOfLastBit( getEye()) ];
    accum.stepsDone = LB_MAX( accum.stepsDone,
                              getSubPixel().size * getPeriod( ));
    accum.transfer = true;
}

void Channel::frameAssemble( const eq::uint128_t& frameID,
                             const eq::Frames& frames )
{
    if( stopRendering( ))
        return;

    if( _isDone( ))
        return;

    Accum& accum = _accum[ lunchbox::getIndexOfLastBit( getEye()) ];

    if( getPixelViewport() != _currentPVP )
    {
        accum.transfer = true;

        if( accum.buffer && !accum.buffer->usesFBO( ))
        {
            LBWARN << "Current viewport different from view viewport, "
                   << "idle anti-aliasing not implemented." << std::endl;
            accum.step = 0;
        }

        eq::Channel::frameAssemble( frameID, frames );
        return;
    }
    // else

    accum.transfer = true;
    for( eq::Frame* frame : frames )
    {
        const eq::SubPixel& subPixel =
            frame->getFrameData()->getContext().subPixel;

        if( subPixel != eq::SubPixel::ALL )
            accum.transfer = false;

        accum.stepsDone = LB_MAX( accum.stepsDone, subPixel.size *
                                  frame->getFrameData()->getContext().period );
    }

    applyBuffer();
    applyViewport();
    setupAssemblyState();

    try
    {
        eq::Compositor::assembleFrames( frames, this, accum.buffer );
    }
    catch( const co::Exception& e )
    {
        LBWARN << e.what() << std::endl;
    }

    resetAssemblyState();
}

void Channel::frameReadback( const eq::uint128_t& frameID,
                             const eq::Frames& frames )
{
    if( stopRendering() || _isDone( ))
        return;

    const FrameData& frameData = _getFrameData();
    for( eq::FramesCIter i = frames.begin(); i != frames.end(); ++i )
    {
        eq::Frame* frame = *i;
        // OPT: Drop alpha channel from all frames during network transport
        frame->setAlphaUsage( false );

        if( frameData.isIdle( ))
            frame->setQuality( eq::Frame::BUFFER_COLOR, 1.f );
        else
            frame->setQuality( eq::Frame::BUFFER_COLOR, frameData.getQuality());

        if( frameData.useCompression( ))
            frame->useCompressor( eq::Frame::BUFFER_COLOR, EQ_COMPRESSOR_AUTO );
        else
            frame->useCompressor( eq::Frame::BUFFER_COLOR, EQ_COMPRESSOR_NONE );
    }

    eq::Channel::frameReadback( frameID, frames );
}

void Channel::frameStart( const eq::uint128_t& frameID,
                          const uint32_t frameNumber )
{
    if( stopRendering( ))
        return;

    for( size_t i = 0; i < eq::NUM_EYES; ++i )
        _accum[ i ].stepsDone = 0;

    eq::Channel::frameStart( frameID, frameNumber );
}

void Channel::frameViewStart( const eq::uint128_t& frameID )
{
    if( stopRendering( ))
        return;

    _currentPVP = getPixelViewport();
    _initJitter();
    eq::Channel::frameViewStart( frameID );
}

void Channel::frameFinish( const eq::uint128_t& frameID,
                           const uint32_t frameNumber )
{
    if( stopRendering( ))
        return;

    for( size_t i = 0; i < eq::NUM_EYES; ++i )
    {
        Accum& accum = _accum[ i ];
        if( accum.step > 0 )
        {
            if( int32_t( accum.stepsDone ) > accum.step )
                accum.step = 0;
            else
                accum.step -= accum.stepsDone;
        }
    }

    eq::Channel::frameFinish( frameID, frameNumber );
}

void Channel::frameViewFinish( const eq::uint128_t& frameID )
{
    if( stopRendering( ))
        return;

    applyBuffer();

    const FrameData& frameData = _getFrameData();
    Accum& accum = _accum[ lunchbox::getIndexOfLastBit( getEye()) ];

    if( accum.buffer )
    {
        const eq::PixelViewport& pvp = getPixelViewport();
        const bool isResized = accum.buffer->resize( pvp );

        if( isResized )
        {
            const View* view = static_cast< const View* >( getView( ));
            accum.buffer->clear();
            accum.step = view->getIdleSteps();
            accum.stepsDone = 0;
        }
        else if( frameData.isIdle( ))
        {
            setupAssemblyState();

            if( !_isDone() && accum.transfer )
                accum.buffer->accum();
            accum.buffer->display();

            resetAssemblyState();
        }
    }

    _drawOverlay();
    _drawHelp();

    if( frameData.useStatistics())
        drawStatistics();

    int32_t steps = 0;
    if( frameData.isIdle( ))
    {
        for( size_t i = 0; i < eq::NUM_EYES; ++i )
            steps = LB_MAX( steps, _accum[i].step );
    }
    else
    {
        const View* view = static_cast< const View* >( getView( ));
        steps = view ? view->getIdleSteps() : 0;
    }

    // if _jitterStep == 0 and no user redraw event happened, the app will exit
    // FSAA idle mode and block on the next redraw event.
    eq::Config* config = getConfig();
    config->sendEvent( IDLE_AA_LEFT ) << steps;

    eq::Channel::frameViewFinish( frameID );
}

bool Channel::useOrtho() const
{
    const FrameData& frameData = _getFrameData();
    return frameData.useOrtho();
}

const FrameData& Channel::_getFrameData() const
{
    const Pipe* pipe = static_cast<const Pipe*>( getPipe( ));
    return pipe->getFrameData();
}

bool Channel::_isDone() const
{
    const FrameData& frameData = _getFrameData();
    if( !frameData.isIdle( ))
        return false;

    const eq::SubPixel& subpixel = getSubPixel();
    const Accum& accum = _accum[ lunchbox::getIndexOfLastBit( getEye()) ];
    return int32_t( subpixel.index ) >= accum.step;
}

void Channel::_initJitter()
{
    if( !_initAccum( ))
        return;

    const FrameData& frameData = _getFrameData();
    if( frameData.isIdle( ))
        return;

    const View* view = static_cast< const View* >( getView( ));
    if( !view )
        return;

    const int32_t idleSteps = view->getIdleSteps();
    if( idleSteps == 0 )
        return;

    // ready for the next FSAA
    Accum& accum = _accum[ lunchbox::getIndexOfLastBit( getEye()) ];
    if( accum.buffer )
        accum.buffer->clear();
    accum.step = idleSteps;
}

bool Channel::_initAccum()
{
    View* view = static_cast< View* >( getNativeView( ));
    if( !view ) // Only alloc accum for dest
        return true;

    const eq::Eye eye = getEye();
    Accum& accum = _accum[ lunchbox::getIndexOfLastBit( eye ) ];

    if( accum.buffer ) // already done
        return true;

    if( accum.step == -1 ) // accum init failed last time
        return false;

    // Check unsupported cases
    if( !eq::util::Accum::usesFBO( glewGetContext( )))
    {
        for( size_t i = 0; i < eq::NUM_EYES; ++i )
        {
            if( _accum[ i ].buffer )
            {
                LBWARN << "glAccum-based accumulation does not support "
                       << "stereo, disabling idle anti-aliasing."
                       << std::endl;
                for( size_t j = 0; j < eq::NUM_EYES; ++j )
                {
                    delete _accum[ j ].buffer;
                    _accum[ j ].buffer = 0;
                    _accum[ j ].step = -1;
                }

                view->setIdleSteps( 0 );
                return false;
            }
        }
    }

    // set up accumulation buffer
    accum.buffer = new eq::util::Accum( glewGetContext( ));
    const eq::PixelViewport& pvp = getPixelViewport();
    LBASSERT( pvp.isValid( ));

    if( !accum.buffer->init( pvp, getWindow()->getColorFormat( )) ||
        accum.buffer->getMaxSteps() < 256 )
    {
        LBWARN <<"Accumulation buffer initialization failed, "
               << "idle AA not available." << std::endl;
        delete accum.buffer;
        accum.buffer = 0;
        accum.step = -1;
        return false;
    }

    // else
    LBVERB << "Initialized "
           << (accum.buffer->usesFBO() ? "FBO accum" : "glAccum")
           << " buffer for " << getName() << " " << getEye()
           << std::endl;

    view->setIdleSteps( accum.buffer ? 256 : 0 );
    return true;
}

bool Channel::stopRendering() const
{
    return getPipe()->getCurrentFrame() < _frameRestart;
}

eq::Vector2f Channel::getJitter() const
{
    const FrameData& frameData = _getFrameData();
    const Accum& accum = _accum[ lunchbox::getIndexOfLastBit( getEye()) ];

    if( !frameData.isIdle() || accum.step <= 0 )
        return eq::Channel::getJitter();

    const View* view = static_cast< const View* >( getView( ));
    if( !view || view->getIdleSteps() != 256 )
        return eq::Vector2f::ZERO;

    const eq::Vector2i jitterStep = _getJitterStep();
    if( jitterStep == eq::Vector2i::ZERO )
        return eq::Vector2f::ZERO;

    const eq::PixelViewport& pvp = getPixelViewport();
    const float pvp_w = float( pvp.w );
    const float pvp_h = float( pvp.h );
    const float frustum_w = float(( getFrustum().getWidth( )));
    const float frustum_h = float(( getFrustum().getHeight( )));

    const float pixel_w = frustum_w / pvp_w;
    const float pixel_h = frustum_h / pvp_h;

    const float sampleSize = 16.f; // sqrt( 256 )
    const float subpixel_w = pixel_w / sampleSize;
    const float subpixel_h = pixel_h / sampleSize;

    // Sample value randomly computed within the subpixel
    lunchbox::RNG rng;
    const eq::Pixel& pixel = getPixel();

    const float i = ( rng.get< float >() * subpixel_w +
                      float( jitterStep.x( )) * subpixel_w ) / float( pixel.w );
    const float j = ( rng.get< float >() * subpixel_h +
                      float( jitterStep.y( )) * subpixel_h ) / float( pixel.h );

    return eq::Vector2f( i, j );
}

static const uint32_t _primes[100] = {
    739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829,
    839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941,
    947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013, 1019, 1021, 1031, 1033,
    1039, 1049, 1051, 1061, 1063, 1069, 1087, 1091, 1093, 1097, 1103, 1109,
    1117, 1123, 1129, 1151, 1153, 1163, 1171, 1181, 1187, 1193, 1201, 1213,
    1217, 1223, 1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289, 1291,
    1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373, 1381, 1399,
    1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451 };

eq::Vector2i Channel::_getJitterStep() const
{
    const eq::SubPixel& subPixel = getSubPixel();
    const uint32_t channelID = subPixel.index;
    const View* view = static_cast< const View* >( getView( ));
    if( !view )
        return eq::Vector2i::ZERO;

    const uint32_t totalSteps = uint32_t( view->getIdleSteps( ));
    if( totalSteps != 256 )
        return eq::Vector2i::ZERO;

    const Accum& accum = _accum[ lunchbox::getIndexOfLastBit( getEye()) ];
    const uint32_t subset = totalSteps / getSubPixel().size;
    const uint32_t index = ( accum.step * _primes[ channelID % 100 ] )%subset +
                           ( channelID * subset );
    const uint32_t sampleSize = 16;
    const int dx = index % sampleSize;
    const int dy = index / sampleSize;

    return eq::Vector2i( dx, dy );
}

const Model* Channel::_getModel()
{
    Config* config = static_cast< Config* >( getConfig( ));
    const View* view = static_cast< const View* >( getView( ));
    const FrameData& frameData = _getFrameData();
    LBASSERT( !view || dynamic_cast< const View* >( getView( )));

    eq::uint128_t id = view ? view->getModelID() : frameData.getModelID();
    if( id == 0 )
        id = frameData.getModelID();
    if( id != _modelID )
    {
        _model = config->getModel( id );
        _modelID = id;
    }

    return _model;
}

void Channel::_drawModel( const Model* scene )
{
    Window* window = static_cast< Window* >( getWindow( ));
    VertexBufferState& state = window->getState();
    const FrameData& frameData = _getFrameData();

    if( frameData.getColorMode() == COLOR_MODEL && scene->hasColors( ))
        state.setColors( true );
    else
        state.setColors( false );
    state.setChannel( this );

    // Compute cull matrix
    const eq::Matrix4f& rotation = frameData.getCameraRotation();
    const eq::Matrix4f& modelRotation = frameData.getModelRotation();
    eq::Matrix4f position;
    position.setTranslation( frameData.getCameraPosition( ));

    const eq::Frustumf& frustum = getFrustum();
    const eq::Matrix4f projection = useOrtho() ? frustum.computeOrthoMatrix() :
                                           frustum.computePerspectiveMatrix();
    const eq::Matrix4f& view = getHeadTransform();
    const eq::Matrix4f model = rotation * position * modelRotation;

    state.setProjectionModelViewMatrix( projection * view * model );
    state.setRange( triply::Range( &getRange().start ));

    const eq::Pipe* pipe = getPipe();
    const GLuint program = state.getProgram( pipe );
    if( program != VertexBufferState::INVALID )
        glUseProgram( program );

    scene->cullDraw( state );

    state.setChannel( 0 );
    if( program != VertexBufferState::INVALID )
        glUseProgram( 0 );

    const InitData& initData =
        static_cast<Config*>( getConfig( ))->getInitData();
    if( initData.useROI( ))
        // declare empty region in case nothing is in frustum
        declareRegion( eq::PixelViewport( ));
    else
        declareRegion( getPixelViewport( ));

#ifndef NDEBUG
    outlineViewport();
#endif
}

void Channel::_drawOverlay()
{
    // Draw the overlay logo
    const Window* window = static_cast<Window*>( getWindow( ));
    const eq::util::Texture* texture = window->getLogoTexture();
    if( !texture )
        return;

    applyOverlayState();
    EQ_GL_CALL( glDisable( GL_COLOR_LOGIC_OP ));
    EQ_GL_CALL( glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ));

    // logo
    EQ_GL_CALL( glEnable( GL_BLEND ));
    EQ_GL_CALL( glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ));
    const GLenum target = texture->getTarget();
    EQ_GL_CALL( glEnable( target ));
    texture->bind();
    EQ_GL_CALL( glTexParameteri( target, GL_TEXTURE_MAG_FILTER, GL_LINEAR ));
    EQ_GL_CALL( glTexParameteri( target, GL_TEXTURE_MIN_FILTER, GL_LINEAR ));

    const float tWidth = float( texture->getWidth( ));
    const float tHeight = float( texture->getHeight( ));

    const float width = target == GL_TEXTURE_2D ? 1.0f : tWidth;
    const float height = target == GL_TEXTURE_2D ? 1.0f : tHeight;

    glBegin( GL_QUADS ); {
        glTexCoord2f( 0, 0 );
        glVertex3f( 5.0f, 5.0f, 0.0f );

        glTexCoord2f( width, 0 );
        glVertex3f( tWidth + 5.0f, 5.0f, 0.0f );

        glTexCoord2f( width, height );
        glVertex3f( tWidth + 5.0f, tHeight + 5.0f, 0.0f );

        glTexCoord2f( 0, height );
        glVertex3f( 5.0f, tHeight + 5.0f, 0.0f );

    } glEnd();

    EQ_GL_CALL( glDisable( target ));
    EQ_GL_CALL( glDisable( GL_BLEND ));
    resetOverlayState();
}

void Channel::_drawHelp()
{
    const FrameData& frameData = _getFrameData();
    std::string message = frameData.getMessage();

    if( !frameData.showHelp() && message.empty( ))
        return;

    applyOverlayState();

    const eq::PixelViewport& pvp = getPixelViewport();
    const eq::Viewport& vp = getViewport();
    const float height = pvp.h / vp.h;

    if( !message.empty( ))
    {
        const eq::util::BitmapFont* font = getWindow()->getMediumFont();

        const float width = pvp.w / vp.w;
        const float xOffset = vp.x * width;

        const float yOffset = vp.y * height;
        const float yPos = 0.618f * height;
        float y = yPos - yOffset;

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

    if( frameData.showHelp( ))
    {
        const eq::util::BitmapFont* font = getWindow()->getSmallFont();
        std::string help = EqPly::getHelp();
        float y = height - 16.f;

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
    resetOverlayState();
}

void Channel::_updateNearFar( const triply::BoundingSphere& boundingSphere )
{
    // compute dynamic near/far plane of whole model
    const FrameData& frameData = _getFrameData();
    const eq::Matrix4f& rotation = frameData.getCameraRotation();
    const eq::Matrix4f& view = getHeadTransform() * rotation;
    const eq::Matrix4f& viewInv = view.inverse();

    const eq::Vector3f& zero  = viewInv * eq::Vector3f::ZERO;
    eq::Vector3f        front = viewInv * eq::Vector3f( 0.0f, 0.0f, -1.0f );

    front -= zero;
    front.normalize();
    front *= boundingSphere.w();

    const eq::Vector3f& center = frameData.getCameraPosition() -
                                 boundingSphere.get_sub_vector< 3, 0 >();
    const eq::Vector3f nearPoint  = view * ( center - front );
    const eq::Vector3f farPoint   = view * ( center + front );

    if( useOrtho( ))
    {
        LBASSERTINFO( fabs( farPoint.z() - nearPoint.z() ) >
                      std::numeric_limits< float >::epsilon(),
                      nearPoint << " == " << farPoint );
        setNearFar( -nearPoint.z(), -farPoint.z() );
    }
    else
    {
        // estimate minimal value of near plane based on frustum size
        const eq::Frustumf& frustum = getFrustum();
        const float width  = std::fabs( frustum.right() - frustum.left() );
        const float height = std::fabs( frustum.top() - frustum.bottom() );
        const float size   = std::min( width, height );
        const float minNear = std::fabs( frustum.nearPlane() / size * .001f );

        const float zNear = std::max( minNear, -nearPoint.z() );
        const float zFar  = std::max( zNear * 2.f, -farPoint.z() );

        setNearFar( zNear, zFar );
    }
}

}
