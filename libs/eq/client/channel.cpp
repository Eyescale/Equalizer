
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com> 
 *                    2011, Cedric Stalder <cedric.stalder@gmail.com>
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

#include "channelPackets.h"
#include "channelStatistics.h"
#include "client.h"
#include "compositor.h"
#include "config.h"
#include "configEvent.h"
#include "error.h"
#include "frame.h"
#include "frameData.h"
#include "global.h"
#include "image.h"
#include "jitter.h"
#include "log.h"
#include "node.h"
#include "nodeFactory.h"
#include "nodePackets.h"
#include "pipe.h"
#include "server.h"
#include "systemWindow.h"
#include "windowPackets.h"

#include <eq/util/accum.h>
#include <eq/util/frameBufferObject.h>
#include <eq/util/objectManager.h>
#include <eq/fabric/commands.h>
#include <eq/fabric/task.h>

#include <co/command.h>
#include <co/connectionDescription.h>
#include <co/exception.h>
#include <co/queueSlave.h>
#include <co/base/rng.h>
#include <co/base/scopedMutex.h>

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

void Channel::attach( const co::base::UUID& id, const uint32_t instanceID )
{
    Super::attach( id, instanceID );
    co::CommandQueue* queue = getPipeThreadQueue();
    co::CommandQueue* commandQ = getCommandThreadQueue();
    co::CommandQueue* tmitQ = &getNode()->transmitter.getQueue();
    co::CommandQueue* asyncRBQ = getPipe()->getAsyncRBThreadQueue();

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
                     CmdFunc( this, &Channel::_cmdFinishReadback ), asyncRBQ );
    registerCommand( fabric::CMD_CHANNEL_FRAME_SET_READY,
                     CmdFunc( this, &Channel::_cmdFrameSetReady ), asyncRBQ );
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
    EQASSERT( window );
    return ( window ? window->getPipe() : 0 );
}

const Pipe* Channel::getPipe() const
{
    const Window* window = getWindow();
    EQASSERT( window );
    return ( window ? window->getPipe() : 0 );
}

Node* Channel::getNode()
{
    Window* window = getWindow();
    EQASSERT( window );
    return ( window ? window->getNode() : 0 );
}
const Node* Channel::getNode() const
{
    const Window* window = getWindow();
    EQASSERT( window );
    return ( window ? window->getNode() : 0 );
}

Config* Channel::getConfig()
{
    Window* window = getWindow();
    EQASSERT( window );
    return ( window ? window->getConfig() : 0 );
}
const Config* Channel::getConfig() const
{
    const Window* window = getWindow();
    EQASSERT( window );
    return ( window ? window->getConfig() : 0 );
}

ServerPtr Channel::getServer()
{
    Window* window = getWindow();
    EQASSERT( window );
    return ( window ? window->getServer() : 0 );
}

Window::ObjectManager* Channel::getObjectManager()
{
    Window* window = getWindow();
    EQASSERT( window );
    return window->getObjectManager();
}

const DrawableConfig& Channel::getDrawableConfig() const
{
    const Window* window = getWindow();
    EQASSERT( window );
    if( _impl->fbo )
        return _impl->drawableConfig;

    return window->getDrawableConfig();
}

const GLEWContext* Channel::glewGetContext() const
{
    const Window* window = getWindow();
    EQASSERT( window );
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
                EQUNIMPLEMENTED;
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
    EQASSERT( event.originator != co::base::UUID::ZERO );
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
        EQASSERT( index < _impl->statistics->size( ));
        EQASSERTINFO( _impl->statistics.data[ index ].used > 0, frameNumber );

        co::base::ScopedFastWrite mutex( _impl->statistics );
        Statistics& statistics = _impl->statistics.data[ index ].data;
        statistics.push_back( event.statistic );
    }
    processEvent( event );
}

//---------------------------------------------------------------------------
// operations
//---------------------------------------------------------------------------

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
        EQWARN << e.what() << std::endl;
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
    EQ_TS_THREAD( _pipeThread );
    Pipe* pipe = getPipe();
    return pipe->getView( getContext().view );
}

const View* Channel::getView() const
{
    EQ_TS_THREAD( _pipeThread );
    const Pipe* pipe = getPipe();
    return pipe->getView( getContext().view );
}

co::QueueSlave* Channel::_getQueue( const co::ObjectVersion& queueVersion )
{
    EQ_TS_THREAD( _pipeThread );
    Pipe* pipe = getPipe();
    return pipe->getQueue( queueVersion );
}

View* Channel::getNativeView()
{
    EQ_TS_THREAD( _pipeThread );
    Pipe* pipe = getPipe();
    return pipe->getView( getNativeContext().view );
}

const View* Channel::getNativeView() const
{
    EQ_TS_THREAD( _pipeThread );
    const Pipe* pipe = getPipe();
    return pipe->getView( getNativeContext().view );
}

void Channel::changeLatency( const uint32_t latency )
{
#ifndef NDEBUG
    for( detail::Channel::StatisticsRBCIter i = _impl->statistics->begin();
         i != _impl->statistics->end(); ++i )
    {
        EQASSERT( (*i).used == 0 );
    }
#endif //NDEBUG
    _impl->statistics->resize( latency + 1 );
}

//---------------------------------------------------------------------------
// apply convenience methods
//---------------------------------------------------------------------------
void Channel::applyFrameBufferObject()
{
    EQ_TS_THREAD( _pipeThread );
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
    EQ_TS_THREAD( _pipeThread );
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
    EQ_TS_THREAD( _pipeThread );
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
    EQ_TS_THREAD( _pipeThread );
    const ColorMask& colorMask = getDrawBufferMask();
    glColorMask( colorMask.red, colorMask.green, colorMask.blue, true );
}

void Channel::applyViewport() const
{
    EQ_TS_THREAD( _pipeThread );
    const PixelViewport& pvp = getPixelViewport();
    // TODO: OPT return if vp unchanged

    if( !pvp.hasArea( ))
    {
        EQERROR << "Can't apply viewport " << pvp << std::endl;
        return;
    }

    EQ_GL_CALL( glViewport( pvp.x, pvp.y, pvp.w, pvp.h ));
    EQ_GL_CALL( glScissor( pvp.x, pvp.y, pvp.w, pvp.h ));
}

void Channel::applyFrustum() const
{
    EQ_TS_THREAD( _pipeThread );
    if( useOrtho( ))
        applyOrtho();
    else
        applyPerspective();
}

void Channel::applyPerspective() const
{
    EQ_TS_THREAD( _pipeThread );
    Frustumf frustum = getPerspective();
    const Vector2f jitter = getJitter();

    frustum.apply_jitter( jitter );
    EQ_GL_CALL( glFrustum( frustum.left(), frustum.right(),
                           frustum.bottom(), frustum.top(),
                           frustum.near_plane(), frustum.far_plane() )); 
}

void Channel::applyOrtho() const
{
    EQ_TS_THREAD( _pipeThread );
    Frustumf ortho = getOrtho();
    const Vector2f jitter = getJitter();

    ortho.apply_jitter( jitter );
    EQ_GL_CALL( glOrtho( ortho.left(), ortho.right(),
                         ortho.bottom(), ortho.top(),
                         ortho.near_plane(), ortho.far_plane() )); 
}

void Channel::applyScreenFrustum() const
{
    EQ_TS_THREAD( _pipeThread );
    const Frustumf frustum = getScreenFrustum();
    EQ_GL_CALL( glOrtho( frustum.left(), frustum.right(),
                         frustum.bottom(), frustum.top(),
                         frustum.near_plane(), frustum.far_plane() ));
}

void Channel::applyHeadTransform() const
{
    EQ_TS_THREAD( _pipeThread );
    if( useOrtho( ))
        applyOrthoTransform();
    else
        applyPerspectiveTransform();
}

void Channel::applyPerspectiveTransform() const
{
    EQ_TS_THREAD( _pipeThread );
    const Matrix4f& xfm = getPerspectiveTransform();
    EQ_GL_CALL( glMultMatrixf( xfm.array ));
}

void Channel::applyOrthoTransform() const
{
    EQ_TS_THREAD( _pipeThread );
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
        static co::base::RNG rng;
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
    EQ_TS_THREAD( _pipeThread );
    return _impl->inputFrames;
}

const Frames& Channel::getOutputFrames()
{
    EQ_TS_THREAD( _pipeThread );
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
#ifdef NDEBUG
        const PixelViewport pvpBefore = getRegion();
#endif
        while( _removeOverlap( regions )) /* nop */ ;

#ifdef NDEBUG
        EQASSERT( !_hasOverlap( regions ));
        EQASSERT( pvpBefore == getRegion( ));
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
    ConfigEvent configEvent;
    configEvent.data = event;

    switch( event.type )
    {
        case Event::CHANNEL_POINTER_MOTION:
        case Event::CHANNEL_POINTER_BUTTON_PRESS:
        case Event::CHANNEL_POINTER_BUTTON_RELEASE:
        case Event::STATISTIC:
            break;

        case Event::CHANNEL_RESIZE:
        {
            const co::base::UUID& viewID = getNativeContext().view.identifier;
            if( viewID == co::base::UUID::ZERO )
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
            EQWARN << "Unhandled channel event of type " << event.type
                   << std::endl;
            EQUNIMPLEMENTED;
    }

    Config* config = getConfig();
    config->sendEvent( configEvent );
    return true;
}

namespace
{
#define HEIGHT 12
#define SPACE  2

struct EntityData
{
    EntityData() : yPos( 0 ), doubleHeight( false ) {}
    uint32_t yPos;
    bool doubleHeight;
    std::string name;
    std::set< uint32_t > downloaders;
    std::set< uint32_t > compressors;
};

struct IdleData
{
    IdleData() : idle( 0 ), nIdle( 0 ) {}
    uint32_t idle;
    uint32_t nIdle;
    std::string name;
};

static bool _compare( const Statistic& stat1, const Statistic& stat2 )
{ return stat1.type < stat2.type; }

}

void Channel::drawStatistics()
{
    const PixelViewport& pvp = getPixelViewport();
    EQASSERT( pvp.hasArea( ));
    if( !pvp.hasArea( ))
        return;

    Config* config = getConfig();
    EQASSERT( config );

    std::vector< eq::FrameStatistics > statistics;
    config->getStatistics( statistics );

    if( statistics.empty( )) 
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
    
    Window* window = getWindow();
    const Window::Font* font = window->getSmallFont();

    //----- find min/max time
    int64_t xMax = 0;
    int64_t xMin = std::numeric_limits< int64_t >::max();

    std::map< uint32_t, EntityData > entities;
    std::map< uint32_t, IdleData >   idles;

    for( std::vector< eq::FrameStatistics >::iterator i = statistics.begin();
         i != statistics.end(); ++i )
    {
        eq::FrameStatistics& frameStats  = *i;
        SortedStatistics& configStats = frameStats.second;

        for( SortedStatistics::iterator j = configStats.begin();
             j != configStats.end(); ++j )
        {
            const uint32_t id = j->first;
            Statistics& stats = j->second;
            std::sort( stats.begin(), stats.end(), _compare );

            for( Statistics::const_iterator k = stats.begin(); 
                 k != stats.end(); ++k )
            {
                const Statistic& stat = *k;

                switch( stat.type )
                {
                  case Statistic::PIPE_IDLE:
                  {
                    IdleData& data = idles[ id ];
                    data.name = stat.resourceName;
                    data.idle += (stat.idleTime * 100ll / stat.totalTime);
                    ++data.nIdle;
                    continue;
                  }

                  case Statistic::WINDOW_FPS:
                    continue;

                  case Statistic::CHANNEL_FRAME_TRANSMIT:
                  case Statistic::CHANNEL_FRAME_COMPRESS:
                  case Statistic::CHANNEL_FRAME_WAIT_SENDTOKEN:
                    entities[ id ].doubleHeight = true;
                    break;
                  default:
                    break;
                }

                xMax = EQ_MAX( xMax, stat.endTime );
                xMin = EQ_MIN( xMin, stat.startTime );

                if( entities.find( id ) == entities.end( ))
                {
                    EntityData& data = entities[ id ];
                    data.name = stats.front().resourceName;
                }
            }
        }
    }
    const Viewport& vp = getViewport();
    const uint32_t width = uint32_t( pvp.w/vp.w );
    uint32_t scale = 1;

    while( (xMax - xMin) / scale > width )
        scale *= 10;

    xMax  /= scale;
    int64_t xStart = xMax - width + SPACE;

    const uint32_t height = uint32_t( pvp.h / vp.h);
    uint32_t nextY = height - SPACE;

    //----- statistics
    float dim = 0.0f;
    for( std::vector< eq::FrameStatistics >::reverse_iterator 
             i = statistics.rbegin(); i != statistics.rend(); ++i )
    {
        const eq::FrameStatistics& frameStats  = *i;
        const SortedStatistics& configStats = frameStats.second;

        int64_t     frameMin = xMax;
        int64_t     frameMax = 0;

        // draw stats
        for( SortedStatistics::const_iterator j = configStats.begin();
             j != configStats.end(); ++j )
        {
            const uint32_t id = j->first;
            const Statistics& stats = j->second;

            if( stats.empty( ))
                continue;

            std::map< uint32_t, EntityData >::iterator l = entities.find( id );
            if( l == entities.end( ))
                continue;

            EntityData& data = l->second;
            if( data.yPos == 0 )
            {
                data.yPos = nextY;
                nextY -= (HEIGHT + SPACE);
                if( data.doubleHeight )
                    nextY -= (HEIGHT + SPACE);
            }

            uint32_t y = data.yPos;

            for( Statistics::const_iterator k = stats.begin(); 
                 k != stats.end(); ++k )
            {
                const Statistic& stat = *k;

                switch( stat.type )
                {
                  case Statistic::PIPE_IDLE:
                  case Statistic::WINDOW_FPS:
                    continue;

                  case Statistic::CHANNEL_FRAME_TRANSMIT:
                  case Statistic::CHANNEL_FRAME_COMPRESS:
                  case Statistic::CHANNEL_FRAME_WAIT_SENDTOKEN:
                    y = data.yPos - (HEIGHT + SPACE);
                    break;
                default:
                    break;
                }

                const int64_t startTime = stat.startTime / scale;
                const int64_t endTime   = stat.endTime   / scale;

                frameMin = EQ_MIN( frameMin, startTime );
                frameMax = EQ_MAX( frameMax, endTime   );

                if( endTime < xStart || endTime == startTime )
                    continue;

                float y1 = static_cast< float >( y );
                float y2 = static_cast< float >( y - HEIGHT );
                const float x1 = static_cast< float >( startTime - xStart );
                const float x2 = static_cast< float >( endTime   - xStart );
                std::stringstream text;
                
                switch( stat.type )
                {
                  case Statistic::CONFIG_WAIT_FINISH_FRAME:
                  case Statistic::CHANNEL_FRAME_WAIT_READY:
                  case Statistic::CHANNEL_FRAME_WAIT_SENDTOKEN:
                    y1 -= SPACE;
                    y2 += SPACE;
                    break;

                  case Statistic::CHANNEL_FRAME_COMPRESS:
                    y1 -= SPACE;
                    y2 += SPACE;
                    text << unsigned( 100.f * stat.ratio ) << '%';
                    if( stat.plugins[ 0 ]  > EQ_COMPRESSOR_NONE )
                        data.compressors.insert( stat.plugins[0] );
                    if( stat.plugins[ 1 ]  > EQ_COMPRESSOR_NONE )
                        data.compressors.insert( stat.plugins[1] );
                    break;

                  case Statistic::CHANNEL_READBACK:
                    text << unsigned( 100.f * stat.ratio ) << '%';
                    if( stat.plugins[ 0 ]  > EQ_COMPRESSOR_NONE )
                        data.downloaders.insert( stat.plugins[0] );
                    if( stat.plugins[ 1 ]  > EQ_COMPRESSOR_NONE )
                        data.downloaders.insert( stat.plugins[1] );
                    break;

                default:
                    break;
                }
                
                const Vector3f color( Statistic::getColor( stat.type ) - dim );
                glColor3fv( color.array );

                glBegin( GL_QUADS );
                    glVertex3f( x2, y1, 0.f );
                    glVertex3f( x1, y1, 0.f );
                    glVertex3f( x1, y2, 0.f);
                    glVertex3f( x2, y2, 0.f );
                glEnd();

                if( !text.str().empty( ))
                {
                    glColor3f( 1.f, 1.f, 1.f );
                    glRasterPos3f( x1+1, y2, 0.f );
                    font->draw( text.str( ));
                }
            }
        }

        frameMin -= xStart;
        frameMax -= xStart;

        float x = static_cast< float >( frameMin );
        const float y1 = static_cast< float >( nextY );
        const float y2 = static_cast< float >( height );

        glBegin( GL_LINES );
            glColor3f( .5f-dim, 1.0f-dim, .5f-dim );
            glVertex3f( x, y1, 0.3f );
            glVertex3f( x, y2, 0.3f );

            x = static_cast< float >( frameMax );
            glColor3f( .5f-dim, .5f-dim, .5f-dim );
            glVertex3f( x, y1, 0.3f );
            glVertex3f( x, y2, 0.3f );
        glEnd();

        dim += .1f;
    }

    //----- Entitity names
    for( std::map< uint32_t, EntityData >::const_iterator i = entities.begin();
         i != entities.end(); ++i )
    {
        const EntityData& data = i->second;

        glColor3f( 1.f, 1.f, 1.f );
        glRasterPos3f( 60.f, data.yPos-SPACE-12.0f, 0.99f );
        font->draw( data.name );

        std::stringstream downloaders;
        for( std::set<uint32_t>::const_iterator j = data.downloaders.begin();
             j != data.downloaders.end(); ++j )
        {
            downloaders << " 0x" << std::hex << *j << std::dec;
        }
        if( !downloaders.str().empty( ))
            font->draw( std::string( ", r" ) + downloaders.str( ));

        std::stringstream compressors;
        for( std::set<uint32_t>::const_iterator j = data.compressors.begin();
             j != data.compressors.end(); ++j )
        {
            compressors << " 0x" << std::hex << *j << std::dec;
        }
        if( !compressors.str().empty( ))
        {
            glRasterPos3f( 80.f, data.yPos - HEIGHT - 2*SPACE - 12.0f, 0.99f );
            font->draw( std::string( "compressors" ) + compressors.str( ));
        }
    }

    //----- Global stats (scale, GPU idle)
    glColor3f( 1.f, 1.f, 1.f );
    nextY -= (HEIGHT + SPACE);
    glRasterPos3f( 60.f, static_cast< float >( nextY ), 0.99f );
    std::ostringstream text;
    text << scale << "ms/pixel";

    if( !idles.empty( ))
        text << ", Idle:";

    for( std::map< uint32_t, IdleData >::const_iterator i = idles.begin();
         i != idles.end(); ++i )
    {
        const IdleData& data = i->second;
        EQASSERT( data.nIdle > 0 );

        text << " " << data.name << ":" << data.idle / data.nIdle << "%";
    }

    font->draw( text.str( ));
    
    //----- Legend
    nextY -= SPACE;
    float x = 0.f;

    glRasterPos3f( x+1.f, nextY-12.f, 0.f );
    glColor3f( 1.f, 1.f, 1.f );
    font->draw( "channel" );

    for( size_t i = 1; i < Statistic::CONFIG_START_FRAME; ++i )
    {
        const Statistic::Type type = static_cast< Statistic::Type >( i );
        if( type == Statistic::CHANNEL_DRAW_FINISH ||
            type == Statistic::PIPE_IDLE || type == Statistic::WINDOW_FPS )
        {
            continue;
        }

        switch( type )
        {
          case Statistic::CHANNEL_FRAME_TRANSMIT:
            x = 0.f;
            nextY -= (HEIGHT + SPACE);

            glColor3f( 1.f, 1.f, 1.f );
            glRasterPos3f( x+1.f, nextY-12.f, 0.f );
            break;

          case Statistic::WINDOW_FINISH:
            x = 0.f;
            nextY -= (HEIGHT + SPACE);

            glColor3f( 1.f, 1.f, 1.f );
            glRasterPos3f( x+1.f, nextY-12.f, 0.f );
            font->draw( "window" );
            break;

          case Statistic::NODE_FRAME_DECOMPRESS:
            x = 0.f;
            nextY -= (HEIGHT + SPACE);

            glColor3f( 1.f, 1.f, 1.f );
            glRasterPos3f( x+1.f, nextY-12.f, 0.f );
            font->draw( "node" );
            break;

          default:
            break;
        }

        x += 60.f;
        const float x2 = x + 60.f - SPACE; 
        const float y1 = static_cast< float >( nextY );
        const float y2 = static_cast< float >( nextY - HEIGHT );

        glColor3fv( Statistic::getColor( type ).array );
        glBegin( GL_QUADS );
            glVertex3f( x2, y1, 0.f );
            glVertex3f( x,  y1, 0.f );
            glVertex3f( x,  y2, 0.f );
            glVertex3f( x2, y2, 0.f );
        glEnd();

        glColor3f( 0.f, 0.f, 0.f );
        glRasterPos3f( x+1.f, nextY-12.f, 0.f );
        font->draw( Statistic::getName( type ));
    }
    // nextY -= (HEIGHT + SPACE);
    
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

struct Channel::RBStat
{
    RBStat( Channel* channel )
            : event( Statistic::CHANNEL_READBACK, channel ), uncompressed( 0 )
            , compressed( 0 )
        {
            event.event.data.statistic.plugins[0] = EQ_COMPRESSOR_NONE;
            event.event.data.statistic.plugins[1] = EQ_COMPRESSOR_NONE;
            EQASSERT( event.event.data.statistic.frameNumber > 0 );
        }

    co::base::SpinLock lock;
    ChannelStatistics event;
    size_t uncompressed;
    size_t compressed;

    void ref() { ++_refCount; }
    void unref()
        {
            if( --_refCount > 0 )
                return;

            if( uncompressed > 0 && compressed > 0 )
                event.event.data.statistic.ratio = float( compressed ) /
                                                   float( uncompressed );
            else
                event.event.data.statistic.ratio = 1.0f;
            delete this;
        }

    int32_t getRefCount() const { return _refCount; }
private:
    a_int32_t _refCount;
};

typedef co::base::RefPtr< Channel::RBStat > RBStatPtr;

void Channel::_frameTiles( const ChannelFrameTilesPacket* packet )
{
    RenderContext context = packet->context;
    _setRenderContext( context );

    frameTilesStart( packet->context.frameID );

    RBStatPtr stat;
    if( packet->tasks & fabric::TASK_READBACK )
    {
        _setOutputFrames( packet->nFrames, packet->frames );
        stat = new RBStat( this );
    }

    int64_t startTime = getConfig()->getTime();
    int64_t clearTime = 0;
    int64_t drawTime = 0;
    int64_t readbackTime = 0;
    bool hasAsyncReadback = false;

    co::QueueSlave* queue = _getQueue( packet->queueVersion );
    EQASSERT( queue );
    for( co::Command* queuePacket = queue->pop(); queuePacket;
         queuePacket = queue->pop( ))
    {
        const TileTaskPacket* tilePacket = queuePacket->get<TileTaskPacket>();
        context.frustum = tilePacket->frustum;
        context.ortho = tilePacket->ortho;
        context.pvp = tilePacket->pvp;
        context.vp = tilePacket->vp;

        if ( !packet->isLocal )
        {
            context.pvp.x = 0;
            context.pvp.y = 0;
        }

        if( packet->tasks & fabric::TASK_CLEAR )
        {
            const int64_t time = getConfig()->getTime();
            frameClear( packet->context.frameID );
            clearTime += getConfig()->getTime() - time;
        }

        if( packet->tasks & fabric::TASK_DRAW )
        {
            const int64_t time = getConfig()->getTime();
            frameDraw( packet->context.frameID );
            drawTime += getConfig()->getTime() - time;
        }

        if( packet->tasks & fabric::TASK_READBACK )
        {
            const int64_t time = getConfig()->getTime();

            const Frames& frames = getOutputFrames();
            const size_t nFrames = frames.size();
            std::vector< size_t > nImages( nFrames, 0 );
            for( size_t i = 0; i < nFrames; ++i )
            {
                nImages[i] = frames[i]->getImages().size();
                frames[i]->getData()->setPixelViewport( getPixelViewport() );
            }

            frameReadback( packet->context.frameID );
            readbackTime += getConfig()->getTime() - time;

            for( size_t i = 0; i < nFrames; ++i )
            {
                const Frame* frame = frames[i];
                const Images& images = frame->getImages();
                for( size_t j = nImages[i]; j < images.size(); ++j )
                {
                    Image* image = images[j];
                    const PixelViewport& pvp = image->getPixelViewport();
                    image->setOffset( pvp.x + tilePacket->pvp.x,
                                      pvp.y + tilePacket->pvp.y );
                }
            }

            if( _startFinishReadback( stat.get(), nImages, false ))
                hasAsyncReadback = true;
        }
        queuePacket->release();
    }

    if( packet->tasks & fabric::TASK_CLEAR )
    {
        ChannelStatistics event( Statistic::CHANNEL_CLEAR, this );
        event.event.data.statistic.startTime = startTime;
        startTime += clearTime;
        event.event.data.statistic.endTime = startTime;
    }

    if( packet->tasks & fabric::TASK_DRAW )
    {
        ChannelStatistics event( Statistic::CHANNEL_DRAW, this );
        event.event.data.statistic.startTime = startTime;
        startTime += drawTime;
        event.event.data.statistic.endTime = startTime;
    }

    if( packet->tasks & fabric::TASK_READBACK )
    {
        stat->event.event.data.statistic.startTime = startTime;
        startTime += readbackTime;
        stat->event.event.data.statistic.endTime = startTime;

        const Frames& frames = getOutputFrames();
        for( FramesCIter i = frames.begin(); i != frames.end(); ++i )
        {
            Frame* frame = *i;
            const Eye eye = getEye();
            const std::vector< uint128_t >& nodes = frame->getInputNodes( eye );
            const std::vector< uint128_t >& netNodes =
                frame->getInputNetNodes( eye );

            if( hasAsyncReadback )
                _startSetReady( frame->getData(), stat.get(), nodes, netNodes );
            else
                _setReady( frame->getData(), stat.get(), nodes, netNodes );
        }
    }

    frameTilesFinish( packet->context.frameID );
    resetRenderContext();
}

void Channel::_refFrame( const uint32_t frameNumber )
{
    const size_t index = frameNumber % _impl->statistics->size();
    detail::Channel::FrameStatistics& stats = _impl->statistics.data[ index ];
    EQASSERTINFO( stats.used > 0, frameNumber );
    ++stats.used;
}

void Channel::_unrefFrame( const uint32_t frameNumber )
{
    const size_t index = frameNumber % _impl->statistics->size();
    detail::Channel::FrameStatistics& stats = _impl->statistics.data[ index ];
    if( --stats.used != 0 ) // Frame still in use
        return;

    ChannelFrameFinishReplyPacket reply;
    reply.nStatistics = uint32_t( stats.data.size( ));
    reply.frameNumber = frameNumber;
    reply.objectID = getID();
    reply.region = stats.region;
    getServer()->send( reply, stats.data );

    stats.data.clear();
    stats.region = Viewport::FULL;
}

void Channel::_setOutputFrames( const uint32_t nFrames,
                                const co::ObjectVersion* frames )
{
    EQ_TS_THREAD( _pipeThread );
    for( uint32_t i=0; i<nFrames; ++i )
    {
        Pipe*  pipe  = getPipe();
        Frame* frame = pipe->getFrame( frames[i], getEye(), true );
        _impl->outputFrames.push_back( frame );
    }
}

void Channel::_resetOutputFrames()
{
    EQ_TS_THREAD( _pipeThread );
    _impl->outputFrames.clear();
}

//---------------------------------------------------------------------------
// Asynchronous image readback, compression and transmission
//---------------------------------------------------------------------------
void Channel::_frameReadback( const uint128_t& frameID, uint32_t nFrames,
                              co::ObjectVersion* frames )
{
    EQ_TS_THREAD( _pipeThread );

    RBStatPtr stat = new RBStat( this );
    _setOutputFrames( nFrames, frames );

    std::vector< size_t > nImages( nFrames, 0 );
    for( size_t i = 0; i < nFrames; ++i )
        nImages[i] = _impl->outputFrames[i]->getImages().size();

    frameReadback( frameID );
    EQASSERT( stat->event.event.data.statistic.frameNumber > 0 );
    _startFinishReadback( stat.get(), nImages, true );
    _resetOutputFrames();
}

bool Channel::_startFinishReadback( RBStat* stat,
                                    const std::vector< size_t >& imagePos,
                                    const bool ready )
{
    EQ_TS_THREAD( _pipeThread );

    bool hasAsyncReadback = false;
    const Frames& frames = getOutputFrames();
    EQASSERT( frames.size() == imagePos.size( ));

    for( size_t i = 0; i < frames.size(); ++i )
    {
        Frame* frame = frames[i];
        FrameData* frameData = frame->getData();
        const uint32_t frameNumber = getCurrentFrame();

        if( frameData->getBuffers() == 0 )
        {
            if( ready )
                _setReady( frameData, stat );
            continue;
        }

        const Images& images = frameData->getImages();
        const size_t nImages = images.size();
        const Eye eye = getEye();
        const std::vector< uint128_t >& nodes = frame->getInputNodes( eye );
        const std::vector< uint128_t >& netNodes = frame->getInputNetNodes(eye);

        bool asyncRB = false;
        for( size_t j = imagePos[i]; j < nImages; ++j )
        {
            if( images[j]->hasAsyncReadback( )) // finish readback asynchron
            {
                EQCHECK( getPipe()->startAsyncRBThread( ));

                asyncRB = true;
                _refFrame( frameNumber );

                ChannelFinishReadbackPacket packet;
                packet.taskID = getTaskID();
                packet.frameData = frameData;
                packet.frameNumber = frameNumber;
                packet.imageIndex = j;
                packet.nNodes = nodes.size();

                std::vector< uint128_t > ids = nodes;
                ids.insert( ids.end(), netNodes.begin(), netNodes.end( ));
                send( getLocalNode(), packet, ids );
            }
            else // transmit images asynchronously
                _startTransmit( frameData, frameNumber, j, nodes, netNodes,
                                getTaskID( ));
        }

        EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "Frame readback async " << asyncRB
                                          <<  " ready "  << ready << std::endl;
        if( ready )
        {
            if( asyncRB )
            {
                hasAsyncReadback = true;
                _startSetReady( frameData, stat, nodes, netNodes );
            }
            else
                _setReady( frameData, stat, nodes, netNodes );
        }
    }
    return hasAsyncReadback;
}

void Channel::_finishReadback( const ChannelFinishReadbackPacket* packet )
{
    EQLOG( LOG_TASKS|LOG_ASSEMBLY ) << "Finish readback " << packet << std::endl;

    FrameData* frameData = getNode()->getFrameData( packet->frameData );
    const Images& images = frameData->getImages();
    Image* image = images[ packet->imageIndex ];
    const GLEWContext* glewContext = getPipe()->getAsyncGlewContext();

    EQASSERT( frameData );
    EQASSERT( images.size() > packet->imageIndex );
    EQASSERT( image->hasAsyncReadback( ));

    image->finishReadback( frameData->getZoom(), glewContext );
    EQASSERT( !image->hasAsyncReadback( ));

    // schedule async image tranmission
    std::vector< uint128_t > nodes;
    std::vector< uint128_t > netNodes;
    nodes.insert( nodes.end(), packet->IDs, packet->IDs + packet->nNodes );
    netNodes.insert( netNodes.end(), packet->IDs + packet->nNodes,
                     packet->IDs + 2 * packet->nNodes );

    _startTransmit( frameData, packet->frameNumber, packet->imageIndex, nodes,
                    netNodes, packet->taskID );
}

void Channel::_startTransmit( FrameData* frame, const uint32_t frameNumber,
                              const size_t image,
                              const std::vector<uint128_t>& nodes,
                              const std::vector< uint128_t >& netNodes,
                              const uint32_t taskID )
{
    EQASSERT( nodes.size() == netNodes.size( ));
    std::vector< uint128_t >::const_iterator j = netNodes.begin();
    for( std::vector< uint128_t >::const_iterator i = nodes.begin();
         i != nodes.end(); ++i, ++j )
    {
        _refFrame( frameNumber );

        ChannelFrameTransmitImagePacket packet;
        packet.frameData = frame;
        packet.netNodeID = *j;
        packet.nodeID = *i;
        packet.frameNumber = frameNumber;
        packet.imageIndex = image;
        packet.taskID = taskID;

        EQLOG( LOG_TASKS|LOG_ASSEMBLY ) << "Start transmit " << &packet
                                        << std::endl;
        send( getNode()->getLocalNode(), packet );
    }
}

void Channel::_transmitImage( const ChannelFrameTransmitImagePacket* request )
{
    EQLOG( LOG_TASKS|LOG_ASSEMBLY ) << "Transmit " << request << std::endl;

    FrameData* frameData = getNode()->getFrameData( request->frameData ); 
    EQASSERT( frameData );

    if( frameData->getBuffers() == 0 )
    {
        EQWARN << "No buffers for frame data" << std::endl;
        return;
    }

    ChannelStatistics transmitEvent( Statistic::CHANNEL_FRAME_TRANSMIT, this,
                                     request->frameNumber );
    transmitEvent.event.data.statistic.task = request->taskID;

    const Images& images = frameData->getImages();
    Image* image = images[ request->imageIndex ];
    EQASSERT( images.size() > request->imageIndex );

    if( image->getStorageType() == Frame::TYPE_TEXTURE )
    {
        EQWARN << "Can't transmit image of type TEXTURE" << std::endl;
        EQUNIMPLEMENTED;
        return;
    }

    co::LocalNodePtr localNode = getLocalNode();
    co::NodePtr toNode = localNode->connect( request->netNodeID );
    if( !toNode || !toNode->isConnected( ))
    {
        EQWARN << "Can't connect node " << request->netNodeID
               << " to send image data" << std::endl;
        return;
    }

    co::ConnectionPtr connection = toNode->getConnection();
    co::ConnectionDescriptionPtr description = connection->getDescription();

    // use compression on links up to 2 GBit/s
    const bool useCompression = ( description->bandwidth <= 262144 );

    NodeFrameDataTransmitPacket packet;
    const uint64_t packetSize = sizeof( packet ) - 8 * sizeof( uint8_t );

    packet.objectID    = request->nodeID;
    packet.frameData   = request->frameData;
    packet.frameNumber = request->frameNumber;

    std::vector< const PixelData* > pixelDatas;
    std::vector< float > qualities;

    packet.size = packetSize;
    packet.buffers = Frame::BUFFER_NONE;
    packet.pvp = image->getPixelViewport();
    packet.useAlpha = image->getAlphaUsage();
    packet.zoom = image->getZoom();
    EQASSERT( packet.pvp.isValid( ));

    {
        uint64_t rawSize( 0 );
        ChannelStatistics compressEvent( Statistic::CHANNEL_FRAME_COMPRESS, 
                                         this, request->frameNumber,
                                         useCompression ? AUTO : OFF );
        compressEvent.event.data.statistic.task = request->taskID;
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
                packet.size += sizeof( FrameData::ImageHeader ); 

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
                        packet.size += sizeof( uint64_t );
                        packet.size += data.compressedSize[ k ];
                    }
                    compressEvent.event.data.statistic.plugins[j] =
                        data.compressorName;
                }
                else
                {
                    packet.size += sizeof( uint64_t );
                    packet.size += image->getPixelDataSize( buffer );
                }

                packet.buffers |= buffer;
                rawSize += image->getPixelDataSize( buffer );
            }
        }

        if( rawSize > 0 )
            compressEvent.event.data.statistic.ratio =
            static_cast< float >( packet.size ) /
            static_cast< float >( rawSize );
    }

    if( pixelDatas.empty( ))
        return;

    // send image pixel data packet
    co::LocalNode::SendToken token;
    if( getIAttribute( IATTR_HINT_SENDTOKEN ) == ON )
    {
        ChannelStatistics waitEvent( Statistic::CHANNEL_FRAME_WAIT_SENDTOKEN,
                                     this, request->frameNumber );
        waitEvent.event.data.statistic.task = request->taskID;
        token = getLocalNode()->acquireSendToken( toNode );
    }

    connection->lockSend();
    connection->send( &packet, packetSize, true );
#ifndef NDEBUG
    size_t sentBytes = packetSize;
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
    EQASSERTINFO( sentBytes == packet.size,
        sentBytes << " != " << packet.size );
#endif

    connection->unlockSend();
    getLocalNode()->releaseSendToken( token );
}

void Channel::_startSetReady( const FrameData* frame, RBStat* stat,
                              const std::vector< uint128_t >& nodes,
                              const std::vector< uint128_t >& netNodes )
{
    stat->ref();
    std::vector< uint128_t > ids = nodes;
    ids.insert( ids.end(), netNodes.begin(), netNodes.end( ));

    EQASSERT( stat->event.event.data.statistic.frameNumber > 0 );
    ChannelFrameSetReadyPacket packet( frame, stat, nodes.size( ));
    send( getLocalNode(), packet, ids );
}

void Channel::_setReady( const ChannelFrameSetReadyPacket* packet )
{
    FrameData* frameData = getNode()->getFrameData( packet->frameData );
    std::vector< uint128_t > nodes;
    std::vector< uint128_t > netNodes;
    nodes.insert( nodes.end(), packet->IDs, packet->IDs + packet->nNodes );
    netNodes.insert( netNodes.end(), packet->IDs + packet->nNodes,
                     packet->IDs + 2 * packet->nNodes );

    EQASSERT( packet->stat->event.event.data.statistic.frameNumber > 0 );
   _setReady( frameData, packet->stat, nodes, netNodes );
   packet->stat->unref();
}

void Channel::_setReady( FrameData* frame, RBStat* stat,
                         const std::vector< uint128_t >& nodes,
                         const std::vector< uint128_t >& netNodes )
{
    EQLOG( LOG_TASKS|LOG_ASSEMBLY ) << "Set ready " << co::ObjectVersion( frame )
                                    << std::endl;
    frame->setReady();

    const uint32_t frameNumber = stat->event.event.data.statistic.frameNumber;
    std::vector<uint128_t>::const_iterator j = netNodes.begin();
    for( std::vector<uint128_t>::const_iterator i = nodes.begin();
         i != nodes.end(); ++i, ++j )
    {
        _refFrame( frameNumber );

        ChannelFrameSetReadyNodePacket packet( frame, *i, *j, frameNumber );
        send( getLocalNode(), packet );
    }

    const DrawableConfig& dc = getDrawableConfig();
    const size_t colorBytes = ( 3 * dc.colorBits + dc.alphaBits ) / 8;

    {
        co::base::ScopedFastWrite mutex( stat->lock );
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
                stat->compressed += image->getPixelDataSize(Frame::BUFFER_DEPTH);
                stat->event.event.data.statistic.plugins[1] =
                                 image->getDownloaderName( Frame::BUFFER_DEPTH );
            }
        }
    }
}

void Channel::_setReadyNode( const ChannelFrameSetReadyNodePacket* packet )
{
    co::LocalNodePtr localNode = getLocalNode();
    co::NodePtr toNode = localNode->connect( packet->netNodeID );
    const FrameData* frameData = getNode()->getFrameData( packet->frameData );

    NodeFrameDataReadyPacket readyPacket( frameData );
    readyPacket.objectID = packet->nodeID;
    toNode->send( readyPacket );
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
bool Channel::_cmdConfigInit( co::Command& command )
{
    const ChannelConfigInitPacket* packet =
        command.get<ChannelConfigInitPacket>();
    EQLOG( LOG_INIT ) << "TASK channel config init " << packet << std::endl;

    const Config* config = getConfig();
    changeLatency( config->getLatency( ));

    ChannelConfigInitReplyPacket reply;
    setError( ERROR_NONE );

    const Window* window = getWindow();
    if( window->isRunning( ))
    {
        _impl->state = STATE_INITIALIZING;

        const PixelViewport& pvp = getPixelViewport();
        EQASSERT( pvp.hasArea( ));
        _impl->initialSize.x() = pvp.w;
        _impl->initialSize.y() = pvp.h;

        reply.result = configInit( packet->initID );

        if( reply.result )
        {
            _initDrawableConfig();
            _impl->state = STATE_RUNNING;
        }
    }
    else
    {
        setError( ERROR_CHANNEL_WINDOW_NOTRUNNING );
        reply.result = false;
    }

    EQLOG( LOG_INIT ) << "TASK channel config init reply " << &reply
                      << std::endl;
    commit();
    send( command.getNode(), reply );
    return true;
}

bool Channel::_cmdConfigExit( co::Command& command )
{
    const ChannelConfigExitPacket* packet =
        command.get<ChannelConfigExitPacket>();
    EQLOG( LOG_INIT ) << "Exit channel " << packet << std::endl;

    if( _impl->state != STATE_STOPPED )
        _impl->state = configExit() ? STATE_STOPPED : STATE_FAILED;

    WindowDestroyChannelPacket destroyPacket( getID( ));
    getWindow()->send( getLocalNode(), destroyPacket );
    return true;
}

bool Channel::_cmdFrameStart( co::Command& command )
{
    ChannelFrameStartPacket* packet = 
        command.getModifiable< ChannelFrameStartPacket >();
    EQVERB << "handle channel frame start " << packet << std::endl;

    //_grabFrame( packet->frameNumber ); single-threaded
    sync( packet->version );

    overrideContext( packet->context );
    bindFrameBuffer();
    frameStart( packet->context.frameID, packet->frameNumber );

    const size_t index = packet->frameNumber % _impl->statistics->size();
    detail::Channel::FrameStatistics& statistic = _impl->statistics.data[index];
    EQASSERTINFO( statistic.used == 0, packet->frameNumber );
    EQASSERT( statistic.data.empty( ));
    statistic.used = 1;

    resetRenderContext();
    return true;
}

bool Channel::_cmdFrameFinish( co::Command& command )
{
    ChannelFrameFinishPacket* packet =
        command.getModifiable< ChannelFrameFinishPacket >();
    EQLOG( LOG_TASKS ) << "TASK frame finish " << getName() <<  " " << packet
                       << std::endl;

    overrideContext( packet->context );
    frameFinish( packet->context.frameID, packet->frameNumber );
    resetRenderContext();

    _unrefFrame( packet->frameNumber );
    return true;
}

bool Channel::_cmdFrameClear( co::Command& command )
{
    EQASSERT( _impl->state == STATE_RUNNING );
    ChannelFrameClearPacket* packet = 
        command.getModifiable< ChannelFrameClearPacket >();
    EQLOG( LOG_TASKS ) << "TASK clear " << getName() <<  " " << packet
                       << std::endl;

    _setRenderContext( packet->context );
    ChannelStatistics event( Statistic::CHANNEL_CLEAR, this );
    frameClear( packet->context.frameID );
    resetRenderContext();

    return true;
}

bool Channel::_cmdFrameDraw( co::Command& command )
{
    ChannelFrameDrawPacket* packet = 
        command.getModifiable< ChannelFrameDrawPacket >();
    EQLOG( LOG_TASKS ) << "TASK draw " << getName() <<  " " << packet
                       << std::endl;

    _setRenderContext( packet->context );
    const uint32_t frameNumber = getCurrentFrame();
    ChannelStatistics event( Statistic::CHANNEL_DRAW, this, frameNumber,
                             packet->finish ? NICEST : AUTO );

    frameDraw( packet->context.frameID );
    // Update ROI for server equalizers
    if( !getRegion().isValid( ))
        declareRegion( getPixelViewport( ));
    const size_t index = frameNumber % _impl->statistics->size();
    _impl->statistics.data[ index ].region = getRegion() / getPixelViewport();

    resetRenderContext();

    return true;
}

bool Channel::_cmdFrameDrawFinish( co::Command& command )
{
    const ChannelFrameDrawFinishPacket* packet = 
        command.get< ChannelFrameDrawFinishPacket >();
    EQLOG( LOG_TASKS ) << "TASK draw finish " << getName() <<  " " << packet
                       << std::endl;

    ChannelStatistics event( Statistic::CHANNEL_DRAW_FINISH, this );
    frameDrawFinish( packet->frameID, packet->frameNumber );

    return true;
}

bool Channel::_cmdFrameAssemble( co::Command& command )
{
    ChannelFrameAssemblePacket* packet = 
        command.getModifiable< ChannelFrameAssemblePacket >();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK assemble " << getName() <<  " " 
                                      << packet << std::endl;

    _setRenderContext( packet->context );
    ChannelStatistics event( Statistic::CHANNEL_ASSEMBLE, this );
    for( uint32_t i=0; i<packet->nFrames; ++i )
    {
        Pipe*  pipe  = getPipe();
        Frame* frame = pipe->getFrame( packet->frames[i], getEye(), false );
        _impl->inputFrames.push_back( frame );
    }

    frameAssemble( packet->context.frameID );

    for( FramesCIter i = _impl->inputFrames.begin();
         i != _impl->inputFrames.end(); ++i )
    {
        // Unset the frame data on input frames, so that they only get flushed
        // once by the output frames during exit.
        (*i)->setData( 0 );
    }
    _impl->inputFrames.clear();
    resetRenderContext();

    return true;
}

bool Channel::_cmdFrameReadback( co::Command& command )
{
    ChannelFrameReadbackPacket* packet = 
        command.getModifiable< ChannelFrameReadbackPacket >();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK readback " << getName() <<  " "
                                      << packet << std::endl;

    _setRenderContext( packet->context );
    _frameReadback( packet->context.frameID, packet->nFrames, packet->frames );
    resetRenderContext();
    return true;
}

bool Channel::_cmdFinishReadback( co::Command& command )
{
    const ChannelFinishReadbackPacket* packet = 
        command.get< ChannelFinishReadbackPacket >();
    _finishReadback( packet );
    _unrefFrame( packet->frameNumber );
    return true;
}

bool Channel::_cmdFrameSetReady( co::Command& command )
{
    _setReady( command.get<ChannelFrameSetReadyPacket>( ));
    return true;
}

bool Channel::_cmdFrameTransmitImage( co::Command& command )
{
    const ChannelFrameTransmitImagePacket* packet =
        command.get<ChannelFrameTransmitImagePacket>();
    _transmitImage( packet );
    _unrefFrame( packet->frameNumber );
    return true;
}

bool Channel::_cmdFrameSetReadyNode( co::Command& command )
{
    const ChannelFrameSetReadyNodePacket* packet =
        command.get<ChannelFrameSetReadyNodePacket>();
    _setReadyNode( packet );
    _unrefFrame( packet->frameNumber );
    return true;
}

bool Channel::_cmdFrameViewStart( co::Command& command )
{
    ChannelFrameViewStartPacket* packet = 
        command.getModifiable< ChannelFrameViewStartPacket >();
    EQLOG( LOG_TASKS ) << "TASK view start " << getName() <<  " " << packet
                       << std::endl;

    _setRenderContext( packet->context );
    frameViewStart( packet->context.frameID );
    resetRenderContext();

    return true;
}

bool Channel::_cmdFrameViewFinish( co::Command& command )
{
    ChannelFrameViewFinishPacket* packet = 
        command.getModifiable< ChannelFrameViewFinishPacket >();
    EQLOG( LOG_TASKS ) << "TASK view finish " << getName() <<  " " << packet
                       << std::endl;

    _setRenderContext( packet->context );
    ChannelStatistics event( Statistic::CHANNEL_VIEW_FINISH, this );
    frameViewFinish( packet->context.frameID );
    resetRenderContext();

    return true;
}

bool Channel::_cmdStopFrame( co::Command& command )
{
    const ChannelStopFramePacket* packet = 
        command.get<ChannelStopFramePacket>();
    EQLOG( LOG_TASKS ) << "TASK channel stop frame " << getName() <<  " "
                       << packet << std::endl;
    
    notifyStopFrame( packet->lastFrameNumber );
    return true;
}

bool Channel::_cmdFrameTiles( co::Command& command )
{
    ChannelFrameTilesPacket* packet =
        command.getModifiable< ChannelFrameTilesPacket >();
    EQLOG( LOG_TASKS ) << "TASK channel frame tiles " << getName() <<  " "
                       << packet << std::endl;

    _frameTiles( packet );
    return true;
}

}

#include "../fabric/channel.ipp"
template class eq::fabric::Channel< eq::Window, eq::Channel >;

/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                                                 const eq::Super& );
/** @endcond */
