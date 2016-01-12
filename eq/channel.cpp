/* Copyright (c) 2005-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Julio Delgado Mangas <julio.delgadomangas@epfl.ch>
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

// must be included before any header defining Bool
#ifdef EQ_QT_USED
#  include "qt/window.h"
#  include <QThread>
#endif

#include "channelStatistics.h"
#include "client.h"
#include "compositor.h"
#include "config.h"
#ifndef EQ_2_0_API
#  include "configEvent.h"
#endif
#include "detail/fileFrameWriter.h"
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
#include "view.h"
#include "window.h"

#include <eq/util/accum.h>
#include <eq/util/objectManager.h>
#include <eq/fabric/commands.h>
#include <eq/fabric/frameData.h>
#include <eq/fabric/task.h>
#include <eq/fabric/tile.h>

#include <co/connectionDescription.h>
#include <co/exception.h>
#include <co/objectICommand.h>
#include <co/queueSlave.h>
#include <co/sendToken.h>
#include <lunchbox/rng.h>
#include <lunchbox/scopedMutex.h>
#include <pression/plugins/compressor.h>

#ifdef EQUALIZER_USE_GLSTATS
#  include "detail/statsRenderer.h"
#  include <GLStats/GLStats.h>
#endif

#include <bitset>
#include <set>

#include "detail/channel.ipp"

#ifdef EQUALIZER_USE_DEFLECT
#  include "deflect/proxy.h"
#endif

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

void Channel::attach( const uint128_t& id, const uint32_t instanceID )
{
    Super::attach( id, instanceID );
    co::CommandQueue* queue = getPipeThreadQueue();
    co::CommandQueue* commandQ = getCommandThreadQueue();
    co::CommandQueue* tmitQ = getNode()->getTransmitterQueue();
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
    registerCommand( fabric::CMD_CHANNEL_DELETE_TRANSFER_WINDOW,
                     CmdFunc( this,&Channel::_cmdDeleteTransferWindow ),
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

util::ObjectManager& Channel::getObjectManager()
{
    Window* window = getWindow();
    LBASSERT( window );
    return window->getObjectManager();
}

const DrawableConfig& Channel::getDrawableConfig() const
{
    const Window* window = getWindow();
    LBASSERT( window );
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
#ifdef EQUALIZER_USE_DEFLECT
    delete _impl->_deflectProxy;
    _impl->_deflectProxy = 0;
#endif
    _impl->framebufferImage.flush();
    return true;
}

bool Channel::configInit( const uint128_t& )
{
#ifdef EQUALIZER_USE_DEFLECT
    if( getView() &&
        !getView()->getSAttribute( View::SATTR_DISPLAYCLUSTER ).empty( ))
    {
        LBASSERT( !_impl->_deflectProxy );
        _impl->_deflectProxy = new deflect::Proxy( this );
    }
#endif
    return true;
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
    LBASSERT( event.originator != 0 );
    event.resize.x   = newPVP.x;
    event.resize.y   = newPVP.y;
    event.resize.w   = newPVP.w;
    event.resize.h   = newPVP.h;

    processEvent( event );
}

void Channel::notifyStopFrame( const uint32_t )
{}

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
bool Channel::waitFrameFinished( const uint32_t frame,
                                 const uint32_t timeout ) const
{
    return _impl->finishedFrame.timedWaitGE( frame, timeout );
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
        EQ_GL_CALL( glClearColor( color.r()/255.f, color.g()/255.f,
                                  color.b()/255.f, 0.f ));
    }
#endif // NDEBUG

    EQ_GL_CALL( glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ));
}

void Channel::frameDraw( const uint128_t& )
{
    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));

    const bool coreProfile = getWindow()->getIAttribute(
                WindowSettings::IATTR_HINT_CORE_PROFILE ) == ON;
    if( coreProfile )
        return;

    EQ_GL_CALL( glMatrixMode( GL_PROJECTION ));
    EQ_GL_CALL( glLoadIdentity( ));
    EQ_GL_CALL( applyFrustum( ));

    EQ_GL_CALL( glMatrixMode( GL_MODELVIEW ));
    EQ_GL_CALL( glLoadIdentity( ));
    EQ_GL_CALL( applyHeadTransform( ));
}

void Channel::frameAssemble( const uint128_t&, const Frames& frames )
{
    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));
    EQ_GL_CALL( setupAssemblyState( ));
    try
    {
        Compositor::assembleFrames( frames, this, 0 );
    }
    catch( const co::Exception& e )
    {
        LBWARN << e.what() << std::endl;
    }
    EQ_GL_CALL( resetAssemblyState( ));
}

void Channel::frameReadback( const uint128_t&, const Frames& frames )
{
    const PixelViewport& region = getRegion();
    if( !region.hasArea( ))
        return;

    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));
    EQ_GL_CALL( setupAssemblyState( ));

    util::ObjectManager&  glObjects   = getObjectManager();
    const DrawableConfig& drawable    = getDrawableConfig();
    const PixelViewports& regions     = getRegions();

    for( Frame* frame : frames )
        frame->startReadback( glObjects, drawable, regions );

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

void Channel::frameViewFinish( const uint128_t& frameID )
{
    frameDrawOverlay( frameID );
    _impl->frameViewFinish( *this );
}

void Channel::frameDrawOverlay( const uint128_t& )
{
    applyOverlayState();

#ifdef EQUALIZER_USE_DEFLECT
    if( _impl->_deflectProxy && _impl->_deflectProxy->isRunning( ))
    {
        const eq::PixelViewport& pvp = getPixelViewport();
        const eq::Viewport& vp = getViewport();

        const float width = pvp.w / vp.w;
        const float height = pvp.h / vp.h;
        const float xOffset = vp.x * width;

        glRasterPos3f( 10.f - xOffset, height - 30.f, 0.99f );
        getWindow()->getMediumFont()->draw( _impl->_deflectProxy->getHelp( ));
    }
#endif

    resetOverlayState();
}

void Channel::setupAssemblyState()
{
    EQ_GL_CALL( bindFrameBuffer( ));
    const PixelViewport& pvp = getPixelViewport();
    const bool coreProfile = getWindow()->getIAttribute(
                WindowSettings::IATTR_HINT_CORE_PROFILE ) == ON;
    if( !coreProfile )
        Compositor::setupAssemblyState( pvp, glewGetContext( ));
}

void Channel::resetAssemblyState()
{
    EQ_GL_CALL( bindFrameBuffer( ));
    const bool coreProfile = getWindow()->getIAttribute(
                WindowSettings::IATTR_HINT_CORE_PROFILE ) == ON;
    if( !coreProfile )
        Compositor::resetAssemblyState();
}

void Channel::_overrideContext( RenderContext& context )
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

co::QueueSlave* Channel::_getQueue( const uint128_t& queueID )
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

void Channel::addResultImageListener( ResultImageListener* listener )
{
    _impl->addResultImageListener( listener );
}

void Channel::removeResultImageListener( ResultImageListener* listener )
{
    _impl->removeResultImageListener( listener );
}

std::string Channel::getDumpImageFileName() const
{
    std::stringstream name;
    name << getCurrentFrame() << ".rgb";
    return name.str();
}

//---------------------------------------------------------------------------
// apply convenience methods
//---------------------------------------------------------------------------
void Channel::applyBuffer()
{
    LB_TS_THREAD( _pipeThread );
    const Window* window = getWindow();
    if( !window->getSystemWindow()->getFrameBufferObject( ))
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

    if( _impl->_updateFrameBuffer )
    {
        window->updateFrameBuffer();
        _impl->_updateFrameBuffer = false;
    }
    window->bindFrameBuffer();
}

void Channel::bindDrawFrameBuffer()
{
    LB_TS_THREAD( _pipeThread );
    const Window* window = getWindow();
    if( !window->getSystemWindow( ))
       return;

    window->bindDrawFrameBuffer();
    _impl->_updateFrameBuffer = true;
}

void Channel::applyColorMask() const
{
    LB_TS_THREAD( _pipeThread );
    const ColorMask& colorMask = getDrawBufferMask();
    EQ_GL_CALL( glColorMask( colorMask.red, colorMask.green, colorMask.blue,
                             true ));
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

void Channel::applyOverlayState()
{
    applyBuffer();
    applyViewport();
    setupAssemblyState();

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    applyScreenFrustum();

    EQ_GL_CALL( glLogicOp( GL_XOR ));
    EQ_GL_CALL( glEnable( GL_COLOR_LOGIC_OP ));
    EQ_GL_CALL( glDisable( GL_DEPTH_TEST ));
    EQ_GL_CALL( glDisable( GL_LIGHTING ));
    EQ_GL_CALL( glCullFace( GL_BACK ));

    EQ_GL_CALL( glColor3f( 1.f, 1.f, 1.f ));
}

void Channel::resetOverlayState()
{
    EQ_GL_CALL( glDisable( GL_COLOR_LOGIC_OP ));
    EQ_GL_CALL( glEnable( GL_DEPTH_TEST ));
    EQ_GL_CALL( glEnable( GL_LIGHTING ));
    resetAssemblyState();
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
    for( const PixelViewport& pvp : _impl->regions )
        region.merge( pvp );

    return region;
}

const PixelViewports& Channel::getRegions() const
{
    return _impl->regions;
}

EventOCommand Channel::sendError( const uint32_t error )
{
    return getConfig()->sendError( Event::CHANNEL_ERROR,
                                   Error( error, getID( )));
}

bool Channel::processEvent( const Event& event )
{
    ConfigEvent configEvent;
    configEvent.data = event;

    switch( event.type )
    {
        case Event::CHANNEL_POINTER_MOTION:
        case Event::CHANNEL_POINTER_BUTTON_PRESS:
        case Event::CHANNEL_POINTER_BUTTON_RELEASE:
        case Event::CHANNEL_POINTER_WHEEL:
        case Event::STATISTIC:
        case Event::KEY_PRESS:
        case Event::KEY_RELEASE:
            break;

        case Event::CHANNEL_RESIZE:
        {
            const uint128_t& viewID = getNativeContext().view.identifier;
            if( viewID == 0 )
                return true;

            // transform to view event, which is meaningful for the config
            configEvent.data.type       = Event::VIEW_RESIZE;
            configEvent.data.originator = viewID;

            ResizeEvent& resize = configEvent.data.resize;
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
    config->sendEvent( configEvent );
    return true;
}

void Channel::drawStatistics()
{
    const PixelViewport& pvp = getPixelViewport();
    LBASSERT( pvp.hasArea( ));
    Window* window = getWindow();
    const bool coreProfile = window->getIAttribute(
                WindowSettings::IATTR_HINT_CORE_PROFILE ) == ON;
    if( !pvp.hasArea() || coreProfile )
        return;

    //----- setup
    applyOverlayState();

    EQ_GL_CALL( glDisable( GL_COLOR_LOGIC_OP ));
    EQ_GL_CALL( glEnable( GL_BLEND ));
    EQ_GL_CALL( glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ));

#ifdef EQUALIZER_USE_GLSTATS
    const util::BitmapFont* font = window->getSmallFont();
    const Config* config = getConfig();
    const GLStats::Data& data = config->getStatistics();
    detail::StatsRenderer renderer( font );
    const Viewport& vp = getViewport();
    const uint32_t width = uint32_t( pvp.w / vp.w );
    const uint32_t height = uint32_t( pvp.h / vp.h);

    renderer.setViewport( width, height );
    renderer.draw( data );
#endif

    EQ_GL_CALL( glEnable( GL_COLOR_LOGIC_OP ));
    EQ_GL_CALL( glColor3f( 1.f, 1.f, 1.f ));
    window->drawFPS();

    EQ_GL_CALL( glDisable( GL_BLEND ));
    resetOverlayState();
}

void Channel::outlineViewport()
{
    const bool coreProfile = getWindow()->getIAttribute(
                WindowSettings::IATTR_HINT_CORE_PROFILE ) == ON;
    if( coreProfile )
        return;

    applyOverlayState();

    const eq::PixelViewport& region = getRegion();
    glColor3f( .5f, .5f, .5f );
    glBegin( GL_LINE_LOOP ); {
        glVertex3f( region.x + .5f,         region.y + .5f,         0.f );
        glVertex3f( region.getXEnd() - .5f, region.y + .5f,         0.f );
        glVertex3f( region.getXEnd() - .5f, region.getYEnd() - .5f, 0.f );
        glVertex3f( region.x + .5f,         region.getYEnd() - .5f, 0.f );
    } glEnd();

    const PixelViewport& pvp = getPixelViewport();
    glColor3f( 1.0f, 1.0f, 1.0f );
    glBegin( GL_LINE_LOOP ); {
        glVertex3f( pvp.x + .5f,         pvp.y + .5f,         0.f );
        glVertex3f( pvp.getXEnd() - .5f, pvp.y + .5f,         0.f );
        glVertex3f( pvp.getXEnd() - .5f, pvp.getYEnd() - .5f, 0.f );
        glVertex3f( pvp.x + .5f,         pvp.getYEnd() - .5f, 0.f );
    } glEnd();

    resetOverlayState();
}

namespace detail
{
struct RBStat
{
    explicit RBStat( eq::Channel* channel )
        : event( Statistic::CHANNEL_READBACK, channel )
        , uncompressed( 0 )
        , compressed( 0 )
    {
        event.event.data.statistic.plugins[0] = EQ_COMPRESSOR_NONE;
        event.event.data.statistic.plugins[1] = EQ_COMPRESSOR_NONE;
        LBASSERT( event.event.data.statistic.frameNumber > 0 );
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
        {
            event.event.data.statistic.ratio = float( compressed ) /
                                               float( uncompressed );
        }
        else
            event.event.data.statistic.ratio = 1.0f;
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
                           const uint128_t& queueID, const uint32_t tasks,
                           const co::ObjectVersions& frameIDs )
{
    _overrideContext( context );

    frameTilesStart( context.frameID );

    RBStatPtr stat;
    Frames frames;
    if( tasks & fabric::TASK_READBACK )
    {
        frames = _getFrames( frameIDs, true );
        stat = new detail::RBStat( this );
    }

    int64_t startTime = getConfig()->getTime();
    int64_t clearTime = 0;
    int64_t drawTime = 0;
    int64_t readbackTime = 0;
    bool hasAsyncReadback = false;
    const uint32_t timeout = getConfig()->getTimeout();

    co::QueueSlave* queue = _getQueue( queueID );
    LBASSERT( queue );
    for( ;; )
    {
        co::ObjectICommand tileCmd = queue->pop( timeout );
        if( !tileCmd.isValid( ))
            break;

        const Tile& tile = tileCmd.read< Tile >();
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
            // Set to full region if application has declared nothing
            if( !getRegion().isValid( ))
                declareRegion( getPixelViewport( ));
        }

        if( tasks & fabric::TASK_READBACK )
        {
            const int64_t time = getConfig()->getTime();
            const size_t nFrames = frames.size();

            std::vector< size_t > nImages( nFrames, 0 );
            for( size_t i = 0; i < nFrames; ++i )
            {
                nImages[i] = frames[i]->getImages().size();
                frames[i]->getFrameData()->setPixelViewport(
                    getPixelViewport( ));
            }

            frameReadback( context.frameID, frames );
            readbackTime += getConfig()->getTime() - time;

            for( size_t i = 0; i < nFrames; ++i )
            {
                const Frame* frame = frames[i];
                const Images& images = frame->getImages();
                for( size_t j = nImages[i]; j < images.size(); ++j )
                {
                    Image* image = images[j];
                    const PixelViewport& pvp = image->getPixelViewport();
                    image->setOffset( pvp.x + tilePVP.x,
                                      pvp.y + tilePVP.y );
                }
            }

            if( _asyncFinishReadback( nImages, frames ))
                hasAsyncReadback = true;
        }
    }

    if( tasks & fabric::TASK_CLEAR )
    {
        ChannelStatistics event( Statistic::CHANNEL_CLEAR, this );
        event.event.data.statistic.startTime = startTime;
        startTime += clearTime;
        event.event.data.statistic.endTime = startTime;
    }

    if( tasks & fabric::TASK_DRAW )
    {
        ChannelStatistics event( Statistic::CHANNEL_DRAW, this );
        event.event.data.statistic.startTime = startTime;
        startTime += drawTime;
        event.event.data.statistic.endTime = startTime;
    }

    if( tasks & fabric::TASK_READBACK )
    {
        stat->event.event.data.statistic.startTime = startTime;
        startTime += readbackTime;
        stat->event.event.data.statistic.endTime = startTime;

        _setReady( hasAsyncReadback, stat.get(), frames );
    }

    frameTilesFinish( context.frameID );
    resetContext();
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

    send( getServer(), fabric::CMD_CHANNEL_FRAME_FINISH_REPLY )
            << stats.region << frameNumber << stats.data;

    stats.data.clear();
    stats.region = Viewport::FULL;
    _impl->finishedFrame = frameNumber;
}

Frames Channel::_getFrames( const co::ObjectVersions& frameIDs,
                            const bool isOutput )
{
    LB_TS_THREAD( _pipeThread );

    Frames frames;
    for( size_t i = 0; i < frameIDs.size(); ++i )
    {
        Pipe*  pipe  = getPipe();
        Frame* frame = pipe->getFrame( frameIDs[i], getEye(), isOutput );
        LBASSERTINFO( lunchbox::find( frames, frame ) == frames.end(),
                      "frame " << i << " " << frameIDs[i] );

        frames.push_back( frame );
    }

    return frames;
}

//---------------------------------------------------------------------------
// Asynchronous image readback, compression and transmission
//---------------------------------------------------------------------------
void Channel::_frameReadback( const uint128_t& frameID,
                              const co::ObjectVersions& frameIDs )
{
    LB_TS_THREAD( _pipeThread );

    RBStatPtr stat = new detail::RBStat( this );
    const Frames& frames = _getFrames( frameIDs, true );

    std::vector< size_t > nImages( frames.size(), 0 );
    for( size_t i = 0; i < frames.size(); ++i )
        nImages[i] = frames[i]->getImages().size();

    frameReadback( frameID, frames );
    LBASSERT( stat->event.event.data.statistic.frameNumber > 0 );
    const bool async = _asyncFinishReadback( nImages, frames );
    _setReady( async, stat.get(), frames );
}

bool Channel::_asyncFinishReadback( const std::vector< size_t >& imagePos,
                                    const Frames& frames )
{
    LB_TS_THREAD( _pipeThread );

    bool hasAsyncReadback = false;
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
        const co::NodeIDs& netNodes = frame->getInputNetNodes(eye);

        for( uint64_t j = imagePos[i]; j < nImages; ++j )
        {
            if( images[j]->hasAsyncReadback( )) // finish async readback
            {
                _createTransferWindow();

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
                               const co::NodeIDs& netNodes )
{
    LBLOG( LOG_TASKS|LOG_ASSEMBLY ) << "Finish readback" << std::endl;

    const Window* window = getWindow();
    const SystemWindow* transferWindow = window->getTransferWindow();
    LBASSERT( transferWindow );
    transferWindow->makeCurrent();

    FrameDataPtr frameData = getNode()->getFrameData( frameDataVersion );
    LBASSERT( frameData );

    const Images& images = frameData->getImages();
    LBASSERT( images.size() > imageIndex );

    Image* image = images[ imageIndex ];
    LBASSERT( image->hasAsyncReadback( ));

    const GLEWContext* glewContext = window->getTransferGlewContext();
    image->finishReadback( glewContext );
    LBASSERT( !image->hasAsyncReadback( ));

    // schedule async image tranmission
    _asyncTransmit( frameData, frameNumber, imageIndex, nodes, netNodes,
                    taskID );
}

void Channel::_asyncTransmit( FrameDataPtr frame, const uint32_t frameNumber,
                              const uint64_t image,
                              const std::vector< uint128_t >& nodes,
                              const co::NodeIDs& netNodes,
                              const uint32_t taskID )
{
    LBASSERT( nodes.size() == netNodes.size( ));
    co::NodeIDs::const_iterator j = netNodes.begin();
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
                              const co::NodeID& netNodeID,
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
    transmitEvent.event.data.statistic.task = taskID;

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
        LBWARN << "Can't connect node " << netNodeID << " to send output frame"
               << std::endl;
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
        compressEvent.event.data.statistic.task = taskID;
        compressEvent.event.data.statistic.ratio = 1.0f;
        compressEvent.event.data.statistic.plugins[0] = EQ_COMPRESSOR_NONE;
        compressEvent.event.data.statistic.plugins[1] = EQ_COMPRESSOR_NONE;

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

                if( data.compressedData.isCompressed( ))
                {
                    imageDataSize += data.compressedData.getSize() +
                        data.compressedData.chunks.size() * sizeof( uint64_t );
                    compressEvent.event.data.statistic.plugins[j] =
                        data.compressedData.compressor;
                }
                else
                    imageDataSize += sizeof( uint64_t ) +
                                     image->getPixelDataSize( buffer );

                commandBuffers |= buffer;
                rawSize += image->getPixelDataSize( buffer );
            }
        }

        if( rawSize > 0 )
            compressEvent.event.data.statistic.ratio =
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
        waitEvent.event.data.statistic.task = taskID;
        token = getLocalNode()->acquireSendToken( toNode );
    }
    LBASSERT( image->getPixelViewport().isValid( ));

    co::ObjectOCommand command( co::Connections( 1, connection ),
                                fabric::CMD_NODE_FRAMEDATA_TRANSMIT,
                                co::COMMANDTYPE_OBJECT, nodeID,
                                CO_INSTANCE_ALL );
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
        const bool isCompressed = data->compressedData.isCompressed();
        const uint32_t nChunks = isCompressed ?
            uint32_t( data->compressedData.chunks.size( )) : 1;

        const FrameData::ImageHeader header =
              { data->internalFormat, data->externalFormat,
                data->pixelSize, data->pvp,
                isCompressed ? data->compressedData.compressor :
                               EQ_COMPRESSOR_NONE,
                data->compressorFlags, nChunks, qualities[ j ] };

        connection->send( &header, sizeof( header ), true );

        if( isCompressed )
        {
            BOOST_FOREACH( const pression::CompressorChunk& chunk,
                           data->compressedData.chunks )
            {
                const uint64_t dataSize = chunk.getNumBytes();

                connection->send( &dataSize, sizeof( dataSize ), true );
                if( dataSize > 0 )
                    connection->send( chunk.data, dataSize, true );
#ifndef NDEBUG
                sentBytes += sizeof( dataSize ) + dataSize;
#endif
            }
        }
        else
        {
            const uint64_t dataSize = data->pvp.getArea() * data->pixelSize;
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
}

void Channel::_setReady( const bool async, detail::RBStat* stat,
                         const Frames& frames )
{
    for( FramesCIter i = frames.begin(); i != frames.end(); ++i )
    {
        Frame* frame = *i;
        const Eye eye = getEye();
        const std::vector< uint128_t >& nodes = frame->getInputNodes( eye );
        const co::NodeIDs& netNodes = frame->getInputNetNodes(eye);

        if( async )
            _asyncSetReady( frame->getFrameData(), stat, nodes, netNodes );
        else
            _setReady( frame->getFrameData(), stat, nodes, netNodes );
    }
}

void Channel::_asyncSetReady( const FrameDataPtr frame, detail::RBStat* stat,
                              const std::vector< uint128_t >& nodes,
                              const co::NodeIDs& netNodes )
{
    LBASSERT( stat->event.event.data.statistic.frameNumber > 0 );

    stat->event.event.data.statistic.type = Statistic::CHANNEL_ASYNC_READBACK;

    _refFrame( stat->event.event.data.statistic.frameNumber );
    stat->ref( 0 );

    send( getLocalNode(), fabric::CMD_CHANNEL_FRAME_SET_READY )
            << co::ObjectVersion( frame ) << stat << nodes << netNodes;
}

void Channel::_setReady( FrameDataPtr frame, detail::RBStat* stat,
                         const std::vector< uint128_t >& nodes,
                         const co::NodeIDs& netNodes )
{
    LBLOG( LOG_TASKS|LOG_ASSEMBLY ) << "Set ready " << co::ObjectVersion(frame)
                                    << std::endl;
    frame->setReady();

    const uint32_t frameNumber = stat->event.event.data.statistic.frameNumber;
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
                stat->event.event.data.statistic.plugins[0] =
                                image->getDownloaderName( Frame::BUFFER_COLOR );
            }
            if( image->hasPixelData( Frame::BUFFER_DEPTH ))
            {
                stat->uncompressed += 4 * image->getPixelViewport().getArea();
                stat->compressed +=image->getPixelDataSize(Frame::BUFFER_DEPTH);
                stat->event.event.data.statistic.plugins[1] =
                                image->getDownloaderName( Frame::BUFFER_DEPTH );
            }
        }
    }
}

void Channel::_createTransferWindow()
{
    if( getWindow()->getTransferWindow( ))
        return;

    Pipe* pipe = getPipe();
    Window* window = getWindow();
    LBCHECK( pipe->startTransferThread( ));
    LBCHECK( window->createTransferWindow( ));

#ifdef EQ_QT_USED
    // transfer window creation must happen in pipe thread (#177), but the
    // context is used in the transfer thread and Qt requires moving the object
    // to that thread.
    qt::Window* qtWindow =
        dynamic_cast< qt::Window* >( window->getTransferWindow( ));
    QThread* qThread = pipe->getTransferQThread();

    if( qtWindow && qThread )
        qtWindow->moveContextToThread( qThread );
#endif
}

void Channel::_deleteTransferWindow()
{
    if( !getPipe()->hasTransferThread( ))
        return;

    // #510: Need to schedule deletion in transfer thread since qt::Window was
    // potentially moved to this thread
    co::LocalNodePtr localNode = getLocalNode();
    const lunchbox::Request< void >& request =
        localNode->registerRequest< void >();
    send( localNode, fabric::CMD_CHANNEL_DELETE_TRANSFER_WINDOW ) << request;
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

        result = configInit( command.read< uint128_t >( ));

        if( result )
            _impl->state = STATE_RUNNING;
    }
    else
        sendError( ERROR_CHANNEL_WINDOW_NOTRUNNING );

    LBLOG( LOG_INIT ) << "TASK channel config init reply " << result
                      << std::endl;
    commit();
    send( command.getRemoteNode(), fabric::CMD_CHANNEL_CONFIG_INIT_REPLY )
            << result;
    return true;
}

bool Channel::_cmdConfigExit( co::ICommand& cmd )
{
    LBLOG( LOG_INIT ) << "Exit channel " << co::ObjectICommand( cmd )
                      << std::endl;

    if( _impl->state != STATE_STOPPED )
        _impl->state = configExit() ? STATE_STOPPED : STATE_FAILED;

    _deleteTransferWindow();
    getWindow()->send( getLocalNode(),
                       fabric::CMD_WINDOW_DESTROY_CHANNEL ) << getID();
    return true;
}

bool Channel::_cmdFrameStart( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    RenderContext context = command.read< RenderContext >();
    const uint128_t& version = command.read< uint128_t >();
    const uint32_t frameNumber = command.read< uint32_t >();

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

    resetContext();
    return true;
}

bool Channel::_cmdFrameFinish( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    RenderContext context = command.read< RenderContext >();
    const uint32_t frameNumber = command.read< uint32_t >();

    LBLOG( LOG_TASKS ) << "TASK frame finish " << getName() <<  " " << command
                       << " " << context << std::endl;

    overrideContext( context );
    frameFinish( context.frameID, frameNumber );
    resetContext();

    _unrefFrame( frameNumber );
    return true;
}

bool Channel::_cmdFrameClear( co::ICommand& cmd )
{
    LBASSERT( _impl->state == STATE_RUNNING );

    co::ObjectICommand command( cmd );
    RenderContext context  = command.read< RenderContext >();

    LBLOG( LOG_TASKS ) << "TASK clear " << getName() <<  " " << command
                       << " " << context << std::endl;

    bindDrawFrameBuffer();
    _overrideContext( context );
    ChannelStatistics event( Statistic::CHANNEL_CLEAR, this );
    frameClear( context.frameID );
    resetContext();
    bindFrameBuffer();

    return true;
}

bool Channel::_cmdFrameDraw( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    RenderContext context  = command.read< RenderContext >();
    const bool finish = command.read< bool >();

    LBLOG( LOG_TASKS ) << "TASK draw " << getName() <<  " " << command
                       << " " << context << std::endl;

    bindDrawFrameBuffer();
    _overrideContext( context );
    const uint32_t frameNumber = getCurrentFrame();
    ChannelStatistics event( Statistic::CHANNEL_DRAW, this, frameNumber,
                             finish ? NICEST : AUTO );

    frameDraw( context.frameID );
    // Set to full region if application has declared nothing
    if( !getRegion().isValid( ))
        declareRegion( getPixelViewport( ));
    const size_t index = frameNumber % _impl->statistics->size();
    _impl->statistics.data[ index ].region = getRegion() / getPixelViewport();

    resetContext();
    bindFrameBuffer();

    return true;
}

bool Channel::_cmdFrameDrawFinish( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    const uint128_t& frameID = command.read< uint128_t >();
    const uint32_t frameNumber = command.read< uint32_t >();

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
    RenderContext context = command.read< RenderContext >();
    const co::ObjectVersions& frameIDs = command.read< co::ObjectVersions >();

    LBLOG( LOG_TASKS | LOG_ASSEMBLY )
        << "TASK assemble " << getName() <<  " " << command << " " << context
        << " nFrames " << frameIDs.size() << std::endl;

    _overrideContext( context );

    ChannelStatistics event( Statistic::CHANNEL_ASSEMBLE, this );
    const Frames& frames = _getFrames( frameIDs, false );
    frameAssemble( context.frameID, frames );

    resetContext();
    return true;
}

bool Channel::_cmdFrameReadback( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    RenderContext context = command.read< RenderContext >();
    const co::ObjectVersions& frames = command.read< co::ObjectVersions >();
    LBLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK readback " << getName() <<  " "
                                      << command << " " << context<< " nFrames "
                                      << frames.size() << std::endl;

    _overrideContext( context );
    _frameReadback( context.frameID, frames );
    resetContext();
    return true;
}

bool Channel::_cmdFinishReadback( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    LBLOG( LOG_TASKS|LOG_ASSEMBLY ) << "Finish readback " << command
                                    << std::endl;

    const co::ObjectVersion& frameData = command.read< co::ObjectVersion >();
    const uint64_t imageIndex = command.read< uint64_t >();
    const uint32_t frameNumber = command.read< uint32_t >();
    const uint32_t taskID = command.read< uint32_t >();
    const std::vector< uint128_t >& nodes =
            command.read< std::vector< uint128_t > >();
    const co::NodeIDs& netNodes = command.read< co::NodeIDs >();

    _finishReadback( frameData, imageIndex, frameNumber, taskID, nodes,
                     netNodes );
    _unrefFrame( frameNumber );
    return true;
}

bool Channel::_cmdFrameSetReady( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    const co::ObjectVersion frameDataVersion =
            command.read< co::ObjectVersion >();
    detail::RBStat* stat = command.read< detail::RBStat* >();
    const std::vector< uint128_t >& nodes =
            command.read< std::vector< uint128_t > >();
    const co::NodeIDs& netNodes = command.read< co::NodeIDs >();

    LBASSERT( stat->event.event.data.statistic.frameNumber > 0 );

    FrameDataPtr frameData = getNode()->getFrameData( frameDataVersion );
    _setReady( frameData, stat, nodes, netNodes );

    const uint32_t frame = stat->event.event.data.statistic.frameNumber;
    stat->unref( 0 );
    _unrefFrame( frame );
    return true;
}

bool Channel::_cmdFrameTransmitImage( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    const co::ObjectVersion& frameData = command.read< co::ObjectVersion >();
    const uint128_t& nodeID = command.read< uint128_t >();
    const co::NodeID& netNodeID = command.read< co::NodeID >();
    const uint64_t imageIndex = command.read< uint64_t >();
    const uint32_t frameNumber = command.read< uint32_t >();
    const uint32_t taskID = command.read< uint32_t >();

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

    const co::ObjectVersion& frameDataVersion =
            command.read< co::ObjectVersion >();
    const std::vector< uint128_t >& nodes =
            command.read< std::vector< uint128_t > >();
    const co::NodeIDs& netNodes = command.read< co::NodeIDs >();
    const uint32_t frameNumber = command.read< uint32_t >();

    co::LocalNodePtr localNode = getLocalNode();
    const FrameDataPtr frameData = getNode()->getFrameData( frameDataVersion );

    co::NodeIDs::const_iterator j = netNodes.begin();
    for( std::vector< uint128_t >::const_iterator i = nodes.begin();
         i != nodes.end(); ++i, ++j )
    {
        co::NodePtr toNode = localNode->connect( *j );
        if( !toNode )
        {
            LBERROR << "Can't connect to " << *j << " to signal ready of frame "
                    << frameNumber << std::endl;
            continue;
        }
        co::ObjectOCommand os( co::Connections( 1, toNode->getConnection( )),
                            fabric::CMD_NODE_FRAMEDATA_READY,
                               co::COMMANDTYPE_OBJECT, *i, CO_INSTANCE_ALL );
        os << frameDataVersion;
        frameData->serialize( os );
    }

    _unrefFrame( frameNumber );
    return true;
}

bool Channel::_cmdFrameViewStart( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    RenderContext context = command.read< RenderContext >();

    LBLOG( LOG_TASKS ) << "TASK view start " << getName() <<  " " << command
                       << " " << context << std::endl;

    _overrideContext( context );
    frameViewStart( context.frameID );
    resetContext();

    return true;
}

bool Channel::_cmdFrameViewFinish( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    RenderContext context = command.read< RenderContext >();

    LBLOG( LOG_TASKS ) << "TASK view finish " << getName() <<  " " << command
                       << " " << context << std::endl;

    _overrideContext( context );
    {
        ChannelStatistics event( Statistic::CHANNEL_VIEW_FINISH, this );
        frameViewFinish( context.frameID );
    }
    resetContext();

    return true;
}

bool Channel::_cmdStopFrame( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    LBLOG( LOG_TASKS ) << "TASK channel stop frame " << getName() <<  " "
                       << command << std::endl;

    notifyStopFrame( command.read< uint32_t >( ));
    return true;
}

bool Channel::_cmdFrameTiles( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    RenderContext context = command.read< RenderContext >();
    const bool isLocal = command.read< bool >();
    const uint128_t& queueID = command.read< uint128_t >();
    const uint32_t tasks = command.read< uint32_t >();
    const co::ObjectVersions& frames = command.read< co::ObjectVersions >();

    LBLOG( LOG_TASKS ) << "TASK channel frame tiles " << getName() <<  " "
                       << command << " " << context << std::endl;

    _frameTiles( context, isLocal, queueID, tasks, frames );
    return true;
}

bool Channel::_cmdDeleteTransferWindow( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    LBLOG( LOG_INIT ) << "Delete transfer window " << command << std::endl;

    getWindow()->deleteTransferWindow();
    getLocalNode()->serveRequest( command.read< uint32_t >( ));
    return true;
}

}

namespace lunchbox
{
template<> inline void byteswap( eq::detail::RBStat*& ) { /*NOP*/ }
}

#include <eq/fabric/channel.ipp>
template class eq::fabric::Channel< eq::Window, eq::Channel >;

/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                                                 const eq::Super& );
/** @endcond */
