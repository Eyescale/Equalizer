
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Cedric Stalder <cedric.stalder@gmail.com>
 *               2011-2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "channelStatistics.h"
#include "client.h"
#include "compositor.h"
#include "config.h"
#ifndef EQ_2_0_API
#  include "configEvent.h"
#endif
#include "error.h"
#include "frame.h"
#include "frameData.h"
#include "gl.h"
#include "global.h"
#include "image.h"
#include "jitter.h"
#include "log.h"
#include "node.h"
#include "nodeFactory.h"
#include "pipe.h"
#include "pixelData.h"
#include "server.h"
#include "systemWindow.h"

#include <eq/util/accum.h>
#include <eq/util/frameBufferObject.h>
#include <eq/util/objectManager.h>
#include <eq/fabric/commands.h>
#include <eq/fabric/task.h>
#include <eq/fabric/tile.h>

#include <co/connectionDescription.h>
#include <co/exception.h>
#include <co/objectICommand.h>
#include <co/queueSlave.h>
#include <lunchbox/rng.h>
#include <lunchbox/scopedMutex.h>

#ifdef EQ_USE_GLSTATS
#  include "detail/statsRenderer.h"
#  include <GLStats/data.h>
#  include <GLStats/entity.h>
#  include <GLStats/item.h>
#  include <GLStats/renderer.h>
#  include <GLStats/thread.h>
#  include <GLStats/type.h>
#endif

#include <bitset>
#include <set>

#include "detail/channel.ipp"

namespace eq
{
/** @cond IGNORE */
typedef fabric::Channel< Window, Channel > Super;
typedef co::CommandFunc<Channel> CmdFunc;
using detail::STATE_STOPPED;
using detail::STATE_INITIALIZING;
using detail::STATE_RUNNING;
using detail::STATE_FAILED;
/** @endcond */

Channel::Channel( Window* parent )
        : Super( parent )
        , _impl( new detail::Channel )
{}

Channel::~Channel()
{
    delete _impl;
}

void Channel::attach( const UUID& id, const uint32_t instanceID )
{
    Super::attach( id, instanceID );
    co::CommandQueue* queue = getPipeThreadQueue();
    co::CommandQueue* commandQ = getCommandThreadQueue();
    co::CommandQueue* tmitQ = &getNode()->transmitter.getQueue();
    co::CommandQueue* transferQ = getPipe()->getTransferThreadQueue();

    registerCommand( fabric::CMD_CHANNEL_CONFIG_INIT,
                     CmdFunc( this, &Channel::_cmdConfigInit ), queue );
    registerCommand( fabric::CMD_CHANNEL_CONFIG_EXIT,
                     CmdFunc( this, &Channel::_cmdConfigExit ), queue );
    registerCommand( fabric::CMD_CHANNEL_FRAME_START,
                     CmdFunc( this, &Channel::_cmdFrameStart ), queue );
    registerCommand( fabric::CMD_CHANNEL_FRAME_FINISH,
                     CmdFunc( this, &Channel::_cmdFrameFinish ), queue );
    registerCommand( fabric::CMD_CHANNEL_FRAME_CLEAR,
                     CmdFunc( this, &Channel::_cmdFrameClear ), queue );
    registerCommand( fabric::CMD_CHANNEL_FRAME_DRAW,
                     CmdFunc( this, &Channel::_cmdFrameDraw ), queue );
    registerCommand( fabric::CMD_CHANNEL_FRAME_DRAW_FINISH,
                     CmdFunc( this, &Channel::_cmdFrameDrawFinish ), queue );
    registerCommand( fabric::CMD_CHANNEL_FRAME_ASSEMBLE,
                     CmdFunc( this, &Channel::_cmdFrameAssemble ), queue );
    registerCommand( fabric::CMD_CHANNEL_FRAME_READBACK,
                     CmdFunc( this, &Channel::_cmdFrameReadback ), queue );
    registerCommand( fabric::CMD_CHANNEL_FRAME_TRANSMIT_IMAGE,
                     CmdFunc( this, &Channel::_cmdFrameTransmitImage ), tmitQ );
    registerCommand( fabric::CMD_CHANNEL_FRAME_SET_READY,
                     CmdFunc( this, &Channel::_cmdFrameSetReady ), transferQ );
    registerCommand( fabric::CMD_CHANNEL_FRAME_SET_READY_NODE,
                     CmdFunc( this, &Channel::_cmdFrameSetReadyNode ), tmitQ );
    registerCommand( fabric::CMD_CHANNEL_FRAME_VIEW_START,
                     CmdFunc( this, &Channel::_cmdFrameViewStart ), queue );
    registerCommand( fabric::CMD_CHANNEL_FRAME_VIEW_FINISH,
                     CmdFunc( this, &Channel::_cmdFrameViewFinish ), queue );
    registerCommand( fabric::CMD_CHANNEL_STOP_FRAME,
                     CmdFunc( this, &Channel::_cmdStopFrame ), commandQ );
    registerCommand( fabric::CMD_CHANNEL_FRAME_TILES,
                     CmdFunc( this, &Channel::_cmdFrameTiles ), queue );
    registerCommand( fabric::CMD_CHANNEL_FINISH_READBACK,
                     CmdFunc( this, &Channel::_cmdFinishReadback ), transferQ );
    registerCommand( fabric::CMD_CHANNEL_DELETE_TRANSFER_CONTEXT,
                     CmdFunc( this,&Channel::_cmdDeleteTransferContext ),
                     transferQ );
}

co::CommandQueue* Channel::getPipeThreadQueue()
{
    return getWindow()->getPipeThreadQueue();
}

co::CommandQueue* Channel::getCommandThreadQueue()
{
    return getWindow()->getCommandThreadQueue();
}

uint32_t Channel::getCurrentFrame() const
{
    return getPipe()->getCurrentFrame();
}

Pipe* Channel::getPipe()
{
    Window* window = getWindow();
    LBASSERT( window );
    return ( window ? window->getPipe() : 0 );
}

const Pipe* Channel::getPipe() const
{
    const Window* window = getWindow();
    LBASSERT( window );
    return ( window ? window->getPipe() : 0 );
}

Node* Channel::getNode()
{
    Window* window = getWindow();
    LBASSERT( window );
    return ( window ? window->getNode() : 0 );
}
const Node* Channel::getNode() const
{
    const Window* window = getWindow();
    LBASSERT( window );
    return ( window ? window->getNode() : 0 );
}

Config* Channel::getConfig()
{
    Window* window = getWindow();
    LBASSERT( window );
    return ( window ? window->getConfig() : 0 );
}
const Config* Channel::getConfig() const
{
    const Window* window = getWindow();
    LBASSERT( window );
    return ( window ? window->getConfig() : 0 );
}

ServerPtr Channel::getServer()
{
    Window* window = getWindow();
    LBASSERT( window );
    return ( window ? window->getServer() : 0 );
}

Window::ObjectManager* Channel::getObjectManager()
{
    Window* window = getWindow();
    LBASSERT( window );
    return window->getObjectManager();
}

const DrawableConfig& Channel::getDrawableConfig() const
{
    const Window* window = getWindow();
    LBASSERT( window );
    if( _impl->fbo )
        return _impl->drawableConfig;

    return window->getDrawableConfig();
}

const GLEWContext* Channel::glewGetContext() const
{
    const Window* window = getWindow();
    LBASSERT( window );
    return window->glewGetContext();
}

bool Channel::configExit()
{
    delete _impl->fbo;
    _impl->fbo = 0;
    return true;
}
bool Channel::configInit( const uint128_t& )
{
    return _configInitFBO();
}

bool Channel::_configInitFBO()
{
    const uint32_t drawable = getDrawable();
    if( drawable == FB_WINDOW )
        return true;

    const Window* window = getWindow();
    if( !window->getSystemWindow()  ||
        !GLEW_ARB_texture_non_power_of_two || !GLEW_EXT_framebuffer_object )
    {
        setError( ERROR_FBO_UNSUPPORTED );
        return false;
    }

    // needs glew initialized (see above)
    _impl->fbo = new util::FrameBufferObject( glewGetContext( ));

    int depthSize = 0;
    if( drawable & FBO_DEPTH )
    {
        depthSize = window->getIAttribute( Window::IATTR_PLANES_DEPTH );
        if( depthSize < 1 )
            depthSize = 24;
    }

    int stencilSize = 0;
    if( drawable & FBO_STENCIL )
    {
        stencilSize = window->getIAttribute( Window::IATTR_PLANES_STENCIL );
        if( stencilSize < 1 )
            stencilSize = 1;
    }

    const PixelViewport& pvp = getNativePixelViewport();
    if( _impl->fbo->init( pvp.w, pvp.h, window->getColorFormat(), depthSize,
                          stencilSize ))
    {
        return true;
    }
    // else

    setError( _impl->fbo->getError( ));
    delete _impl->fbo;
    _impl->fbo = 0;
    return false;
}

void Channel::_initDrawableConfig()
{
    const Window* window = getWindow();
    _impl->drawableConfig = window->getDrawableConfig();
    if( !_impl->fbo )
        return;

    const util::Textures& colors = _impl->fbo->getColorTextures();
    if( !colors.empty( ))
    {
        switch( colors.front()->getType( ))
        {
            case GL_FLOAT:
                _impl->drawableConfig.colorBits = 32;
                break;
            case GL_HALF_FLOAT:
                _impl->drawableConfig.colorBits = 16;
                break;
            case GL_UNSIGNED_INT_10_10_10_2:
                _impl->drawableConfig.colorBits = 10;
                break;

            default:
                LBUNIMPLEMENTED;
            case GL_UNSIGNED_BYTE:
                _impl->drawableConfig.colorBits = 8;
                break;
        }
    }
}


void Channel::notifyViewportChanged()
{
    const PixelViewport oldPVP = getPixelViewport();
    Super::notifyViewportChanged();
    const PixelViewport& newPVP = getPixelViewport();

    if( newPVP == oldPVP )
        return;

    Event event;
    event.type       = Event::CHANNEL_RESIZE;
    event.originator = getID();
    event.serial     = getSerial();
    LBASSERT( event.originator != UUID::ZERO );
    event.resize.x   = newPVP.x;
    event.resize.y   = newPVP.y;
    event.resize.w   = newPVP.w;
    event.resize.h   = newPVP.h;

    processEvent( event );
}

void Channel::addStatistic( Event& event )
{
    {
        const uint32_t frameNumber = event.statistic.frameNumber;
        const size_t index = frameNumber % _impl->statistics->size();
        LBASSERT( index < _impl->statistics->size( ));
        LBASSERTINFO( _impl->statistics.data[ index ].used > 0, frameNumber );

        lunchbox::ScopedFastWrite mutex( _impl->statistics );
        Statistics& statistics = _impl->statistics.data[ index ].data;
        statistics.push_back( event.statistic );
    }
    processEvent( event );
}

//---------------------------------------------------------------------------
// operations
//---------------------------------------------------------------------------
void Channel::waitFrameFinished( const uint32_t frame ) const
{
    _impl->finishedFrame.waitGE( frame );
}

void Channel::frameClear( const uint128_t& )
{
    resetRegions();
    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));

#ifndef NDEBUG
    if( getenv( "EQ_TAINT_CHANNELS" ))
    {
        const Vector3ub color = getUniqueColor();
        glClearColor( color.r()/255.0f,
                      color.g()/255.0f,
                      color.b()/255.0f, 1.0f );
    }
#endif // NDEBUG

    EQ_GL_CALL( glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ));
}

void Channel::frameDraw( const uint128_t& )
{
    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));

    EQ_GL_CALL( glMatrixMode( GL_PROJECTION ));
    EQ_GL_CALL( glLoadIdentity( ));
    EQ_GL_CALL( applyFrustum( ));

    EQ_GL_CALL( glMatrixMode( GL_MODELVIEW ));
    EQ_GL_CALL( glLoadIdentity( ));
    EQ_GL_CALL( applyHeadTransform( ));
}

void Channel::frameAssemble( const uint128_t& )
{
    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));
    EQ_GL_CALL( setupAssemblyState( ));
    try
    {
        Compositor::assembleFrames( getInputFrames(), this, 0 );
    }
    catch( const co::Exception& e )
    {
        LBWARN << e.what() << std::endl;
    }
    EQ_GL_CALL( resetAssemblyState( ));
}

void Channel::frameReadback( const uint128_t& )
{
    const PixelViewport& region = getRegion();
    if( !region.hasArea( ))
        return;

    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));
    EQ_GL_CALL( setupAssemblyState( ));

    Window::ObjectManager*  glObjects   = getObjectManager();
    const DrawableConfig&   drawable    = getDrawableConfig();
    const Frames&           frames      = getOutputFrames();
    const PixelViewports&   regions     = getRegions();

    for( FramesCIter i = frames.begin(); i != frames.end(); ++i )
    {
        Frame* frame = *i;
        frame->startReadback( glObjects, drawable, regions );
    }

    EQ_GL_CALL( resetAssemblyState( ));
}

void Channel::startFrame( const uint32_t ) { /* nop */ }
void Channel::releaseFrame( const uint32_t ) { /* nop */ }
void Channel::releaseFrameLocal( const uint32_t ) { /* nop */ }

void Channel::frameStart( const uint128_t&, const uint32_t frameNumber )
{
    resetRegions();
    startFrame( frameNumber );
}
void Channel::frameFinish( const uint128_t&, const uint32_t frameNumber )
{
    releaseFrame( frameNumber );
}
void Channel::frameDrawFinish( const uint128_t&, const uint32_t frameNumber )
{
    releaseFrameLocal( frameNumber );
}
void Channel::frameViewStart( const uint128_t& ) { /* nop */ }
void Channel::frameViewFinish( const uint128_t& ) { /* nop */ }

void Channel::setupAssemblyState()
{
    const PixelViewport& pvp = getPixelViewport();
    Compositor::setupAssemblyState( pvp, glewGetContext( ));
}

void Channel::resetAssemblyState()
{
    Compositor::resetAssemblyState();
}

void Channel::_setRenderContext( RenderContext& context )
{
    overrideContext( context );
    Window* window = getWindow();
    window->_addRenderContext( context );
}

Frustumf Channel::getScreenFrustum() const
{
    const Pixel& pixel = getPixel();
    PixelViewport pvp( getPixelViewport( ));
    const Viewport& vp( getViewport( ));

    pvp.x = static_cast<int32_t>( pvp.w / vp.w * vp.x );
    pvp.y = static_cast<int32_t>( pvp.h / vp.h * vp.y );
    pvp.unapply( pixel );

    return eq::Frustumf( static_cast< float >( pvp.x ),
                         static_cast< float >( pvp.getXEnd( )),
                         static_cast< float >( pvp.y ),
                         static_cast< float >( pvp.getYEnd( )),
                         -1.f, 1.f );
}

util::FrameBufferObject* Channel::getFrameBufferObject()
{
    return _impl->fbo;
}

View* Channel::getView()
{
    LB_TS_THREAD( _pipeThread );
    Pipe* pipe = getPipe();
    return pipe->getView( getContext().view );
}

const View* Channel::getView() const
{
    LB_TS_THREAD( _pipeThread );
    const Pipe* pipe = getPipe();
    return pipe->getView( getContext().view );
}

co::QueueSlave* Channel::_getQueue( const UUID& queueID )
{
    LB_TS_THREAD( _pipeThread );
    Pipe* pipe = getPipe();
    return pipe->getQueue( queueID );
}

View* Channel::getNativeView()
{
    LB_TS_THREAD( _pipeThread );
    Pipe* pipe = getPipe();
    return pipe->getView( getNativeContext().view );
}

const View* Channel::getNativeView() const
{
    LB_TS_THREAD( _pipeThread );
    const Pipe* pipe = getPipe();
    return pipe->getView( getNativeContext().view );
}

void Channel::changeLatency( const uint32_t latency )
{
#ifndef NDEBUG
    for( detail::Channel::StatisticsRBCIter i = _impl->statistics->begin();
         i != _impl->statistics->end(); ++i )
    {
        LBASSERT( (*i).used == 0 );
    }
#endif //NDEBUG
    _impl->statistics->resize( latency + 1 );
}

//---------------------------------------------------------------------------
// apply convenience methods
//---------------------------------------------------------------------------
void Channel::applyFrameBufferObject()
{
    LB_TS_THREAD( _pipeThread );
    if( _impl->fbo )
    {
        const PixelViewport& pvp = getNativePixelViewport();
        _impl->fbo->resize( pvp.w, pvp.h );
        _impl->fbo->bind();
    }
    else if( GLEW_EXT_framebuffer_object )
        glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
}

void Channel::applyBuffer()
{
    LB_TS_THREAD( _pipeThread );
    const Window* window = getWindow();
    if( !_impl->fbo && window->getSystemWindow()->getFrameBufferObject() == 0 )
    {
        EQ_GL_CALL( glReadBuffer( getReadBuffer( )));
        EQ_GL_CALL( glDrawBuffer( getDrawBuffer( )));
    }

    applyColorMask();
}

void Channel::bindFrameBuffer()
{
    LB_TS_THREAD( _pipeThread );
    const Window* window = getWindow();
    if( !window->getSystemWindow( ))
       return;

   if( _impl->fbo )
       applyFrameBufferObject();
   else
       window->bindFrameBuffer();
}

void Channel::applyColorMask() const
{
    LB_TS_THREAD( _pipeThread );
    const ColorMask& colorMask = getDrawBufferMask();
    glColorMask( colorMask.red, colorMask.green, colorMask.blue, true );
}

void Channel::applyViewport() const
{
    LB_TS_THREAD( _pipeThread );
    const PixelViewport& pvp = getPixelViewport();

    if( !pvp.hasArea( ))
    {
        LBERROR << "Can't apply viewport " << pvp << std::endl;
        return;
    }

    EQ_GL_CALL( glViewport( pvp.x, pvp.y, pvp.w, pvp.h ));
    EQ_GL_CALL( glScissor( pvp.x, pvp.y, pvp.w, pvp.h ));
}

void Channel::applyFrustum() const
{
    LB_TS_THREAD( _pipeThread );
    if( useOrtho( ))
        applyOrtho();
    else
        applyPerspective();
}

void Channel::applyPerspective() const
{
    LB_TS_THREAD( _pipeThread );
    Frustumf frustum = getPerspective();
    const Vector2f jitter = getJitter();

    frustum.apply_jitter( jitter );
    EQ_GL_CALL( glFrustum( frustum.left(), frustum.right(),
                           frustum.bottom(), frustum.top(),
                           frustum.near_plane(), frustum.far_plane() ));
}

void Channel::applyOrtho() const
{
    LB_TS_THREAD( _pipeThread );
    Frustumf ortho = getOrtho();
    const Vector2f jitter = getJitter();

    ortho.apply_jitter( jitter );
    EQ_GL_CALL( glOrtho( ortho.left(), ortho.right(),
                         ortho.bottom(), ortho.top(),
                         ortho.near_plane(), ortho.far_plane() ));
}

void Channel::applyScreenFrustum() const
{
    LB_TS_THREAD( _pipeThread );
    const Frustumf frustum = getScreenFrustum();
    EQ_GL_CALL( glOrtho( frustum.left(), frustum.right(),
                         frustum.bottom(), frustum.top(),
                         frustum.near_plane(), frustum.far_plane() ));
}

void Channel::applyHeadTransform() const
{
    LB_TS_THREAD( _pipeThread );
    if( useOrtho( ))
        applyOrthoTransform();
    else
        applyPerspectiveTransform();
}

void Channel::applyPerspectiveTransform() const
{
    LB_TS_THREAD( _pipeThread );
    const Matrix4f& xfm = getPerspectiveTransform();
    EQ_GL_CALL( glMultMatrixf( xfm.array ));
}

void Channel::applyOrthoTransform() const
{
    LB_TS_THREAD( _pipeThread );
    const Matrix4f& xfm = getOrthoTransform();
    EQ_GL_CALL( glMultMatrixf( xfm.array ));
}

namespace
{
static Vector2f* _lookupJitterTable( const uint32_t size )
{
    switch( size )
    {
        case 2:
            return Jitter::j2;
        case 3:
            return Jitter::j3;
        case 4:
            return Jitter::j4;
        case 8:
            return Jitter::j8;
        case 15:
            return Jitter::j15;
        case 24:
            return Jitter::j24;
        case 66:
            return Jitter::j66;
        default:
            break;
    }
    return 0;
}
}

Vector2f Channel::getJitter() const
{
    const SubPixel& subpixel = getSubPixel();
    if( subpixel == SubPixel::ALL )
        return Vector2f::ZERO;

    // Compute a pixel size
    const PixelViewport& pvp = getPixelViewport();
    const float pvp_w = static_cast<float>( pvp.w );
    const float pvp_h = static_cast<float>( pvp.h );

    const Frustumf& frustum = getFrustum();
    const float frustum_w = frustum.get_width();
    const float frustum_h = frustum.get_height();

    const float pixel_w = frustum_w / pvp_w;
    const float pixel_h = frustum_h / pvp_h;

    const Vector2f pixelSize( pixel_w, pixel_h );

    Vector2f* table = _lookupJitterTable( subpixel.size );
    Vector2f jitter;
    if( !table )
    {
        static lunchbox::RNG rng;
        jitter.x() = rng.get< float >();
        jitter.y() = rng.get< float >();
    }
    else
        jitter = table[ subpixel.index ];

    const Pixel& pixel = getPixel();
    jitter.x() /= static_cast<float>( pixel.w );
    jitter.y() /= static_cast<float>( pixel.h );

    return jitter * pixelSize;
}

bool Channel::isStopped() const { return _impl->state == STATE_STOPPED; }

const Frames& Channel::getInputFrames()
{
    LB_TS_THREAD( _pipeThread );
    return _impl->inputFrames;
}

const Frames& Channel::getOutputFrames()
{
    LB_TS_THREAD( _pipeThread );
    return _impl->outputFrames;
}

const Vector3ub& Channel::getUniqueColor() const { return _impl->color; }

void Channel::resetRegions()
{
    _impl->regions.clear();
}

void Channel::declareRegion( const eq::Viewport& vp )
{
    eq::PixelViewport region = getPixelViewport();
    region.x = 0;
    region.y = 0;

    region.apply( vp );
    declareRegion( region );
}

namespace
{

#ifndef NDEBUG
bool _hasOverlap( PixelViewports& regions )
{
    if( regions.size() < 2 )
        return false;

    for( size_t i = 0; i < regions.size()-1; ++i )
        for( size_t j = i+1; j < regions.size(); ++j )
        {
            PixelViewport pv = regions[j];
            pv.intersect( regions[i] );
            if( pv.hasArea( ))
                return true;
        }
    return false;
}
#endif

/** Remove overlapping regions by merging them */
bool _removeOverlap( PixelViewports& regions )
{
    if( regions.size() < 2 )
        return false;

    for( size_t i = 0; i < regions.size()-1; ++i )
        for( size_t j = i+1; j < regions.size(); ++j )
        {
            PixelViewport pvp = regions[i];
            if( !pvp.hasArea( ))
            {
                std::swap( regions[i], regions.back() );
                regions.pop_back();
                return true;
            }

            pvp.intersect( regions[j] );
            if( pvp.hasArea( ))
            {
                regions[i].merge( regions[j] );
                std::swap( regions[j], regions.back() );
                regions.pop_back();
                return true;
            }
        }
    return false;
}
}

void Channel::declareRegion( const PixelViewport& region )
{
    PixelViewports& regions = _impl->regions;

    PixelViewport clippedRegion = region;
    PixelViewport pvp = getPixelViewport();
    pvp.x = 0;
    pvp.y = 0;

    clippedRegion.intersect( pvp );

    if( clippedRegion.hasArea( ))
    {
        regions.push_back( clippedRegion );
#ifndef NDEBUG
        const PixelViewport pvpBefore = getRegion();
#endif
        while( _removeOverlap( regions )) { /* nop */ }

#ifndef NDEBUG
        LBASSERT( !_hasOverlap( regions ));
        LBASSERT( pvpBefore == getRegion( ));
#endif
        return;
    }

    if( regions.empty( )) // set on first declaration of empty ROI
        regions.push_back( PixelViewport( 0, 0, 0, 0 ));
}


PixelViewport Channel::getRegion() const
{
    PixelViewport region;
    for( size_t i = 0; i < _impl->regions.size(); ++i )
        region.merge( _impl->regions[i] );

    return region;
}

const PixelViewports& Channel::getRegions() const
{
    return _impl->regions;
}

bool Channel::processEvent( const Event& event )
{
    Event configEvent = event;

    switch( event.type )
    {
        case Event::CHANNEL_POINTER_MOTION:
        case Event::CHANNEL_POINTER_BUTTON_PRESS:
        case Event::CHANNEL_POINTER_BUTTON_RELEASE:
        case Event::STATISTIC:
            break;

        case Event::CHANNEL_RESIZE:
        {
            const UUID& viewID = getNativeContext().view.identifier;
            if( viewID == UUID::ZERO )
                return true;

            // transform to view event, which is meaningful for the config
            configEvent.type       = Event::VIEW_RESIZE;
            configEvent.originator = viewID;

            ResizeEvent& resize = configEvent.resize;
            resize.dw = resize.w / float( _impl->initialSize.x( ));
            resize.dh = resize.h / float( _impl->initialSize.y( ));
            break;
        }

        default:
            LBWARN << "Unhandled channel event of type " << event.type
                   << std::endl;
            LBUNIMPLEMENTED;
    }

    Config* config = getConfig();
    config->sendEvent( configEvent.type ) << configEvent;
    return true;
}

void Channel::drawStatistics()
{
    const PixelViewport& pvp = getPixelViewport();
    LBASSERT( pvp.hasArea( ));
    if( !pvp.hasArea( ))
        return;

    //----- setup
    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));
    EQ_GL_CALL( setupAssemblyState( ));

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    applyScreenFrustum();

    glMatrixMode( GL_MODELVIEW );
    glDisable( GL_LIGHTING );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glDisable( GL_COLOR_LOGIC_OP );

    Window* window = getWindow();

#ifdef EQ_USE_GLSTATS
    const Window::Font* font = window->getSmallFont();
    const Config* config = getConfig();
    const GLStats::Data& data = config->getStatistics();
    detail::StatsRenderer renderer( font );
    const Viewport& vp = getViewport();
    const uint32_t width = uint32_t( pvp.w/vp.w );
    const uint32_t height = uint32_t( pvp.h / vp.h);

    renderer.setViewport( width, height );
    renderer.draw( data );
#endif

    glColor3f( 1.f, 1.f, 1.f );
    window->drawFPS();
    EQ_GL_CALL( resetAssemblyState( ));
}

void Channel::outlineViewport()
{
    setupAssemblyState();

    const PixelViewport& pvp = getPixelViewport();
    glDisable( GL_LIGHTING );
    glColor3f( 1.0f, 1.0f, 1.0f );
    glBegin( GL_LINE_LOOP );
    {
        glVertex3f( pvp.x + .5f,         pvp.y + .5f,         0.f );
        glVertex3f( pvp.getXEnd() - .5f, pvp.y + .5f,         0.f );
        glVertex3f( pvp.getXEnd() - .5f, pvp.getYEnd() - .5f, 0.f );
        glVertex3f( pvp.x + .5f,         pvp.getYEnd() - .5f, 0.f );
    }
    glEnd();

    resetAssemblyState();
}

namespace detail
{
struct RBStat
{
    RBStat( eq::Channel* channel )
            : event( Statistic::CHANNEL_READBACK, channel )
            , uncompressed( 0 )
            , compressed( 0 )
        {
            event.event.statistic.plugins[0] = EQ_COMPRESSOR_NONE;
            event.event.statistic.plugins[1] = EQ_COMPRESSOR_NONE;
            LBASSERT( event.event.statistic.frameNumber > 0 );
        }

    lunchbox::SpinLock lock;
    ChannelStatistics event;
    size_t uncompressed;
    size_t compressed;

    void ref( void* ) { ++_refCount; }
    bool unref( void* )
        {
            if( --_refCount > 0 )
                return false;

            if( uncompressed > 0 && compressed > 0 )
                event.event.statistic.ratio = float( compressed ) /
                                                   float( uncompressed );
            else
                event.event.statistic.ratio = 1.0f;
            delete this;
            return true;
        }

    int32_t getRefCount() const { return _refCount; }

private:
    a_int32_t _refCount;
};
}

typedef lunchbox::RefPtr< detail::RBStat > RBStatPtr;

void Channel::_frameTiles( RenderContext& context, const bool isLocal,
                           const UUID& queueID, const uint32_t tasks,
                           const co::ObjectVersions& frames )
{
    _setRenderContext( context );

    frameTilesStart( context.frameID );

    RBStatPtr stat;
    if( tasks & fabric::TASK_READBACK )
    {
        _setOutputFrames( frames );
        stat = new detail::RBStat( this );
    }

    int64_t startTime = getConfig()->getTime();
    int64_t clearTime = 0;
    int64_t drawTime = 0;
    int64_t readbackTime = 0;
    bool hasAsyncReadback = false;

    co::QueueSlave* queue = _getQueue( queueID );
    LBASSERT( queue );
    for( ;; )
    {
        co::ObjectICommand tileCmd = queue->pop();
        if( !tileCmd.isValid( ))
            break;

        const Tile& tile = tileCmd.get< Tile >();
        context.apply( tile );

        const PixelViewport tilePVP = context.pvp;

        if ( !isLocal )
        {
            context.pvp.x = 0;
            context.pvp.y = 0;
        }

        if( tasks & fabric::TASK_CLEAR )
        {
            const int64_t time = getConfig()->getTime();
            frameClear( context.frameID );
            clearTime += getConfig()->getTime() - time;
        }

        if( tasks & fabric::TASK_DRAW )
        {
            const int64_t time = getConfig()->getTime();
            frameDraw( context.frameID );
            drawTime += getConfig()->getTime() - time;
        }

        if( tasks & fabric::TASK_READBACK )
        {
            const int64_t time = getConfig()->getTime();
            const Frames& outFrames = getOutputFrames();
            const size_t nFrames = outFrames.size();

            std::vector< size_t > nImages( nFrames, 0 );
            for( size_t i = 0; i < nFrames; ++i )
            {
                nImages[i] = outFrames[i]->getImages().size();
                outFrames[i]->getFrameData()->setPixelViewport(
                    getPixelViewport( ));
            }

            frameReadback( context.frameID );
            readbackTime += getConfig()->getTime() - time;

            for( size_t i = 0; i < nFrames; ++i )
            {
                const Frame* frame = outFrames[i];
                const Images& images = frame->getImages();
                for( size_t j = nImages[i]; j < images.size(); ++j )
                {
                    Image* image = images[j];
                    const PixelViewport& pvp = image->getPixelViewport();
                    image->setOffset( pvp.x + tilePVP.x,
                                      pvp.y + tilePVP.y );
                }
            }

            if( _asyncFinishReadback( nImages ))
                hasAsyncReadback = true;
        }
    }

    if( tasks & fabric::TASK_CLEAR )
    {
        ChannelStatistics event( Statistic::CHANNEL_CLEAR, this );
        event.event.statistic.startTime = startTime;
        startTime += clearTime;
        event.event.statistic.endTime = startTime;
    }

    if( tasks & fabric::TASK_DRAW )
    {
        ChannelStatistics event( Statistic::CHANNEL_DRAW, this );
        event.event.statistic.startTime = startTime;
        startTime += drawTime;
        event.event.statistic.endTime = startTime;
    }

    if( tasks & fabric::TASK_READBACK )
    {
        stat->event.event.statistic.startTime = startTime;
        startTime += readbackTime;
        stat->event.event.statistic.endTime = startTime;

        _setReady( hasAsyncReadback, stat.get( ));
        _resetOutputFrames();
    }

    frameTilesFinish( context.frameID );
    resetRenderContext();
}

void Channel::_refFrame( const uint32_t frameNumber )
{
    const size_t index = frameNumber % _impl->statistics->size();
    detail::Channel::FrameStatistics& stats = _impl->statistics.data[ index ];
    LBASSERTINFO( stats.used > 0, frameNumber );
    ++stats.used;
}

void Channel::_unrefFrame( const uint32_t frameNumber )
{
    const size_t index = frameNumber % _impl->statistics->size();
    detail::Channel::FrameStatistics& stats = _impl->statistics.data[ index ];
    if( --stats.used != 0 ) // Frame still in use
        return;

    ++stats.used; // otherwise assertion
    { ChannelStatistics event( Statistic::CHANNEL_FRAME_FINISH, this,
                               frameNumber ); }
    --stats.used;

    send( getServer(), fabric::CMD_CHANNEL_FRAME_FINISH_REPLY )
            << stats.region << frameNumber << stats.data;

    stats.data.clear();
    stats.region = Viewport::FULL;

    _impl->finishedFrame = frameNumber;
}

void Channel::_setOutputFrames( const co::ObjectVersions& frames )
{
    LB_TS_THREAD( _pipeThread );
    LBASSERT( _impl->outputFrames.empty( ))

    for( size_t i = 0; i < frames.size(); ++i )
    {
        Pipe*  pipe  = getPipe();
        Frame* frame = pipe->getFrame( frames[i], getEye(), true );
        LBASSERTINFO( stde::find( _impl->outputFrames, frame ) ==
                      _impl->outputFrames.end(),
                      "frame " << i << " " << frames[i] );

        _impl->outputFrames.push_back( frame );
    }
}

void Channel::_resetOutputFrames()
{
    LB_TS_THREAD( _pipeThread );
    _impl->outputFrames.clear();
}

//---------------------------------------------------------------------------
// Asynchronous image readback, compression and transmission
//---------------------------------------------------------------------------
void Channel::_frameReadback( const uint128_t& frameID,
                              const co::ObjectVersions& frames )
{
    LB_TS_THREAD( _pipeThread );

    RBStatPtr stat = new detail::RBStat( this );
    _setOutputFrames( frames );

    std::vector< size_t > nImages( frames.size(), 0 );
    for( size_t i = 0; i < frames.size(); ++i )
        nImages[i] = _impl->outputFrames[i]->getImages().size();

    frameReadback( frameID );
    LBASSERT( stat->event.event.statistic.frameNumber > 0 );
    const bool async = _asyncFinishReadback( nImages );
    _setReady( async, stat.get( ));
    _resetOutputFrames();
}

bool Channel::_asyncFinishReadback( const std::vector< size_t >& imagePos )
{
    LB_TS_THREAD( _pipeThread );

    bool hasAsyncReadback = false;
    const Frames& frames = getOutputFrames();
    LBASSERT( frames.size() == imagePos.size( ));

    for( size_t i = 0; i < frames.size(); ++i )
    {
        Frame* frame = frames[i];
        FrameDataPtr frameData = frame->getFrameData();
        const uint32_t frameNumber = getCurrentFrame();

        if( frameData->getBuffers() == 0 )
            continue;

        const Images& images = frameData->getImages();
        const size_t nImages = images.size();
        const Eye eye = getEye();
        const std::vector< uint128_t >& nodes = frame->getInputNodes( eye );
        const std::vector< uint128_t >& netNodes = frame->getInputNetNodes(eye);

        for( uint64_t j = imagePos[i]; j < nImages; ++j )
        {
            if( images[j]->hasAsyncReadback( )) // finish async readback
            {
                LBCHECK( getPipe()->startTransferThread( ));
                LBCHECK( getWindow()->createTransferWindow( ));

                hasAsyncReadback = true;
                _refFrame( frameNumber );

                send( getLocalNode(), fabric::CMD_CHANNEL_FINISH_READBACK )
                        << co::ObjectVersion( frameData ) << j << frameNumber
                        << getTaskID() << nodes << netNodes;
            }
            else // transmit images asynchronously
                _asyncTransmit( frameData, frameNumber, j, nodes, netNodes,
                                getTaskID( ));
        }
    }
    return hasAsyncReadback;
}

void Channel::_finishReadback( const co::ObjectVersion& frameDataVersion,
                               const uint64_t imageIndex,
                               const uint32_t frameNumber,
                               const uint32_t taskID,
                               const std::vector< uint128_t >& nodes,
                               const std::vector< uint128_t >& netNodes )
{
    LBLOG( LOG_TASKS|LOG_ASSEMBLY ) << "Finish readback" << std::endl;
    FrameDataPtr frameData = getNode()->getFrameData( frameDataVersion );
    const Images& images = frameData->getImages();
    Image* image = images[ imageIndex ];
    const GLEWContext* glewContext = getWindow()->getTransferGlewContext();

    LBASSERT( frameData );
    LBASSERT( images.size() > imageIndex );
    LBASSERT( image->hasAsyncReadback( ));

    image->finishReadback( frameData->getZoom(), glewContext );
    LBASSERT( !image->hasAsyncReadback( ));

    // schedule async image tranmission
    _asyncTransmit( frameData, frameNumber, imageIndex, nodes,
                    netNodes, taskID );
}

void Channel::_asyncTransmit( FrameDataPtr frame, const uint32_t frameNumber,
                              const uint64_t image,
                              const std::vector<uint128_t>& nodes,
                              const std::vector< uint128_t >& netNodes,
                              const uint32_t taskID )
{
    LBASSERT( nodes.size() == netNodes.size( ));
    std::vector< uint128_t >::const_iterator j = netNodes.begin();
    for( std::vector< uint128_t >::const_iterator i = nodes.begin();
         i != nodes.end(); ++i, ++j )
    {
        _refFrame( frameNumber );

        LBLOG( LOG_TASKS|LOG_ASSEMBLY ) << "Start transmit frame data " << frame
                                        << " receiver " << *i << " on " << *j
                                        << std::endl;
        send( getLocalNode(), fabric::CMD_CHANNEL_FRAME_TRANSMIT_IMAGE )
                << co::ObjectVersion( frame ) << *i << *j << image
                << frameNumber << taskID;
    }
}

void Channel::_transmitImage( const co::ObjectVersion& frameDataVersion,
                              const uint128_t& nodeID,
                              const uint128_t& netNodeID,
                              const uint64_t imageIndex,
                              const uint32_t frameNumber,
                              const uint32_t taskID )
{
    LBLOG( LOG_TASKS|LOG_ASSEMBLY ) << "Transmit" << std::endl;
    FrameDataPtr frameData = getNode()->getFrameData( frameDataVersion );
    LBASSERT( frameData );

    if( frameData->getBuffers() == 0 )
    {
        LBWARN << "No buffers for frame data" << std::endl;
        return;
    }

    ChannelStatistics transmitEvent( Statistic::CHANNEL_FRAME_TRANSMIT, this,
                                     frameNumber );
    transmitEvent.event.statistic.task = taskID;

    const Images& images = frameData->getImages();
    Image* image = images[ imageIndex ];
    LBASSERT( images.size() > imageIndex );

    if( image->getStorageType() == Frame::TYPE_TEXTURE )
    {
        LBWARN << "Can't transmit image of type TEXTURE" << std::endl;
        LBUNIMPLEMENTED;
        return;
    }

    co::LocalNodePtr localNode = getLocalNode();
    co::NodePtr toNode = localNode->connect( netNodeID );
    if( !toNode || !toNode->isReachable( ))
    {
        LBWARN << "Can't connect node " << netNodeID
               << " to send image data" << std::endl;
        return;
    }

    co::ConnectionPtr connection = toNode->getConnection();
    co::ConstConnectionDescriptionPtr description =connection->getDescription();

    // use compression on links up to 2 GBit/s
    const bool useCompression = ( description->bandwidth <= 262144 );

    std::vector< const PixelData* > pixelDatas;
    std::vector< float > qualities;

    uint32_t commandBuffers = Frame::BUFFER_NONE;
    uint64_t imageDataSize = 0;


    {
        uint64_t rawSize( 0 );
        ChannelStatistics compressEvent( Statistic::CHANNEL_FRAME_COMPRESS,
                                         this, frameNumber,
                                         useCompression ? AUTO : OFF );
        compressEvent.event.statistic.task = taskID;
        compressEvent.event.statistic.ratio = 1.0f;
        compressEvent.event.statistic.plugins[0] = EQ_COMPRESSOR_NONE;
        compressEvent.event.statistic.plugins[1] = EQ_COMPRESSOR_NONE;

        // Prepare image pixel data
        Frame::Buffer buffers[] = {Frame::BUFFER_COLOR,Frame::BUFFER_DEPTH};

        // for each image attachment
        for( unsigned j = 0; j < 2; ++j )
        {
            Frame::Buffer buffer = buffers[j];
            if( image->hasPixelData( buffer ))
            {
                // format, type, nChunks, compressor name
                imageDataSize += sizeof( FrameData::ImageHeader );

                const PixelData& data = useCompression ?
                    image->compressPixelData( buffer ) :
                    image->getPixelData( buffer );
                pixelDatas.push_back( &data );
                qualities.push_back( image->getQuality( buffer ));

                if( data.isCompressed )
                {
                    const uint32_t nElements =
                        uint32_t( data.compressedSize.size( ));
                    for( uint32_t k = 0 ; k < nElements; ++k )
                    {
                        imageDataSize += sizeof( uint64_t );
                        imageDataSize += data.compressedSize[ k ];
                    }
                    compressEvent.event.statistic.plugins[j] =
                        data.compressorName;
                }
                else
                {
                    imageDataSize += sizeof( uint64_t );
                    imageDataSize += image->getPixelDataSize( buffer );
                }

                commandBuffers |= buffer;
                rawSize += image->getPixelDataSize( buffer );
            }
        }

        if( rawSize > 0 )
            compressEvent.event.statistic.ratio =
            static_cast< float >( imageDataSize ) /
            static_cast< float >( rawSize );
    }

    if( pixelDatas.empty( ))
        return;

    // send image pixel data command
    co::LocalNode::SendToken token;
    if( getIAttribute( IATTR_HINT_SENDTOKEN ) == ON )
    {
        ChannelStatistics waitEvent( Statistic::CHANNEL_FRAME_WAIT_SENDTOKEN,
                                     this, frameNumber );
        waitEvent.event.statistic.task = taskID;
        token = getLocalNode()->acquireSendToken( toNode );
    }
    LBASSERT( image->getPixelViewport().isValid( ));

    co::ObjectOCommand command( co::Connections( 1, connection ),
                                fabric::CMD_NODE_FRAMEDATA_TRANSMIT,
                                co::COMMANDTYPE_OBJECT, nodeID,
                                EQ_INSTANCE_ALL );
    command << frameDataVersion << image->getPixelViewport() << image->getZoom()
            << commandBuffers << frameNumber << image->getAlphaUsage();
    command.sendHeader( imageDataSize );

#ifndef NDEBUG
    size_t sentBytes = 0;
#endif

    for( uint32_t j=0; j < pixelDatas.size(); ++j )
    {
#ifndef NDEBUG
        sentBytes += sizeof( FrameData::ImageHeader );
#endif
        const PixelData* data = pixelDatas[j];
        const FrameData::ImageHeader header =
              { data->internalFormat, data->externalFormat,
                data->pixelSize, data->pvp,
                useCompression ? data->compressorName : EQ_COMPRESSOR_NONE,
                data->compressorFlags,
                data->isCompressed ? uint32_t( data->compressedSize.size()) : 1,
                qualities[ j ] };

        connection->send( &header, sizeof( header ), true );

        if( data->isCompressed )
        {
            for( uint32_t k = 0 ; k < data->compressedSize.size(); ++k )
            {
                const uint64_t dataSize = data->compressedSize[k];
                connection->send( &dataSize, sizeof( dataSize ), true );
                if( dataSize > 0 )
                    connection->send( data->compressedData[k],
                                      dataSize, true );
#ifndef NDEBUG
                sentBytes += sizeof( dataSize ) + dataSize;
#endif
            }
        }
        else
        {
            const uint64_t dataSize = data->pvp.getArea() *
                data->pixelSize;
            connection->send( &dataSize, sizeof( dataSize ), true );
            connection->send( data->pixels, dataSize, true );
#ifndef NDEBUG
            sentBytes += sizeof( dataSize ) + dataSize;
#endif
        }
    }
#ifndef NDEBUG
    LBASSERTINFO( sentBytes == imageDataSize,
        sentBytes << " != " << imageDataSize );
#endif

    getLocalNode()->releaseSendToken( token );
}

void Channel::_setReady( const bool async, detail::RBStat* stat )
{
    const Frames& frames = getOutputFrames();
    for( FramesCIter i = frames.begin(); i != frames.end(); ++i )
    {
        Frame* frame = *i;
        const Eye eye = getEye();
        const std::vector< uint128_t >& nodes = frame->getInputNodes( eye );
        const std::vector< uint128_t >& netNodes = frame->getInputNetNodes(eye);

        if( async )
            _asyncSetReady( frame->getFrameData(), stat, nodes, netNodes );
        else
            _setReady( frame->getFrameData(), stat, nodes, netNodes );
    }
}

void Channel::_asyncSetReady( const FrameDataPtr frame, detail::RBStat* stat,
                              const std::vector< uint128_t >& nodes,
                              const std::vector< uint128_t >& netNodes )
{
    LBASSERT( stat->event.event.statistic.frameNumber > 0 );

    stat->event.event.statistic.type = Statistic::CHANNEL_ASYNC_READBACK;

    _refFrame( stat->event.event.statistic.frameNumber );
    stat->ref( 0 );

    send( getLocalNode(), fabric::CMD_CHANNEL_FRAME_SET_READY )
            << co::ObjectVersion( frame ) << stat << nodes << netNodes;
}

void Channel::_setReady( FrameDataPtr frame, detail::RBStat* stat,
                         const std::vector< uint128_t >& nodes,
                         const std::vector< uint128_t >& netNodes )
{
    LBLOG( LOG_TASKS|LOG_ASSEMBLY ) << "Set ready " << co::ObjectVersion(frame)
                                    << std::endl;
    frame->setReady();

    const uint32_t frameNumber = stat->event.event.statistic.frameNumber;
    _refFrame( frameNumber );

    send( getLocalNode(), fabric::CMD_CHANNEL_FRAME_SET_READY_NODE )
            << co::ObjectVersion( frame ) << nodes << netNodes << frameNumber;

    const DrawableConfig& dc = getDrawableConfig();
    const size_t colorBytes = ( 3 * dc.colorBits + dc.alphaBits ) / 8;

    {
        lunchbox::ScopedFastWrite mutex( stat->lock );
        const Images& images = frame->getImages();
        for( ImagesCIter i = images.begin(); i != images.end(); ++i )
        {
            const Image* image = *i;
            if( image->hasPixelData( Frame::BUFFER_COLOR ))
            {
                stat->uncompressed +=
                    colorBytes * image->getPixelViewport().getArea();
                stat->compressed +=
                    image->getPixelDataSize( Frame::BUFFER_COLOR );
                stat->event.event.statistic.plugins[0] =
                                image->getDownloaderName( Frame::BUFFER_COLOR );
            }
            if( image->hasPixelData( Frame::BUFFER_DEPTH ))
            {
                stat->uncompressed += 4 * image->getPixelViewport().getArea();
                stat->compressed +=image->getPixelDataSize(Frame::BUFFER_DEPTH);
                stat->event.event.statistic.plugins[1] =
                                image->getDownloaderName( Frame::BUFFER_DEPTH );
            }
        }
    }
}

void Channel::_deleteTransferContext()
{
    if( !getPipe()->hasTransferThread( ))
        return;

    co::LocalNodePtr localNode = getLocalNode();
    const uint32_t requestID = localNode->registerRequest();
    send( localNode, fabric::CMD_CHANNEL_DELETE_TRANSFER_CONTEXT ) << requestID;
    localNode->waitRequest( requestID );
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
bool Channel::_cmdConfigInit( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    LBLOG( LOG_INIT ) << "TASK channel config init " << command << std::endl;

    const Config* config = getConfig();
    changeLatency( config->getLatency( ));

    setError( ERROR_NONE );

    bool result = false;
    const Window* window = getWindow();
    if( window->isRunning( ))
    {
        _impl->state = STATE_INITIALIZING;

        const PixelViewport& pvp = getPixelViewport();
        LBASSERT( pvp.hasArea( ));
        _impl->initialSize.x() = pvp.w;
        _impl->initialSize.y() = pvp.h;
        _impl->finishedFrame = window->getCurrentFrame();

        result = configInit( command.get< uint128_t >( ));

        if( result )
        {
            _initDrawableConfig();
            _impl->state = STATE_RUNNING;
        }
    }
    else
        setError( ERROR_CHANNEL_WINDOW_NOTRUNNING );

    LBLOG( LOG_INIT ) << "TASK channel config init reply " << result
                      << std::endl;
    commit();
    send( command.getNode(), fabric::CMD_CHANNEL_CONFIG_INIT_REPLY ) << result;
    return true;
}

bool Channel::_cmdConfigExit( co::ICommand& cmd )
{
    LBLOG( LOG_INIT ) << "Exit channel " << co::ObjectICommand( cmd )
                      << std::endl;

    _deleteTransferContext();

    if( _impl->state != STATE_STOPPED )
        _impl->state = configExit() ? STATE_STOPPED : STATE_FAILED;

    getWindow()->send( getLocalNode(),
                       fabric::CMD_WINDOW_DESTROY_CHANNEL ) << getID();
    return true;
}

bool Channel::_cmdFrameStart( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    RenderContext context = command.get< RenderContext >();
    const uint128_t version = command.get< uint128_t >();
    const uint32_t frameNumber = command.get< uint32_t >();

    LBVERB << "handle channel frame start " << command << " " << context
           << " frame " << frameNumber << std::endl;

    //_grabFrame( frameNumber ); single-threaded
    sync( version );

    overrideContext( context );
    bindFrameBuffer();
    frameStart( context.frameID, frameNumber );

    const size_t index = frameNumber % _impl->statistics->size();
    detail::Channel::FrameStatistics& statistic = _impl->statistics.data[index];
    LBASSERTINFO( statistic.used == 0,
                  "Frame " << frameNumber << " used " <<statistic.used);
    LBASSERT( statistic.data.empty( ));
    statistic.used = 1;

    resetRenderContext();
    return true;
}

bool Channel::_cmdFrameFinish( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    RenderContext context = command.get< RenderContext >();
    const uint32_t frameNumber = command.get< uint32_t >();

    LBLOG( LOG_TASKS ) << "TASK frame finish " << getName() <<  " " << command
                       << " " << context << std::endl;

    overrideContext( context );
    frameFinish( context.frameID, frameNumber );
    resetRenderContext();

    _unrefFrame( frameNumber );
    return true;
}

bool Channel::_cmdFrameClear( co::ICommand& cmd )
{
    LBASSERT( _impl->state == STATE_RUNNING );

    co::ObjectICommand command( cmd );
    RenderContext context  = command.get< RenderContext >();

    LBLOG( LOG_TASKS ) << "TASK clear " << getName() <<  " " << command
                       << " " << context << std::endl;

    _setRenderContext( context );
    ChannelStatistics event( Statistic::CHANNEL_CLEAR, this );
    frameClear( context.frameID );
    resetRenderContext();

    return true;
}

bool Channel::_cmdFrameDraw( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    RenderContext context  = command.get< RenderContext >();
    const bool finish = command.get< bool >();

    LBLOG( LOG_TASKS ) << "TASK draw " << getName() <<  " " << command
                       << " " << context << std::endl;

    _setRenderContext( context );
    const uint32_t frameNumber = getCurrentFrame();
    ChannelStatistics event( Statistic::CHANNEL_DRAW, this, frameNumber,
                             finish ? NICEST : AUTO );

    frameDraw( context.frameID );
    // Update ROI for server equalizers
    if( !getRegion().isValid( ))
        declareRegion( getPixelViewport( ));
    const size_t index = frameNumber % _impl->statistics->size();
    _impl->statistics.data[ index ].region = getRegion() / getPixelViewport();

    resetRenderContext();

    return true;
}

bool Channel::_cmdFrameDrawFinish( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    const uint128_t frameID = command.get< uint128_t >();
    const uint32_t frameNumber = command.get< uint32_t >();

    LBLOG( LOG_TASKS ) << "TASK draw finish " << getName() <<  " " << command
                       << " frame " << frameNumber << " id " << frameID
                       << std::endl;

    ChannelStatistics event( Statistic::CHANNEL_DRAW_FINISH, this );
    frameDrawFinish( frameID, frameNumber );

    return true;
}

bool Channel::_cmdFrameAssemble( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    RenderContext context = command.get< RenderContext >();
    const co::ObjectVersions frames = command.get< co::ObjectVersions >();

    LBLOG( LOG_TASKS | LOG_ASSEMBLY )
        << "TASK assemble " << getName() <<  " " << command << " " << context
        << " nFrames " << frames.size() << std::endl;

    _setRenderContext( context );
    ChannelStatistics event( Statistic::CHANNEL_ASSEMBLE, this );
    for( size_t i=0; i < frames.size(); ++i )
    {
        Pipe*  pipe  = getPipe();
        Frame* frame = pipe->getFrame( frames[i], getEye(), false );
        _impl->inputFrames.push_back( frame );
        LBLOG( LOG_ASSEMBLY ) << *frame << " " << *frame->getFrameData()
                              << std::endl;
    }

    frameAssemble( context.frameID );

    _impl->inputFrames.clear();
    resetRenderContext();
    return true;
}

bool Channel::_cmdFrameReadback( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    RenderContext context = command.get< RenderContext >();
    const co::ObjectVersions frames = command.get< co::ObjectVersions >();
    LBLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK readback " << getName() <<  " "
                                      << command << " " << context<< " nFrames "
                                      << frames.size() << std::endl;

    _setRenderContext( context );
    _frameReadback( context.frameID, frames );
    resetRenderContext();
    return true;
}

bool Channel::_cmdFinishReadback( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    LBLOG( LOG_TASKS|LOG_ASSEMBLY ) << "Finish readback " << command
                                    << std::endl;

    const co::ObjectVersion frameData = command.get< co::ObjectVersion >();
    const uint64_t imageIndex = command.get< uint64_t >();
    const uint32_t frameNumber = command.get< uint32_t >();
    const uint32_t taskID = command.get< uint32_t >();
    const std::vector< uint128_t >& nodes =
                                      command.get< std::vector< uint128_t > >();
    const std::vector< uint128_t >& netNodes =
                                      command.get< std::vector< uint128_t > >();

    getWindow()->makeCurrentTransfer();
    _finishReadback( frameData, imageIndex, frameNumber, taskID, nodes,
                     netNodes );
    _unrefFrame( frameNumber );
    return true;
}

bool Channel::_cmdFrameSetReady( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    const co::ObjectVersion frameDataVersion = command.get<co::ObjectVersion>();
    detail::RBStat* stat = command.get< detail::RBStat* >();
    const std::vector< uint128_t >& nodes =
            command.get< std::vector< uint128_t > >();
    const std::vector< uint128_t >& netNodes =
            command.get< std::vector< uint128_t > >();

    LBASSERT( stat->event.event.statistic.frameNumber > 0 );

    FrameDataPtr frameData = getNode()->getFrameData( frameDataVersion );
    _setReady( frameData, stat, nodes, netNodes );

    const uint32_t frame = stat->event.event.statistic.frameNumber;
    stat->unref( 0 );
    _unrefFrame( frame );
    return true;
}

bool Channel::_cmdFrameTransmitImage( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    const co::ObjectVersion frameData = command.get< co::ObjectVersion >();
    const uint128_t nodeID = command.get< uint128_t >();
    const uint128_t netNodeID = command.get< uint128_t >();
    const uint64_t imageIndex = command.get< uint64_t >();
    const uint32_t frameNumber = command.get< uint32_t >();
    const uint32_t taskID = command.get< uint32_t >();

    LBLOG( LOG_TASKS|LOG_ASSEMBLY ) << "Transmit " << command << " frame data "
                                    << frameData << " receiver " << nodeID
                                    << " on " << netNodeID << std::endl;

    _transmitImage( frameData, nodeID, netNodeID, imageIndex, frameNumber,
                    taskID );
    _unrefFrame( frameNumber );
    return true;
}

bool Channel::_cmdFrameSetReadyNode( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    const co::ObjectVersion& frameDataVersion = command.get< co::ObjectVersion >();
    const std::vector< uint128_t >& nodes =
            command.get< std::vector< uint128_t > >();
    const std::vector< uint128_t >& netNodes =
            command.get< std::vector< uint128_t > >();
    const uint32_t frameNumber = command.get< uint32_t >();

    co::LocalNodePtr localNode = getLocalNode();
    const FrameDataPtr frameData = getNode()->getFrameData( frameDataVersion );

    std::vector< uint128_t >::const_iterator j = netNodes.begin();
    for( std::vector< uint128_t >::const_iterator i = nodes.begin();
         i != nodes.end(); ++i, ++j )
    {
        co::NodePtr toNode = localNode->connect( *j );
        co::ObjectOCommand( co::Connections( 1, toNode->getConnection( )),
                            fabric::CMD_NODE_FRAMEDATA_READY,
                            co::COMMANDTYPE_OBJECT, *i, EQ_INSTANCE_ALL )
            << frameDataVersion << frameData->_data;
    }

    _unrefFrame( frameNumber );
    return true;
}

bool Channel::_cmdFrameViewStart( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    RenderContext context = command.get< RenderContext >();

    LBLOG( LOG_TASKS ) << "TASK view start " << getName() <<  " " << command
                       << " " << context << std::endl;

    _setRenderContext( context );
    frameViewStart( context.frameID );
    resetRenderContext();

    return true;
}

bool Channel::_cmdFrameViewFinish( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    RenderContext context = command.get< RenderContext >();

    LBLOG( LOG_TASKS ) << "TASK view finish " << getName() <<  " " << command
                       << " " << context << std::endl;

    _setRenderContext( context );
    ChannelStatistics event( Statistic::CHANNEL_VIEW_FINISH, this );
    frameViewFinish( context.frameID );
    resetRenderContext();

    return true;
}

bool Channel::_cmdStopFrame( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    LBLOG( LOG_TASKS ) << "TASK channel stop frame " << getName() <<  " "
                       << command << std::endl;

    notifyStopFrame( command.get< uint32_t >( ));
    return true;
}

bool Channel::_cmdFrameTiles( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    RenderContext context = command.get< RenderContext >();
    const bool isLocal = command.get< bool >();
    const UUID& queueID = command.get< UUID >();
    const uint32_t tasks = command.get< uint32_t >();
    const co::ObjectVersions frames = command.get< co::ObjectVersions >();

    LBLOG( LOG_TASKS ) << "TASK channel frame tiles " << getName() <<  " "
                       << command << " " << context << std::endl;

    _frameTiles( context, isLocal, queueID, tasks, frames );
    return true;
}

bool Channel::_cmdDeleteTransferContext( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    LBLOG( LOG_INIT ) << "Delete transfer context " << command << std::endl;

    getWindow()->deleteTransferSystemWindow();
    getLocalNode()->serveRequest( command.get< uint32_t >( ));
    return true;
}

}

namespace lunchbox
{
template<> inline void byteswap( eq::detail::RBStat*& value ) { /*NOP*/ }
}

#include "../fabric/channel.ipp"
template class eq::fabric::Channel< eq::Window, eq::Channel >;

/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                                                 const eq::Super& );
/** @endcond */
