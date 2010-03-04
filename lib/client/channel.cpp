
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include "channelVisitor.h"
#include "client.h"
#include "compositor.h"
#include "commands.h"
#include "config.h"
#include "configEvent.h"
#include "frame.h"
#include "global.h"
#include "jitter.h"
#include "log.h"
#include "node.h"
#include "nodeFactory.h"
#include "osWindow.h"
#include "packets.h"
#include "pipe.h"
#include "server.h"
#include "task.h"
#include "view.h"

#include <eq/util/accum.h>
#include <eq/util/frameBufferObject.h>
#include <eq/net/command.h>
#include <eq/base/rng.h>

namespace eq
{
#define MAKE_ATTR_STRING( attr ) ( std::string("EQ_CHANNEL_") + #attr )
std::string Channel::_iAttributeStrings[IATTR_ALL] = {
    MAKE_ATTR_STRING( IATTR_HINT_STATISTICS ),
    MAKE_ATTR_STRING( IATTR_HINT_SENDTOKEN ),
    MAKE_ATTR_STRING( IATTR_FILL1 ),
    MAKE_ATTR_STRING( IATTR_FILL2 )
};

Channel::Channel( Window* parent )
        : fabric::Channel< Channel, Window >( parent )
        , _context( &_nativeContext )
        , _tasks( TASK_NONE )
        , _state( STATE_STOPPED )
        , _fixedPVP( false )
        , _fbo(0)
        , _drawable( 0 )
        , _initialSize( Vector2i::ZERO )
        , _maxSize( Vector2i::ZERO )
{
    EQINFO << " New eq::Channel @" << (void*)this << std::endl;
}

Channel::~Channel()
{  
    EQINFO << " Delete eq::Channel @" << (void*)this << std::endl;
}

/** @cond IGNORE */
typedef net::CommandFunc<Channel> CmdFunc;
/** @endcond */

void Channel::attachToSession( const uint32_t id, 
                               const uint32_t instanceID, 
                               net::Session* session )
{
    net::Object::attachToSession( id, instanceID, session );
    net::CommandQueue* queue = _window->getPipeThreadQueue();

    registerCommand( CMD_CHANNEL_CONFIG_INIT, 
                     CmdFunc( this, &Channel::_cmdConfigInit ), queue );
    registerCommand( CMD_CHANNEL_CONFIG_EXIT, 
                     CmdFunc( this, &Channel::_cmdConfigExit ), queue );
    registerCommand( CMD_CHANNEL_FRAME_START,
                     CmdFunc( this, &Channel::_cmdFrameStart ), queue );
    registerCommand( CMD_CHANNEL_FRAME_FINISH,
                     CmdFunc( this, &Channel::_cmdFrameFinish ), queue );
    registerCommand( CMD_CHANNEL_FRAME_CLEAR, 
                     CmdFunc( this, &Channel::_cmdFrameClear ), queue );
    registerCommand( CMD_CHANNEL_FRAME_DRAW, 
                     CmdFunc( this, &Channel::_cmdFrameDraw ), queue );
    registerCommand( CMD_CHANNEL_FRAME_DRAW_FINISH, 
                    CmdFunc( this, &Channel::_cmdFrameDrawFinish ), queue );
    registerCommand( CMD_CHANNEL_FRAME_ASSEMBLE, 
                     CmdFunc( this, &Channel::_cmdFrameAssemble ), queue );
    registerCommand( CMD_CHANNEL_FRAME_READBACK, 
                     CmdFunc( this, &Channel::_cmdFrameReadback ), queue );
    registerCommand( CMD_CHANNEL_FRAME_TRANSMIT, 
                     CmdFunc( this, &Channel::_cmdFrameTransmit ), queue );
    registerCommand( CMD_CHANNEL_FRAME_VIEW_START, 
                     CmdFunc( this, &Channel::_cmdFrameViewStart ), queue );
    registerCommand( CMD_CHANNEL_FRAME_VIEW_FINISH, 
                     CmdFunc( this, &Channel::_cmdFrameViewFinish ), queue );
}

Pipe* Channel::getPipe()
{
    EQASSERT( _window );
    return ( _window ? _window->getPipe() : 0 );
}
const Pipe* Channel::getPipe() const
{
    EQASSERT( _window );
    return ( _window ? _window->getPipe() : 0 );
}

Node* Channel::getNode()
{
    EQASSERT( _window );
    return ( _window ? _window->getNode() : 0 );
}
const Node* Channel::getNode() const
{
    EQASSERT( _window );
    return ( _window ? _window->getNode() : 0 );
}

Config* Channel::getConfig()
{
    EQASSERT( _window );
    return ( _window ? _window->getConfig() : 0 );
}
const Config* Channel::getConfig() const
{
    EQASSERT( _window );
    return ( _window ? _window->getConfig() : 0 );
}

ServerPtr Channel::getServer()
{
    EQASSERT( _window );
    return ( _window ? _window->getServer() : 0 );
}

Window::ObjectManager* Channel::getObjectManager()
{
    EQASSERT( _window );
    return _window->getObjectManager();
}

const DrawableConfig& Channel::getDrawableConfig() const
{
    EQASSERT( _window );
    if( _fbo )
        return _drawableConfig;

    return _window->getDrawableConfig();
}

GLEWContext* Channel::glewGetContext()
{
    EQASSERT( _window );
    return _window->glewGetContext();
}
const GLEWContext* Channel::glewGetContext() const
{
    EQASSERT( _window );
    return _window->glewGetContext();
}

bool Channel::configExit()
{
    delete _fbo;
    _fbo = 0;
    return true;
}
bool Channel::configInit( const uint32_t initID )
{ 
    return _configInitFBO(); 
}

bool Channel::_configInitFBO()
{
    if( _drawable == FB_WINDOW )
        return true;
    
    if( !_window->getOSWindow()  ||
        !GLEW_ARB_texture_non_power_of_two || !GLEW_EXT_framebuffer_object )
    {
        setErrorMessage( "Can't use FBO due to missing GL extensions" );
        return false;
    }
        
    // needs glew initialized (see above)
    _fbo = new util::FrameBufferObject( glewGetContext( ));
    _fbo->setColorFormat( _window->getColorFormat( ));
        
    int depthSize = 0;
    if( _drawable & FBO_DEPTH )
    {
        depthSize = _window->getIAttribute( Window::IATTR_PLANES_DEPTH );

        if( depthSize < 1 )
            depthSize = 24;
    }

    int stencilSize = 0;
    if( _drawable & FBO_STENCIL )
    {
        stencilSize = _window->getIAttribute( Window::IATTR_PLANES_STENCIL );

        if( stencilSize < 1 )
            stencilSize = 1;
    }

    const PixelViewport& pvp = _nativeContext.pvp;

    if( _fbo->init( pvp.w, pvp.h, depthSize, stencilSize ))
        return true;
    // else

    setErrorMessage( "FBO initialization failed: " + _fbo->getErrorMessage( ));
    delete _fbo;
    _fbo = 0;
    return false;
}

void Channel::_initDrawableConfig()
{
    _drawableConfig = _window->getDrawableConfig();
    if( !_fbo )
        return;

    const util::TextureVector& colors = _fbo->getColorTextures();
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
//----------------------------------------------------------------------
// viewport
//----------------------------------------------------------------------
void Channel::_setPixelViewport( const PixelViewport& pvp )
{
    EQASSERT( pvp.hasArea( ));
    if( !pvp.hasArea( ))
        return;

    _fixedPVP = true;

    if( _nativeContext.pvp == pvp && _nativeContext.vp.hasArea( ))
        return;

    _nativeContext.pvp = pvp;
    _nativeContext.vp.invalidate();

    if( !_window )
        return;
    
    const PixelViewport& windowPVP = _window->getPixelViewport();
    if( windowPVP.isValid( ))
        _nativeContext.vp = pvp.getSubVP( windowPVP );

    EQVERB << "Channel pvp set: " << _nativeContext.pvp << ":" 
           << _nativeContext.vp << std::endl;
}

void Channel::_setViewport( const Viewport& vp )
{
    if( !vp.hasArea( ))
        return;
    
    _fixedPVP = false;

    if( _nativeContext.vp == vp && _nativeContext.pvp.hasArea( ))
        return;

    _nativeContext.vp = vp;
    _nativeContext.pvp.invalidate();

    if( !_window )
        return;

    PixelViewport windowPVP = _window->getPixelViewport();
    if( windowPVP.isValid( ))
    {
        windowPVP.x = 0;
        windowPVP.y = 0;
        _nativeContext.pvp = windowPVP.getSubPVP( vp );

        // send event
        Event event;
        event.type       = Event::CHANNEL_RESIZE;
        event.originator = getID();
        event.resize.x   = _nativeContext.pvp.x;
        event.resize.y   = _nativeContext.pvp.y;
        event.resize.w   = _nativeContext.pvp.w;
        event.resize.h   = _nativeContext.pvp.h;

        processEvent( event );
    }

    EQVERB << "Channel vp set: " << _nativeContext.pvp << ":" 
           << _nativeContext.vp << std::endl;
}

void Channel::_notifyViewportChanged()
{
    if( !_window )
        return;

    eq::PixelViewport windowPVP = _window->getPixelViewport();
    if( !windowPVP.isValid( ))
        return;

    windowPVP.x = 0;
    windowPVP.y = 0;

    if( _fixedPVP ) // update viewport
        _nativeContext.vp = _nativeContext.pvp.getSubVP( windowPVP );
    else            // update pixel viewport
    {
        const eq::PixelViewport pvp = windowPVP.getSubPVP( _nativeContext.vp );
        if( _nativeContext.pvp == pvp )
            return;

        _nativeContext.pvp = pvp;

        // send event
        Event event;
        event.type       = Event::CHANNEL_RESIZE;
        event.originator = getID();
        event.resize.x   = _nativeContext.pvp.x;
        event.resize.y   = _nativeContext.pvp.y;
        event.resize.w   = _nativeContext.pvp.w;
        event.resize.h   = _nativeContext.pvp.h;

        processEvent( event );
    }

    EQINFO << "Channel viewport update: " << _nativeContext.pvp << ":" 
           << _nativeContext.vp << std::endl;
}

void Channel::setNearFar( const float nearPlane, const float farPlane )
{
    if( _context->frustum.near_plane() == nearPlane && 
        _context->frustum.far_plane() == farPlane )
    {
        return;
    }

    _nativeContext.frustum.adjust_near( nearPlane );
    _nativeContext.frustum.far_plane() = farPlane;
    _nativeContext.ortho.near_plane()  = nearPlane;
    _nativeContext.ortho.far_plane()   = farPlane;

    if( _context != &_nativeContext )
    {
        _context->frustum.adjust_near( nearPlane );
        _context->frustum.far_plane() = farPlane;
        _context->ortho.near_plane() = nearPlane;
        _context->ortho.far_plane()  = farPlane;
    }

    ChannelSetNearFarPacket packet;
    packet.nearPlane = nearPlane;
    packet.farPlane  = farPlane;
    
    ServerPtr server = getServer();
    net::NodePtr node = server.get();
    send( node, packet );
}

void Channel::addStatistic( Event& event )
{
    _statistics.push_back( event.statistic );
    processEvent( event );
}

//---------------------------------------------------------------------------
// operations
//---------------------------------------------------------------------------

void Channel::frameClear( const uint32_t frameID )
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

void Channel::frameDraw( const uint32_t frameID )
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

void Channel::frameAssemble( const uint32_t frameID )
{
    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));
    EQ_GL_CALL( setupAssemblyState( ));

    Compositor::assembleFrames( getInputFrames(), this, 0 );

    EQ_GL_CALL( resetAssemblyState( ));
}

void Channel::frameReadback( const uint32_t frameID )
{
    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));
    EQ_GL_CALL( setupAssemblyState( ));

    Window::ObjectManager* glObjects = getObjectManager();
    const DrawableConfig& drawableConfig = getDrawableConfig();

    const FrameVector& frames = getOutputFrames();
    for( FrameVector::const_iterator i = frames.begin(); i != frames.end(); ++i)
    {
        Frame* frame = *i;
        frame->startReadback( glObjects, drawableConfig );
    }
    for( FrameVector::const_iterator i = frames.begin(); i != frames.end(); ++i)
    {
        Frame* frame = *i;
        frame->syncReadback();
    }

    EQ_GL_CALL( resetAssemblyState( ));
}

void Channel::setupAssemblyState()
{
    // copy to be thread-safe when pvp changes
    const PixelViewport pvp( getPixelViewport( ));
    Compositor::setupAssemblyState( pvp );
}

void Channel::resetAssemblyState()
{
    Compositor::resetAssemblyState();
}

void Channel::setErrorMessage( const std::string& message )
{
    _error = message;
}

void Channel::_setRenderContext( RenderContext& context )
{
    _context = &context;
    _window->_addRenderContext( context );
}

const Viewport& Channel::getViewport() const
{
    return _context->vp;
}

const PixelViewport& Channel::getPixelViewport() const
{
    return _context->pvp;
}

const Vector2i& Channel::getPixelOffset() const
{
    return _context->offset;
}

uint32_t Channel::getDrawBuffer() const
{
    return _context->buffer;
}

uint32_t Channel::getReadBuffer() const
{
    return _context->buffer;
}

const ColorMask& Channel::getDrawBufferMask() const
{
    return _context->bufferMask;
}

const Frustumf& Channel::getFrustum() const
{
    return _context->frustum;
}

const Frustumf& Channel::getOrtho() const
{
    return _context->ortho;
}

const Range& Channel::getRange() const
{
    return _context->range;
}

const Pixel& Channel::getPixel() const
{
    return _context->pixel;
}

const SubPixel& Channel::getSubPixel() const
{
    return _context->subpixel;
}

const Zoom& Channel::getZoom() const
{
    return _context->zoom;
}

uint32_t Channel::getPeriod() const
{
    return _context->period;
}

uint32_t Channel::getPhase() const
{
    return _context->phase;
}

Eye Channel::getEye() const
{
    return _context->eye;
}

const Matrix4f& Channel::getHeadTransform() const
{
    return _context->headTransform;
}

Frustumf Channel::getScreenFrustum() const
{
    const Pixel& pixel = getPixel();
    PixelViewport pvp( getPixelViewport( ));
    const Viewport& vp( getViewport( ));

    pvp.x = static_cast<int32_t>( pvp.w / vp.w * vp.x );
    pvp.y = static_cast<int32_t>( pvp.h / vp.h * vp.y );
    pvp *= pixel;

    return eq::Frustumf( static_cast< float >( pvp.x ),
                         static_cast< float >( pvp.getXEnd( )),
                         static_cast< float >( pvp.y ),
                         static_cast< float >( pvp.getYEnd( )),
                         -1.f, 1.f );
}

const Vector4i& Channel::getOverdraw() const
{
    return _context->overdraw;
}

void Channel::setMaxSize( const Vector2i& size )
{
    _maxSize = size;
}


uint32_t Channel::getTaskID() const
{
    return _context->taskID;
}

util::FrameBufferObject* Channel::getFrameBufferObject()
{
    return _fbo;
}

View* Channel::getView()
{
    Pipe* pipe = getPipe();
    return pipe->getView( _context->view );
}

const View* Channel::getView() const
{
    const Pipe* pipe = getPipe();
    return pipe->getView( _context->view );
}

View* Channel::getNativeView()
{
    Pipe* pipe = getPipe();
    return pipe->getView( _nativeContext.view );
}

const View* Channel::getNativeView() const
{
    const Pipe* pipe = getPipe();
    return pipe->getView( _nativeContext.view );
}

//---------------------------------------------------------------------------
// apply convenience methods
//---------------------------------------------------------------------------

void Channel::applyFrameBufferObject()
{
    if( _fbo )
    {
        _fbo->resize( _nativeContext.pvp.w, _nativeContext.pvp.h );
        _fbo->bind(); 
    }
    else if( GLEW_EXT_framebuffer_object )
        glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
}

void Channel::applyBuffer()
{
    if( !_fbo && _window->getOSWindow()->getFrameBufferObject() == 0 )
    {
        EQ_GL_CALL( glReadBuffer( getReadBuffer( )));
        EQ_GL_CALL( glDrawBuffer( getDrawBuffer( )));
    }
    
    applyColorMask();
}

void Channel::bindFrameBuffer()
{
   if( !_window->getOSWindow() )
       return;
        
   if( _fbo )
       applyFrameBufferObject();
   else
       _window->bindFrameBuffer();
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
        static base::RNG rng;
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
        case Event::STATISTIC:
            break;

        case Event::CHANNEL_RESIZE:
        {
            const uint32_t viewID = _nativeContext.view.identifier;
            if( viewID == EQ_ID_INVALID )
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
    EntityData() : yPos( 0 ) {}
    uint32_t yPos;
    std::string name;
};

struct IdleData
{
    IdleData() : idle( 0 ), nIdle( 0 ) {}
    uint32_t idle;
    uint32_t nIdle;
    std::string name;
};

}

void Channel::drawStatistics()
{
    const PixelViewport& pvp = getPixelViewport();
    EQASSERT( pvp.hasArea( ));
    if( !pvp.hasArea( ))
        return;

    Config* config = getConfig();
    EQASSERT( config );

    std::vector< FrameStatistics > statistics;
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
    glClear( GL_DEPTH_BUFFER_BIT );
    glEnable( GL_DEPTH_TEST );
    
    const Window::Font* font = _window->getSmallFont();

    // find min/max time
    int64_t                 xMax      = 0;
    int64_t                 xMin      = std::numeric_limits< int64_t >::max();

    for( std::vector< FrameStatistics >::const_iterator i = statistics.begin();
         i != statistics.end(); ++i )
    {
        const FrameStatistics&  frameStats  = *i;
        const SortedStatistics& configStats = frameStats.second;

        for( SortedStatistics::const_iterator j = configStats.begin();
             j != configStats.end(); ++j )
        {
            const Statistics& stats = j->second;

            for( Statistics::const_iterator k = stats.begin(); 
                 k != stats.end(); ++k )
            {
                const Statistic& stat = *k;
                if( stat.type == Statistic::PIPE_IDLE )
                    continue;

                xMax = EQ_MAX( xMax, stat.endTime );
                xMin = EQ_MIN( xMin, stat.endTime );
            }
        }
    }
    uint32_t scale = 1;
    const Viewport& vp = getViewport();
    const uint32_t width = static_cast< uint32_t >( getPixelViewport().w/vp.w );
    while( (xMax - xMin) / scale > width )
        scale *= 10;

    xMax  /= scale;
    int64_t xStart = xMax - width + SPACE;

    const uint32_t height = static_cast< uint32_t >( getPixelViewport().h/vp.h);
    uint32_t nextY = height - SPACE;

    std::map< uint32_t, EntityData > entities;
    std::map< uint32_t, IdleData >   idles;

    float dim = 0.0f;
    for( std::vector< FrameStatistics >::reverse_iterator i=statistics.rbegin();
         i != statistics.rend(); ++i )
    {
        const FrameStatistics&  frameStats  = *i;
        const SortedStatistics& configStats = frameStats.second;

        int64_t     frameMin = xMax;
        int64_t     frameMax = 0;

        // draw stats
        for( SortedStatistics::const_iterator j = configStats.begin();
             j != configStats.end(); ++j )
        {
            const uint32_t    id    = j->first;
            const Statistics& stats = j->second;

            if( stats.empty( ))
                continue;

            if( entities.find( id ) == entities.end( ))
            {
                EntityData& data = entities[ id ];
                data.yPos = nextY;
                data.name = stats.front().resourceName;

                nextY -= (HEIGHT + SPACE);
            }

            const uint32_t y = entities[ id ].yPos;

            for( Statistics::const_iterator k = stats.begin(); 
                 k != stats.end(); ++k )
            {
                const Statistic& stat = *k;

                if( stat.type == Statistic::PIPE_IDLE )
                {
                    IdleData& data = idles[ id ];
                    std::map< uint32_t, EntityData >::iterator l = 
                        entities.find( id );

                    if( l != entities.end( ))
                    {
                        entities.erase( l );
                        nextY += (HEIGHT + SPACE);
                        data.name = stat.resourceName;
                    }
                    
                    data.idle += (stat.idleTime * 100ll / stat.totalTime);
                    ++data.nIdle;
                    continue;
                }

                const int64_t startTime = stat.startTime / scale;
                const int64_t endTime   = stat.endTime   / scale;

                frameMin = EQ_MIN( frameMin, startTime );
                frameMax = EQ_MAX( frameMax, endTime   );

                if( endTime < xStart || endTime == startTime )
                    continue;

                float y1 = static_cast< float >( y );
                float y2 = static_cast< float >( y - HEIGHT );
                float z  = 0.0f;
                const float x1 = static_cast< float >( startTime - xStart );
                const float x2 = static_cast< float >( endTime   - xStart );
                
                switch( stat.type )
                {
                    case Statistic::FRAME_COMPRESS:
                    {
                        z = 0.7f; 
                        
                        glColor3f( 0.f, 0.f, 0.f );
                        std::stringstream text;
                        text << static_cast< unsigned >( 100.f * stat.ratio ) 
                             << '%';
                        glRasterPos3f( x1+1, y2, 0.99f );

                        font->draw( text.str( ));
                        break;
                    }

                    case Statistic::FRAME_TRANSMIT:
                    case Statistic::FRAME_RECEIVE:
                        z = 0.5f; 
                        break;

                    case Statistic::CHANNEL_WAIT_FRAME:
                    case Statistic::CONFIG_WAIT_FINISH_FRAME:
                        y1 -= SPACE;
                        y2 += SPACE;
                        // no break;
                    case Statistic::CONFIG_START_FRAME:
                        z = 0.1f; 
                        break;

                    default:
                        z = 0.0f; 
                        break;
                }
                
                const Vector3f color( Statistic::getColor( stat.type ) - dim );
                glColor3fv( color.array );

                glBegin( GL_QUADS );
                glVertex3f( x2, y1, z );
                glVertex3f( x1, y1, z );
                glVertex3f( x1, y2, z );
                glVertex3f( x2, y2, z );
                glEnd();
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
    for( std::map< uint32_t, EntityData >::const_iterator i = entities.begin();
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

    for( std::map< uint32_t, IdleData >::const_iterator i = idles.begin();
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
    float z = 0.f;

    glRasterPos3f( x+1.f, nextY-12.f, z );
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

        if( type == Statistic::WINDOW_FINISH )
        {
            x = 0.f;
            nextY -= (HEIGHT + SPACE);
            z = 0.f;

            glColor3f( 1.f, 1.f, 1.f );
            glRasterPos3f( x+1.f, nextY-12.f, z );
            font->draw( "window" );
        }
        else if( type == Statistic::FRAME_TRANSMIT )
        {
            x = 0.f;
            nextY -= (HEIGHT + SPACE);
            z = 0.f;

            glColor3f( 1.f, 1.f, 1.f );
            glRasterPos3f( x+1.f, nextY-12.f, z );
            font->draw( "frame" );
        }

        x += 60.f;
        const float x2 = x + 60.f - SPACE; 
        const float y1 = static_cast< float >( nextY );
        const float y2 = static_cast< float >( nextY - HEIGHT );

        z += 0.01f;
        glColor3fv( Statistic::getColor( type ).array );
        glBegin( GL_QUADS );
        glVertex3f( x2, y1, z );
        glVertex3f( x,  y1, z );
        glVertex3f( x,  y2, z );
        glVertex3f( x2, y2, z );
        glEnd();

        z += 0.01f;
        glColor3f( 0.f, 0.f, 0.f );
        glRasterPos3f( x+1.f, nextY-12.f, z );
        font->draw( Statistic::getName( type ));
    }
    // nextY -= (HEIGHT + SPACE);
    
    glColor3f( 1.f, 1.f, 1.f );
    _window->drawFPS();
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

int32_t Channel::getIAttribute( const IAttribute attr ) const
{
    return _iAttributes[attr];
}

const std::string& Channel::getIAttributeString( const IAttribute attr )
{
    return _iAttributeStrings[attr];
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
net::CommandResult Channel::_cmdConfigInit( net::Command& command )
{
    const ChannelConfigInitPacket* packet = 
        command.getPacket<ChannelConfigInitPacket>();
    EQLOG( LOG_INIT ) << "TASK channel config init " << packet << std::endl;

    ChannelConfigInitReplyPacket reply;
    _error.clear();

    if( _window->isRunning( ))
    {
        _state = STATE_INITIALIZING;
        if( packet->pvp.isValid( ))
            _setPixelViewport( packet->pvp );
        else
            _setViewport( packet->vp );
        
        setName( packet->name );
        _tasks    = packet->tasks;
        _color    = packet->color;
        _drawable = packet->drawable;
        _nativeContext.view = packet->view;
        _initialSize.x() = _nativeContext.pvp.w;
        _initialSize.y() = _nativeContext.pvp.h;

        memcpy( _iAttributes, packet->iAttributes, IATTR_ALL * sizeof(int32_t));

        reply.result = configInit( packet->initID );

        reply.nearPlane   = _nativeContext.frustum.near_plane();
        reply.farPlane    = _nativeContext.frustum.far_plane();
        reply.maxSize     = _maxSize;

        if( reply.result )
        {
            _initDrawableConfig();
            _state = STATE_RUNNING;
        }
    }
    else
        reply.result = false;

    EQLOG( LOG_INIT ) << "TASK channel config init reply " << &reply << std::endl;
    send( command.getNode(), reply, _error );
    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdConfigExit( net::Command& command )
{
    const ChannelConfigExitPacket* packet =
        command.getPacket<ChannelConfigExitPacket>();
    EQLOG( LOG_INIT ) << "Exit channel " << packet << std::endl;

    ChannelConfigExitReplyPacket reply;
    if( _state == STATE_STOPPED )
        reply.result = true;
    else
        reply.result = configExit();

    send( command.getNode(), reply );
    _state = STATE_STOPPED;
    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdFrameStart( net::Command& command )
{
    ChannelFrameStartPacket* packet = 
        command.getPacket<ChannelFrameStartPacket>();
    EQVERB << "handle channel frame start " << packet << std::endl;

    //_grabFrame( packet->frameNumber ); single-threaded
    _nativeContext.view     = packet->context.view;
    _nativeContext.overdraw = packet->context.overdraw;

    _context = &packet->context;
    bindFrameBuffer();
    frameStart( packet->context.frameID, packet->frameNumber );
    _context = &_nativeContext;

    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdFrameFinish( net::Command& command )
{
    ChannelFrameFinishPacket* packet =
        command.getPacket<ChannelFrameFinishPacket>();
    EQLOG( LOG_TASKS ) << "TASK frame finish " << getName() <<  " " << packet
                       << std::endl;

    _context = &packet->context;
    frameFinish( packet->context.frameID, packet->frameNumber );
    _context = &_nativeContext;

    ChannelFrameFinishReplyPacket reply( packet );
    reply.nStatistics = _statistics.size();

    command.getNode()->send( reply, _statistics );

    _statistics.clear();
    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdFrameClear( net::Command& command )
{
    ChannelFrameClearPacket* packet = 
        command.getPacket<ChannelFrameClearPacket>();
    EQLOG( LOG_TASKS ) << "TASK clear " << getName() <<  " " << packet << std::endl;

    _setRenderContext( packet->context );
    ChannelStatistics event( Statistic::CHANNEL_CLEAR, this );
    frameClear( packet->context.frameID );
    _context = &_nativeContext;

    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdFrameDraw( net::Command& command )
{
    ChannelFrameDrawPacket* packet = 
        command.getPacket<ChannelFrameDrawPacket>();
    EQLOG( LOG_TASKS ) << "TASK draw " << getName() <<  " " << packet << std::endl;

    _setRenderContext( packet->context );
    ChannelStatistics event( Statistic::CHANNEL_DRAW, this );
    frameDraw( packet->context.frameID );
    _context = &_nativeContext;

    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdFrameDrawFinish( net::Command& command )
{
    ChannelFrameDrawFinishPacket* packet = 
        command.getPacket< ChannelFrameDrawFinishPacket >();
    EQLOG( LOG_TASKS ) << "TASK draw finish " << getName() <<  " " << packet
                       << std::endl;

    ChannelStatistics event( Statistic::CHANNEL_DRAW_FINISH, this );
    frameDrawFinish( packet->frameID, packet->frameNumber );

    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdFrameAssemble( net::Command& command )
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
        Frame* frame = pipe->getFrame( packet->frames[i], getEye( ));
        _inputFrames.push_back( frame );
    }

    frameAssemble( packet->context.frameID );

    for( FrameVector::const_iterator i = _inputFrames.begin();
         i != _inputFrames.end(); ++i )
    {
        // Unset the frame data on input frames, so that they only get flushed
        // once by the output frames during exit.
        (*i)->setData( 0 );
    }
    _inputFrames.clear();
    _context = &_nativeContext;

    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdFrameReadback( net::Command& command )
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
        Frame* frame = pipe->getFrame( packet->frames[i], getEye( ));
        _outputFrames.push_back( frame );
    }

    frameReadback( packet->context.frameID );

    for( FrameVector::const_iterator i = _outputFrames.begin(); 
         i != _outputFrames.end(); ++i)
    {
        Frame* frame = *i;
        frame->setReady();
    }

    _outputFrames.clear();
    _context = &_nativeContext;
    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdFrameTransmit( net::Command& command )
{
    const ChannelFrameTransmitPacket* packet = 
        command.getPacket<ChannelFrameTransmitPacket>();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK transmit " << getName() <<  " " 
                                      << packet << std::endl;

    net::Session* session   = getSession();
    net::NodePtr  localNode = session->getLocalNode();
    net::NodePtr  server    = session->getServer();
    Pipe*         pipe      = getPipe();
    Frame*        frame     = pipe->getFrame( packet->frame,
                                              packet->context.eye );

    for( uint32_t i=0; i<packet->nNodes; ++i )
    {
        const net::NodeID& nodeID = packet->nodes[i];
        net::NodePtr toNode = localNode->connect( nodeID );
        EQASSERT( toNode->isConnected( ));
        EQLOG( LOG_ASSEMBLY ) << "channel \"" << getName() << "\" transmit " 
                              << frame << " to " << nodeID << std::endl;

        frame->useSendToken( getIAttribute( IATTR_HINT_SENDTOKEN ) == ON );
        getNode()->transmitter.send( frame->getData(), toNode, 
                                     getPipe()->getCurrentFrame( ));
    }

    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdFrameViewStart( net::Command& command )
{
    ChannelFrameViewStartPacket* packet = 
        command.getPacket<ChannelFrameViewStartPacket>();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK view start " << getName() <<  " "
                                       << packet << std::endl;

    _setRenderContext( packet->context );
    // TBD ChannelStatistics event( Statistic::CHANNEL_READBACK, this );
    frameViewStart( packet->context.frameID );
    _context = &_nativeContext;

    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdFrameViewFinish( net::Command& command )
{
    ChannelFrameViewFinishPacket* packet = 
        command.getPacket<ChannelFrameViewFinishPacket>();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK view finish " << getName()
                                      <<  " " << packet << std::endl;

    _setRenderContext( packet->context );
    ChannelStatistics event( Statistic::CHANNEL_VIEW_FINISH, this );

    // TBD ChannelStatistics event( Statistic::CHANNEL_READBACK, this );
    frameViewFinish( packet->context.frameID );
    _context = &_nativeContext;

    return net::COMMAND_HANDLED;
}

}

#include "../fabric/channel.cpp"
template class eq::fabric::Channel< eq::Channel, eq::Window >;


