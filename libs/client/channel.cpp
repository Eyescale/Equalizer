
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include "view.h"
#include "windowPackets.h"

#include <eq/util/accum.h>
#include <eq/util/frameBufferObject.h>
#include <eq/fabric/commands.h>
#include <co/command.h>
#include <co/connectionDescription.h>
#include <co/base/rng.h>

namespace eq
{
typedef fabric::Channel< Window, Channel > Super;

Channel::Channel( Window* parent )
        : Super( parent )
        , _state( STATE_STOPPED )
        , _fbo( 0 )
        , _statisticsIndex( 0 )
        , _initialSize( Vector2i::ZERO )
{
    co::base::RNG rng;
    _color.r() = rng.get< uint8_t >();
    _color.g() = rng.get< uint8_t >();
    _color.b() = rng.get< uint8_t >();
}

Channel::~Channel()
{  
    _statistics.clear();
}

/** @cond IGNORE */
typedef co::CommandFunc<Channel> CmdFunc;
/** @endcond */

void Channel::attach( const co::base::UUID& id, const uint32_t instanceID )
{
    Super::attach( id, instanceID );
    co::CommandQueue* queue = getPipeThreadQueue();
    co::CommandQueue* commandQ = getCommandThreadQueue();
    co::CommandQueue* transmitQ = &getNode()->transmitter.getQueue();

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
    registerCommand( fabric::CMD_CHANNEL_FRAME_TRANSMIT, 
                     CmdFunc( this, &Channel::_cmdFrameTransmit ), queue );
    registerCommand( fabric::CMD_CHANNEL_FRAME_TRANSMIT_ASYNC, 
                     CmdFunc( this, &Channel::_cmdFrameTransmitAsync ),
                     transmitQ );
    registerCommand( fabric::CMD_CHANNEL_FRAME_VIEW_START, 
                     CmdFunc( this, &Channel::_cmdFrameViewStart ), queue );
    registerCommand( fabric::CMD_CHANNEL_FRAME_VIEW_FINISH, 
                     CmdFunc( this, &Channel::_cmdFrameViewFinish ), queue );
    registerCommand( fabric::CMD_CHANNEL_STOP_FRAME, 
                     CmdFunc( this, &Channel::_cmdStopFrame ), commandQ );
}

co::CommandQueue* Channel::getPipeThreadQueue()
{ 
    return getWindow()->getPipeThreadQueue(); 
}

co::CommandQueue* Channel::getCommandThreadQueue()
{ 
    return getWindow()->getCommandThreadQueue(); 
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
    if( _fbo )
        return _drawableConfig;

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
    delete _fbo;
    _fbo = 0;
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
    _fbo = new util::FrameBufferObject( glewGetContext( ));

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
    if( _fbo->init( pvp.w, pvp.h, 
                    window->getColorFormat(), depthSize, stencilSize ))
    {
        return true;
    }
    // else

    setError( _fbo->getError( ));
    delete _fbo;
    _fbo = 0;
    return false;
}

void Channel::_initDrawableConfig()
{
    const Window* window = getWindow();
    _drawableConfig = window->getDrawableConfig();
    if( !_fbo )
        return;

    const util::Textures& colors = _fbo->getColorTextures();
    if( !colors.empty( ))
    {
        switch( colors.front()->getType( ))
        {
            case GL_FLOAT:
                _drawableConfig.colorBits = 32;
                break;
            case GL_HALF_FLOAT:
                _drawableConfig.colorBits = 16;
                break;
            case GL_UNSIGNED_INT_10_10_10_2:
                _drawableConfig.colorBits = 10;
                break;

            default:
                EQUNIMPLEMENTED;
            case GL_UNSIGNED_BYTE:
                _drawableConfig.colorBits = 8;
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
    EQASSERT( event.originator != co::base::UUID::ZERO );
    event.resize.x   = newPVP.x;
    event.resize.y   = newPVP.y;
    event.resize.w   = newPVP.w;
    event.resize.h   = newPVP.h;

    processEvent( event );
}

void Channel::addStatistic( Event& event, const uint32_t index )
{
    EQASSERT( index < _statistics.size( ));
    EQASSERT( _statistics[ index ].used > 0 );
    Statistics& statistics = _statistics[ index ].data;

    statistics.push_back( event.statistic );
    processEvent( event );
}

//---------------------------------------------------------------------------
// operations
//---------------------------------------------------------------------------

void Channel::frameClear( const uint128_t& )
{
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

    Compositor::assembleFrames( getInputFrames(), this, 0 );

    EQ_GL_CALL( resetAssemblyState( ));
}

void Channel::frameReadback( const uint128_t& )
{
    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));
    EQ_GL_CALL( setupAssemblyState( ));

    Window::ObjectManager* glObjects = getObjectManager();
    const DrawableConfig& drawableConfig = getDrawableConfig();

    const Frames& frames = getOutputFrames();
    for( Frames::const_iterator i = frames.begin(); i != frames.end(); ++i)
    {
        Frame* frame = *i;
        frame->readback( glObjects, drawableConfig );
    }

    EQ_GL_CALL( resetAssemblyState( ));
}

void Channel::startFrame( const uint32_t ) { /* nop */ }
void Channel::releaseFrame( const uint32_t ) { /* nop */ }
void Channel::releaseFrameLocal( const uint32_t ) { /* nop */ }

void Channel::frameStart( const uint128_t&, const uint32_t frameNumber ) 
{
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
    // copy to be thread-safe when pvp changes
    const PixelViewport pvp( getPixelViewport( ));
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
    return _fbo;
}

View* Channel::getView()
{
    Pipe* pipe = getPipe();
    return pipe->getView( getContext().view );
}

const View* Channel::getView() const
{
    const Pipe* pipe = getPipe();
    return pipe->getView( getContext().view );
}

View* Channel::getNativeView()
{
    Pipe* pipe = getPipe();
    return pipe->getView( getNativeContext().view );
}

const View* Channel::getNativeView() const
{
    const Pipe* pipe = getPipe();
    return pipe->getView( getNativeContext().view );
}

void Channel::changeLatency( const uint32_t latency )
{
#ifndef NDEBUG
    for(  StatisticsRB::const_iterator i = _statistics.begin();
         i != _statistics.end(); ++i )
    {
        EQASSERT( (*i).used == 0 );
    }
#endif //NDEBUG
    _statistics.resize( latency + 1 );
    _statisticsIndex = 0;
}

//---------------------------------------------------------------------------
// apply convenience methods
//---------------------------------------------------------------------------

void Channel::applyFrameBufferObject()
{
    if( _fbo )
    {
        const PixelViewport& pvp = getNativePixelViewport();
        _fbo->resize( pvp.w, pvp.h );
        _fbo->bind(); 
    }
    else if( GLEW_EXT_framebuffer_object )
        glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
}

void Channel::applyBuffer()
{
    const Window* window = getWindow();
    if( !_fbo && window->getSystemWindow()->getFrameBufferObject() == 0 )
    {
        EQ_GL_CALL( glReadBuffer( getReadBuffer( )));
        EQ_GL_CALL( glDrawBuffer( getDrawBuffer( )));
    }
    
    applyColorMask();
}

void Channel::bindFrameBuffer()
{
    const Window* window = getWindow();
    if( !window->getSystemWindow( ))
       return;
        
   if( _fbo )
       applyFrameBufferObject();
   else
       window->bindFrameBuffer();
}

void Channel::applyColorMask() const
{
    const ColorMask& colorMask = getDrawBufferMask();
    glColorMask( colorMask.red, colorMask.green, colorMask.blue, true );
}

void Channel::applyViewport() const
{
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
    Frustumf frustum = getFrustum();
    const Vector2f jitter = getJitter();

    frustum.apply_jitter( jitter );
    EQ_GL_CALL( glFrustum( frustum.left(), frustum.right(),
                           frustum.bottom(), frustum.top(),
                           frustum.near_plane(), frustum.far_plane() )); 
    EQVERB << "Perspective " << frustum << std::endl;
}

void Channel::applyOrtho() const
{
    Frustumf ortho = getOrtho();
    const Vector2f jitter = getJitter();

    ortho.apply_jitter( jitter );
    EQ_GL_CALL( glOrtho( ortho.left(), ortho.right(),
                         ortho.bottom(), ortho.top(),
                         ortho.near_plane(), ortho.far_plane() )); 
    EQVERB << "Orthographic " << ortho << std::endl;
}

void Channel::applyScreenFrustum() const
{
    const Frustumf frustum = getScreenFrustum();
    EQ_GL_CALL( glOrtho( frustum.left(), frustum.right(),
                         frustum.bottom(), frustum.top(),
                         frustum.near_plane(), frustum.far_plane() ));
    EQVERB << "Apply " << frustum << std::endl;
}

void Channel::applyHeadTransform() const
{
    const Matrix4f& xfm = getHeadTransform();
    EQ_GL_CALL( glMultMatrixf( xfm.array ));
    EQVERB << "Apply head transform: " << xfm << std::endl;
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
            resize.dw = resize.w / static_cast< float >( _initialSize.x() );
            resize.dh = resize.h / static_cast< float >( _initialSize.y() );
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

    // find min/max time
    int64_t xMax = 0;
    int64_t xMin = std::numeric_limits< int64_t >::max();

    std::map< co::base::UUID, EntityData > entities;
    std::map< co::base::UUID, IdleData >   idles;

    for( std::vector< eq::FrameStatistics >::iterator i = statistics.begin();
         i != statistics.end(); ++i )
    {
        eq::FrameStatistics& frameStats  = *i;
        SortedStatistics& configStats = frameStats.second;

        for( SortedStatistics::iterator j = configStats.begin();
             j != configStats.end(); ++j )
        {
            const co::base::UUID& id = j->first;
            Statistics& stats = j->second;
            std::sort( stats.begin(), stats.end(), _compare );

            if( entities.find( id ) == entities.end( ))
            {
                EntityData& data = entities[ id ];
                data.name = stats.front().resourceName;
            }

            for( Statistics::const_iterator k = stats.begin(); 
                 k != stats.end(); ++k )
            {
                const Statistic& stat = *k;

                switch( stat.type )
                {
                case Statistic::PIPE_IDLE:
                {
                    IdleData& data = idles[ id ];
                    std::map< UUID, EntityData >::iterator l = 
                        entities.find( id );

                    if( l != entities.end( ))
                    {
                        entities.erase( l );
                        data.name = stat.resourceName;
                    }

                    data.idle += (stat.idleTime * 100ll / stat.totalTime);
                    ++data.nIdle;
                    continue;
                }
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
            const co::base::UUID&    id    = j->first;
            const Statistics& stats = j->second;

            if( stats.empty( ))
                continue;

            std::map< co::base::UUID, EntityData >::iterator l = entities.find( id );
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
                    // no break;
                case Statistic::CHANNEL_READBACK:
                    text << unsigned( 100.f * stat.ratio ) << '%';
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
                    glColor3f( 0.f, 0.f, 0.f );
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

    // Entitity names
    for( std::map< co::base::UUID, EntityData >::const_iterator i = entities.begin();
         i != entities.end(); ++i )
    {
        const EntityData& data = i->second;

        glColor3f( 1.f, 1.f, 1.f );
        glRasterPos3f( 60.f, data.yPos-SPACE-12.0f, 0.99f );
        font->draw( data.name );
    }

    // Global stats (scale, GPU idle)
    glColor3f( 1.f, 1.f, 1.f );
    nextY -= (HEIGHT + SPACE);
    glRasterPos3f( 60.f, static_cast< float >( nextY ), 0.99f );
    std::ostringstream text;
    text << scale << "ms/pixel";

    if( !idles.empty( ))
        text << ", Idle:";

    for( std::map< co::base::UUID, IdleData >::const_iterator i = idles.begin();
         i != idles.end(); ++i )
    {
        const IdleData& data = i->second;
        EQASSERT( data.nIdle > 0 );

        text << " " << data.name << ":" << data.idle / data.nIdle << "%";
    }

    font->draw( text.str( ));
    
    // Legend
    nextY -= SPACE;
    float x = 0.f;

    glRasterPos3f( x+1.f, nextY-12.f, 0.f );
    glColor3f( 1.f, 1.f, 1.f );
    font->draw( "channel" );

    for( size_t i = 1; i < Statistic::CONFIG_START_FRAME; ++i )
    {
        const Statistic::Type type = static_cast< Statistic::Type >( i );
        if( type == Statistic::CHANNEL_DRAW_FINISH ||
            type == Statistic::PIPE_IDLE )
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

void Channel::_unrefFrame( const uint32_t frameNumber, const uint32_t index )
{
    Channel::FrameStatistics& stats = _statistics[ index ];
    if( --stats.used != 0 ) // Frame still in use
        return;

    ChannelFrameFinishReplyPacket reply;
    reply.nStatistics = uint32_t( stats.data.size( ));
    reply.frameNumber = frameNumber;
    reply.objectID = getID();
    getServer()->send( reply, stats.data );
    stats.data.clear();
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
bool Channel::_cmdConfigInit( co::Command& command )
{
    const ChannelConfigInitPacket* packet = 
        command.getPacket<ChannelConfigInitPacket>();
    EQLOG( LOG_INIT ) << "TASK channel config init " << packet << std::endl;

    const Config* config = getConfig();
    changeLatency( config->getLatency( ));

    ChannelConfigInitReplyPacket reply;
    setError( ERROR_NONE );

    const Window* window = getWindow();
    if( window->isRunning( ))
    {
        _state = STATE_INITIALIZING;

        const PixelViewport& pvp = getPixelViewport();
        EQASSERT( pvp.hasArea( ));
        _initialSize.x() = pvp.w;
        _initialSize.y() = pvp.h;

        reply.result = configInit( packet->initID );

        if( reply.result )
        {
            _initDrawableConfig();
            _state = STATE_RUNNING;
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
        command.getPacket<ChannelConfigExitPacket>();
    EQLOG( LOG_INIT ) << "Exit channel " << packet << std::endl;

    if( _state != STATE_STOPPED )
        _state = configExit() ? STATE_STOPPED : STATE_FAILED;

    WindowDestroyChannelPacket destroyPacket( getID( ));
    getWindow()->send( getLocalNode(), destroyPacket );
    return true;
}

bool Channel::_cmdFrameStart( co::Command& command )
{
    ChannelFrameStartPacket* packet = 
        command.getPacket<ChannelFrameStartPacket>();
    EQVERB << "handle channel frame start " << packet << std::endl;

    //_grabFrame( packet->frameNumber ); single-threaded
    sync( packet->version );

    overrideContext( packet->context );
    bindFrameBuffer();
    frameStart( packet->context.frameID, packet->frameNumber );

    _statisticsIndex = ( _statisticsIndex + 1 ) % _statistics.size();
    EQASSERT( _statistics[ _statisticsIndex ].data.empty( ));
    EQASSERT( _statistics[ _statisticsIndex ].used == 0 );
    _statistics[ _statisticsIndex ].used = 1;
    resetContext();
    return true;
}

bool Channel::_cmdFrameFinish( co::Command& command )
{
    ChannelFrameFinishPacket* packet =
        command.getPacket<ChannelFrameFinishPacket>();
    EQLOG( LOG_TASKS ) << "TASK frame finish " << getName() <<  " " << packet
                       << std::endl;

    overrideContext( packet->context );
    frameFinish( packet->context.frameID, packet->frameNumber );
    resetContext();
    commit();

    _unrefFrame( packet->frameNumber, _statisticsIndex );
    return true;
}

bool Channel::_cmdFrameClear( co::Command& command )
{
    EQASSERT( _state == STATE_RUNNING );
    ChannelFrameClearPacket* packet = 
        command.getPacket<ChannelFrameClearPacket>();
    EQLOG( LOG_TASKS ) << "TASK clear " << getName() <<  " " << packet
                       << std::endl;

    _setRenderContext( packet->context );
    ChannelStatistics event( Statistic::CHANNEL_CLEAR, this );
    frameClear( packet->context.frameID );
    resetContext();

    return true;
}

bool Channel::_cmdFrameDraw( co::Command& command )
{
    ChannelFrameDrawPacket* packet = 
        command.getPacket<ChannelFrameDrawPacket>();
    EQLOG( LOG_TASKS ) << "TASK draw " << getName() <<  " " << packet
                       << std::endl;

    _setRenderContext( packet->context );
    ChannelStatistics event( Statistic::CHANNEL_DRAW, this );
    frameDraw( packet->context.frameID );
    resetContext();

    return true;
}

bool Channel::_cmdFrameDrawFinish( co::Command& command )
{
    ChannelFrameDrawFinishPacket* packet = 
        command.getPacket< ChannelFrameDrawFinishPacket >();
    EQLOG( LOG_TASKS ) << "TASK draw finish " << getName() <<  " " << packet
                       << std::endl;

    ChannelStatistics event( Statistic::CHANNEL_DRAW_FINISH, this );
    frameDrawFinish( packet->frameID, packet->frameNumber );

    return true;
}

bool Channel::_cmdFrameAssemble( co::Command& command )
{
    ChannelFrameAssemblePacket* packet = 
        command.getPacket<ChannelFrameAssemblePacket>();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK assemble " << getName() <<  " " 
                                       << packet << std::endl;

    _setRenderContext( packet->context );
    ChannelStatistics event( Statistic::CHANNEL_ASSEMBLE, this );

    for( uint32_t i=0; i<packet->nFrames; ++i )
    {
        Pipe*  pipe  = getPipe();
        Frame* frame = pipe->getFrame( packet->frames[i], getEye(), false );
        _inputFrames.push_back( frame );
    }

    frameAssemble( packet->context.frameID );

    for( Frames::const_iterator i = _inputFrames.begin();
         i != _inputFrames.end(); ++i )
    {
        // Unset the frame data on input frames, so that they only get flushed
        // once by the output frames during exit.
        (*i)->setData( 0 );
    }
    _inputFrames.clear();
    resetContext();

    return true;
}

bool Channel::_cmdFrameReadback( co::Command& command )
{
    ChannelFrameReadbackPacket* packet = 
        command.getPacket<ChannelFrameReadbackPacket>();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK readback " << getName() <<  " " 
                                       << packet << std::endl;

    _setRenderContext( packet->context );
    ChannelStatistics event( Statistic::CHANNEL_READBACK, this );

    for( uint32_t i=0; i<packet->nFrames; ++i )
    {
        Pipe*  pipe  = getPipe();
        Frame* frame = pipe->getFrame( packet->frames[i], getEye(), true );
        _outputFrames.push_back( frame );
    }

    frameReadback( packet->context.frameID );

    size_t in = 0;
    size_t out = 0;
    const DrawableConfig& dc = getDrawableConfig();
    const size_t colorBytes = ( 3 * dc.colorBits + dc.alphaBits ) / 8;
    for( Frames::const_iterator i = _outputFrames.begin(); 
         i != _outputFrames.end(); ++i)
    {
        Frame* frame = *i;
        frame->setReady();

        const Images& images = frame->getImages();
        for( Images::const_iterator j = images.begin(); j != images.end(); ++j )
        {
            const Image* image = *j;
            if( image->hasPixelData( Frame::BUFFER_COLOR ))
            {
                in += colorBytes * image->getPixelViewport().getArea();
                out += image->getPixelDataSize( Frame::BUFFER_COLOR );
            }
            if( image->hasPixelData( Frame::BUFFER_DEPTH ))
            {
                in += 4 * image->getPixelViewport().getArea();
                out += image->getPixelDataSize( Frame::BUFFER_DEPTH );
            }
        }
    }

    if( in > 0 && out > 0 )
        event.event.data.statistic.ratio = float( out ) / float( in );
    else
        event.event.data.statistic.ratio = 1.0f;

    _outputFrames.clear();
    resetContext();
    return true;
}

bool Channel::_cmdFrameTransmit( co::Command& command )
{
    ChannelFrameTransmitPacket* packet = 
        command.getPacket<ChannelFrameTransmitPacket>();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK transmit " << getName() <<  " " 
                                      << packet << std::endl;

    ++_statistics[ _statisticsIndex ].used;

    packet->command = fabric::CMD_CHANNEL_FRAME_TRANSMIT_ASYNC;
    packet->statisticsIndex = _statisticsIndex;
    packet->frameNumber = getPipe()->getCurrentFrame();
    dispatchCommand( command );
    return true;
}

bool Channel::_cmdFrameTransmitAsync( co::Command& command )
{
    const ChannelFrameTransmitPacket* packet = 
        command.getPacket<ChannelFrameTransmitPacket>();

    _transmit( packet );
    _unrefFrame( packet->frameNumber, packet->statisticsIndex );
    return true;
}

void Channel::_transmit( const ChannelFrameTransmitPacket* command )
{
    ChannelStatistics transmitEvent( Statistic::CHANNEL_FRAME_TRANSMIT, this );
    transmitEvent.statisticsIndex = command->statisticsIndex;
    transmitEvent.event.data.statistic.task = command->context.taskID;

    FrameData* frameData = getNode()->getFrameData( command->frameData ); 
    EQASSERT( frameData );
    frameData->setSendToken( getIAttribute( IATTR_HINT_SENDTOKEN ) == ON );

    if( frameData->getBuffers() == 0 )
    {
        EQWARN << "No buffers for frame data" << std::endl;
        return;
    }

    co::LocalNodePtr localNode = getLocalNode();
    co::NodePtr toNode = localNode->connect( command->netNodeID );
    co::ConnectionPtr connection = toNode->getConnection();
    co::ConnectionDescriptionPtr description = connection->getDescription();

    // use compression on links up to 2 GBit/s
    const bool useCompression = ( description->bandwidth <= 262144 );

    NodeFrameDataTransmitPacket packet;
    const uint64_t packetSize = sizeof( packet ) - 8 * sizeof( uint8_t );

    packet.objectID    = command->clientNodeID;
    packet.frameData   = frameData;
    packet.frameNumber = command->frameNumber;

    const Images& images = frameData->getImages();
    // send all images
    for( Images::const_iterator i = images.begin(); i != images.end(); ++i )
    {
        Image* image = *i;
        if ( image->getStorageType() == Frame::TYPE_TEXTURE )
        {
            EQWARN << "Can't transmit image of type TEXTURE" << std::endl;
            EQUNIMPLEMENTED;
            return;
        }

        std::vector< const PixelData* > pixelDatas;
        std::vector< float > qualities;

        packet.size = packetSize;
        packet.buffers = Frame::BUFFER_NONE;
        packet.pvp = image->getPixelViewport();
        packet.useAlpha = image->getAlphaUsage();
        EQASSERT( packet.pvp.isValid( ));

        {
            uint64_t rawSize( 0 );
            ChannelStatistics compressEvent( Statistic::CHANNEL_FRAME_COMPRESS, 
                                             this );
            compressEvent.statisticsIndex = command->statisticsIndex;
            compressEvent.event.data.statistic.task = command->context.taskID;
            compressEvent.event.data.statistic.ratio = 1.0f;
            if( !useCompression ) // don't send event
                compressEvent.event.data.statistic.frameNumber = 0;

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
            continue;

        // send image pixel data packet
        if( frameData->getSendToken() )
        {
            ChannelStatistics waitEvent(Statistic::CHANNEL_FRAME_WAIT_SENDTOKEN,
                                        this );
            waitEvent.statisticsIndex = command->statisticsIndex;
            waitEvent.event.data.statistic.task = command->context.taskID;
            getLocalNode()->acquireSendToken( toNode );
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
                    data->compressorName, data->compressorFlags,
                    data->isCompressed ?
                        uint32_t( data->compressedSize.size( )) : 1,
                    qualities[ j ] };

            connection->send( &header, sizeof( FrameData::ImageHeader ), true );
            
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
        if( frameData->getSendToken() )
            getLocalNode()->releaseSendToken( toNode );
    }

    // all data transmitted -> ready
    NodeFrameDataReadyPacket readyPacket( frameData );
    readyPacket.objectID  = command->clientNodeID;
    toNode->send( readyPacket );
}

bool Channel::_cmdFrameViewStart( co::Command& command )
{
    ChannelFrameViewStartPacket* packet = 
        command.getPacket<ChannelFrameViewStartPacket>();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK view start " << getName() <<  " "
                                       << packet << std::endl;

    _setRenderContext( packet->context );
    frameViewStart( packet->context.frameID );
    resetContext();

    return true;
}

bool Channel::_cmdFrameViewFinish( co::Command& command )
{
    ChannelFrameViewFinishPacket* packet = 
        command.getPacket<ChannelFrameViewFinishPacket>();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK view finish " << getName()
                                      <<  " " << packet << std::endl;

    _setRenderContext( packet->context );
    ChannelStatistics event( Statistic::CHANNEL_VIEW_FINISH, this );
    frameViewFinish( packet->context.frameID );
    resetContext();

    return true;
}

bool Channel::_cmdStopFrame( co::Command& command )
{
    ChannelStopFramePacket* packet = 
        command.getPacket<ChannelStopFramePacket>();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK channel stop frame " << getName()
                                      <<  " " << packet << std::endl;

    notifyStopFrame( packet->lastFrameNumber );
    return true;
}
}

#include "../fabric/channel.ipp"
template class eq::fabric::Channel< eq::Window, eq::Channel >;
/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                                                 const eq::Super& );
/** @endcond */

