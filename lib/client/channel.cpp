
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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
#include "log.h"
#include "node.h"
#include "nodeFactory.h"
#include "packets.h"
#include "pipe.h"
#include "range.h"
#include "renderContext.h"
#include "server.h"
#include "task.h"
#include "view.h"
#include "frameBufferObject.h"

#include <eq/net/command.h>

using namespace eq::base;
using namespace std;

namespace eq
{
typedef net::CommandFunc<Channel> ChannelFunc;

#define MAKE_ATTR_STRING( attr ) ( string("EQ_CHANNEL_") + #attr )
std::string Channel::_iAttributeStrings[IATTR_ALL] = {
    MAKE_ATTR_STRING( IATTR_HINT_STATISTICS ),
    MAKE_ATTR_STRING( IATTR_FILL1 ),
    MAKE_ATTR_STRING( IATTR_FILL2 )
};

Channel::Channel( Window* parent )
        : _window( parent )
        , _currentContext( &_nativeContext )
        , _tasks( TASK_NONE )
        , _state( STATE_STOPPED )
        , _fixedPVP( false )
        , _fbo(0)
        , _drawable( 0 )
{
    parent->_addChannel( this );
    EQINFO << " New eq::Channel @" << (void*)this << endl;
}

Channel::~Channel()
{  
    EQINFO << " Delete eq::Channel @" << (void*)this << endl;
    _window->_removeChannel( this );
}

void Channel::attachToSession( const uint32_t id, 
                              const uint32_t instanceID, 
                              net::Session* session )
{
    net::Object::attachToSession( id, instanceID, session );

    net::CommandQueue* queue = _window->getPipeThreadQueue();

    registerCommand( CMD_CHANNEL_CONFIG_INIT, 
                     ChannelFunc( this, &Channel::_cmdConfigInit ), queue );
    registerCommand( CMD_CHANNEL_CONFIG_EXIT, 
                     ChannelFunc( this, &Channel::_cmdConfigExit ), queue );
    registerCommand( CMD_CHANNEL_FRAME_START,
                     ChannelFunc( this, &Channel::_cmdFrameStart ), queue );
    registerCommand( CMD_CHANNEL_FRAME_FINISH,
                     ChannelFunc( this, &Channel::_cmdFrameFinish ), queue );
    registerCommand( CMD_CHANNEL_FRAME_CLEAR, 
                     ChannelFunc( this, &Channel::_cmdFrameClear ), queue );
    registerCommand( CMD_CHANNEL_FRAME_DRAW, 
                     ChannelFunc( this, &Channel::_cmdFrameDraw ), queue );
    registerCommand( CMD_CHANNEL_FRAME_DRAW_FINISH, 
                    ChannelFunc( this, &Channel::_cmdFrameDrawFinish ), queue );
    registerCommand( CMD_CHANNEL_FRAME_ASSEMBLE, 
                     ChannelFunc( this, &Channel::_cmdFrameAssemble ), queue );
    registerCommand( CMD_CHANNEL_FRAME_READBACK, 
                     ChannelFunc( this, &Channel::_cmdFrameReadback ), queue );
    registerCommand( CMD_CHANNEL_FRAME_TRANSMIT, 
                     ChannelFunc( this, &Channel::_cmdFrameTransmit ), queue );
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

VisitorResult Channel::accept( ChannelVisitor& visitor )
{
    return visitor.visit( this );
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
    if ( _drawable == FBO_NONE )
        return true;
    
    if  (  !_window->getOSWindow()  ||
          !GLEW_ARB_texture_non_power_of_two ||
          !GLEW_EXT_framebuffer_object )
    {
    	setErrorMessage( "Can't use FBO due to missing GL extensions" );
        return false;
    }
        
    // needs glew initialized (see above)
    _fbo = new FrameBufferObject( glewGetContext( ));
    _fbo->setColorFormat( _window->getColorType( ));
        
    if( _fbo->init( _nativeContext.pvp.w, _nativeContext.pvp.h, 
                    _drawable & FBO_DEPTH, _drawable & FBO_STENCIL ) ) 
    {
         return true;
    }

    setErrorMessage( "FBO initialization failed" );
    delete _fbo;
    _fbo = 0;
    return false;
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
        eq::PixelViewport pvp = windowPVP.getSubPVP( _nativeContext.vp );
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
    if( _currentContext->frustum.nearPlane == nearPlane && 
        _currentContext->frustum.farPlane == farPlane )
    {
        return;
    }

    _nativeContext.frustum.adjustNear( nearPlane );
    _nativeContext.frustum.farPlane = farPlane;
    _nativeContext.ortho.nearPlane  = nearPlane;
    _nativeContext.ortho.farPlane   = farPlane;

    if( _currentContext != &_nativeContext )
    {
        _currentContext->frustum.adjustNear( nearPlane );
        _currentContext->frustum.farPlane = farPlane;
        _currentContext->ortho.nearPlane = nearPlane;
        _currentContext->ortho.farPlane  = farPlane;
    }

    ChannelSetNearFarPacket packet;
    packet.nearPlane = nearPlane;
    packet.farPlane  = farPlane;
    
    net::NodePtr node = RefPtr_static_cast< Server, net::Node >( getServer( ));
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
        const vmml::Vector3ub color = getUniqueColor();
        glClearColor( color.r/255.0f, color.g/255.0f, color.b/255.0f, 1.0f );
    }
#endif // DEBUG

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

    Compositor::assembleFrames( getInputFrames(), this );

    EQ_GL_CALL( resetAssemblyState( ));
}

void Channel::frameReadback( const uint32_t frameID )
{
    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));
    EQ_GL_CALL( setupAssemblyState( ));

    Window::ObjectManager* glObjects = getWindow()->getObjectManager();

    const FrameVector& frames = getOutputFrames();
    for( FrameVector::const_iterator i = frames.begin(); i != frames.end(); ++i)
    {
        Frame* frame = *i;
        frame->startReadback( glObjects );
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
    EQ_GL_ERROR( "before setupAssemblyState" );
    glPushAttrib( GL_ENABLE_BIT | GL_STENCIL_BUFFER_BIT | GL_VIEWPORT_BIT | 
                  GL_SCISSOR_BIT | GL_LINE_BIT | GL_PIXEL_MODE_BIT | 
                  GL_POLYGON_BIT );

    glDisable( GL_DEPTH_TEST );
    glDisable( GL_BLEND );
    glDisable( GL_ALPHA_TEST );
    glDisable( GL_STENCIL_TEST );
    glDisable( GL_TEXTURE_1D );
    glDisable( GL_TEXTURE_2D );
    glDisable( GL_TEXTURE_3D );
    glDisable( GL_FOG );
    glDisable( GL_CLIP_PLANE0 );
    glDisable( GL_CLIP_PLANE1 );
    glDisable( GL_CLIP_PLANE2 );
    glDisable( GL_CLIP_PLANE3 );
    glDisable( GL_CLIP_PLANE4 );
    glDisable( GL_CLIP_PLANE5 );
    
    glPolygonMode( GL_FRONT, GL_FILL );

    EQASSERT( _window );    
    const PixelViewport& pvp = _window->getPixelViewport();
    EQASSERT( pvp.isValid( ));

    glViewport( 0, 0, pvp.w, pvp.h );
    glScissor( 0, 0, pvp.w, pvp.h );

    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glOrtho( 0.0f, pvp.w, 0.0f, pvp.h, -1.0f, 1.0f );

    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    EQ_GL_ERROR( "after  setupAssemblyState" );
}

void Channel::resetAssemblyState()
{
    EQ_GL_ERROR( "before resetAssemblyState" );
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();

    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();

    glPopAttrib();
    EQ_GL_ERROR( "after  resetAssemblyState" );
}

void Channel::_setRenderContext( RenderContext& context )
{
    _currentContext = &context;
    _window->addRenderContext( context );
}

const Viewport& Channel::getViewport() const
{
    return _currentContext->vp;
}

const PixelViewport& Channel::getPixelViewport() const
{
    return _currentContext->pvp;
}

const vmml::Vector2i& Channel::getPixelOffset() const
{
    return _currentContext->offset;
}

uint32_t Channel::getDrawBuffer() const
{
    return _currentContext->buffer;
}

uint32_t Channel::getReadBuffer() const
{
    return _currentContext->buffer;
}

const ColorMask& Channel::getDrawBufferMask() const
{
    return _currentContext->bufferMask;
}

const vmml::Frustumf& Channel::getFrustum() const
{
    return _currentContext->frustum;
}

const vmml::Frustumf& Channel::getOrtho() const
{
    return _currentContext->ortho;
}

const Range& Channel::getRange() const
{
    return _currentContext->range;
}

const Pixel& Channel::getPixel() const
{
    return _currentContext->pixel;
}

const Zoom& Channel::getZoom() const
{
    return _currentContext->zoom;
}

Eye Channel::getEye() const
{
    return _currentContext->eye;
}

const vmml::Matrix4f& Channel::getHeadTransform() const
{
    return _currentContext->headTransform;
}

vmml::Frustumf Channel::getScreenFrustum() const
{
    const Pixel& pixel = getPixel();
    PixelViewport pvp( getPixelViewport( ));
    const Viewport& vp( getViewport( ));

    pvp.x = static_cast<int32_t>( pvp.w / vp.w * vp.x );
    pvp.y = static_cast<int32_t>( pvp.h / vp.h * vp.y );
    pvp *= pixel;

    return vmml::Frustumf( pvp.x, pvp.getXEnd(), pvp.y, pvp.getYEnd(),
                           -1.f, 1.f );
}

const vmml::Vector4i& Channel::getOverdraw() const
{
    return _currentContext->overdraw;
}

FrameBufferObject* Channel::getFrameBufferObject()
{
    return _fbo;
}

const View* Channel::getView()
{
    Pipe* pipe = getPipe();
    return pipe->getView( _currentContext->view );
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
    if (( !_fbo )&&( !_window->isFBOWindow() ))
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
        EQERROR << "Can't apply viewport " << pvp << endl;
        return;
    }

    EQ_GL_CALL( glViewport( pvp.x, pvp.y, pvp.w, pvp.h ));
    EQ_GL_CALL( glScissor( pvp.x, pvp.y, pvp.w, pvp.h ));
}

void Channel::applyFrustum() const
{
    const vmml::Frustumf& frustum = getFrustum();
    EQ_GL_CALL( glFrustum( frustum.left, frustum.right,             \
                           frustum.bottom, frustum.top,             \
                           frustum.nearPlane, frustum.farPlane )); 
    EQVERB << "Apply " << frustum << endl;
}

void Channel::applyOrtho() const
{
    const vmml::Frustumf& ortho = getOrtho();
    EQ_GL_CALL( glOrtho( ortho.left, ortho.right,               \
                         ortho.bottom, ortho.top,               \
                         ortho.nearPlane, ortho.farPlane )); 
    EQVERB << "Apply " << ortho << endl;
}

void Channel::applyScreenFrustum() const
{
    const vmml::Frustumf frustum = getScreenFrustum();
    EQ_GL_CALL( glOrtho( frustum.left, frustum.right,               \
                         frustum.bottom, frustum.top,               \
                         frustum.nearPlane, frustum.farPlane ));
    EQVERB << "Apply " << frustum << endl;
}

void Channel::applyHeadTransform() const
{
    const vmml::Matrix4f& xfm = getHeadTransform();
    EQ_GL_CALL( glMultMatrixf( xfm.ml ));
    EQVERB << "Apply head transform: " << xfm << endl;
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
            const View* view = getView();
            if( !view )
                return true;

            EQASSERT( _currentContext == &_nativeContext );
            // transform to view event, which is meaningful for the config 
            configEvent.data.type       = Event::VIEW_RESIZE;
            configEvent.data.originator = view->getID();

            ResizeEvent& resize = configEvent.data.resize;
            resize.dw = resize.w / static_cast< float >( _initialSize.x );
            resize.dh = resize.h / static_cast< float >( _initialSize.y );
            break;
        }

        default:
            EQWARN << "Unhandled channel event of type " << event.type
                   << endl;
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
    EntityData() : yPos( 0 ), idle( 0 ), nIdle( 0 ) {}
    uint32_t yPos;
    uint32_t idle;
    uint32_t nIdle;
    std::string name;
};
}

void Channel::drawStatistics()
{
    Config* config = getConfig();
    EQASSERT( config );

    vector< FrameStatistics > statistics;
    config->getStatistics( statistics );

    if( statistics.empty( )) 
        return;

    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));
    EQ_GL_CALL( setupAssemblyState( ));

    glClear( GL_DEPTH_BUFFER_BIT );

    glDisable( GL_LIGHTING );
    glEnable( GL_DEPTH_TEST );

    const util::BitmapFont& font =_window->getObjectManager()->getDefaultFont();

    int64_t       xStart = 0;
    PixelViewport pvp    = _window->getPixelViewport();
    pvp.x = 0;
    pvp.y = 0;

    // find min/max time
    int64_t                 xMax      = 0;
    int64_t                 xMin      = std::numeric_limits< int64_t >::max();

    for( vector< FrameStatistics >::const_iterator i = statistics.begin();
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
    while( (xMax - xMin) / scale > pvp.w )
        scale *= 10;

    xMax  /= scale;
    xStart = xMax - pvp.getXEnd() + SPACE;
    uint32_t                       nextY = pvp.getYEnd() - SPACE;

    std::map< uint32_t, EntityData > entities;

    float dim = 0.0f;
    for( vector< FrameStatistics >::reverse_iterator i = statistics.rbegin();
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
                    EntityData& data = entities[ id ];
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

                float y1 = y;
                float y2 = y - HEIGHT;
                float z  = 0.0f;
                const float x1 = startTime - xStart;
                const float x2 = endTime   - xStart;
                
                switch( stat.type )
                {
                    case Statistic::CHANNEL_CLEAR:
                        glColor3f( .5f-dim, 1.0f-dim, .5f-dim );
                        break;
                    case Statistic::CHANNEL_DRAW:
                        glColor3f( 0.f, 1.0f-dim, 0.f ); 
                        break;
                    case Statistic::CHANNEL_DRAW_FINISH:
                        glColor3f( 0.f, .5f-dim, 0.f ); 
                        break;
                    case Statistic::CHANNEL_ASSEMBLE:
                        glColor3f( 1.0f-dim, 1.0f-dim, 0.f ); 
                        break;
                    case Statistic::CHANNEL_READBACK:
                        glColor3f( 1.0f-dim, .5f-dim, .5f-dim ); 
                        break;
                    case Statistic::NODE_TRANSMIT:
                    case Statistic::CHANNEL_TRANSMIT:
                        glColor3f( 0.f, 0.f, 1.0f-dim ); 
                        z = 0.5f; 
                        break;
                    case Statistic::CHANNEL_TRANSMIT_NODE:
                        glColor3f( 0.5f-dim, 0.5f-dim, 1.0f-dim ); 
                        y1 -= SPACE;
                        z = 0.6f; 
                        break;
                    case Statistic::CHANNEL_COMPRESS:
                    case Statistic::NODE_COMPRESS:
                    {
                        glColor3f( 1.0f-dim, 1.0f-dim, 1.0f-dim ); 
                        y1 -= SPACE;
                        z = 0.7f; 
                        
                        stringstream text;
                        text << static_cast< unsigned >( 100.f * stat.ratio );
                        glRasterPos3f( x2+1, y2, 0.99f );
                        font.draw( text.str( ));
                        break;
                    }
                    case Statistic::CHANNEL_WAIT_FRAME:
                    case Statistic::CONFIG_WAIT_FINISH_FRAME:
                        glColor3f( 1.0f-dim, 0.f, 0.f ); 
                        y1 -= SPACE;
                        y2 += SPACE;
                        z = 0.1f; 
                        break;

                    case Statistic::WINDOW_FINISH:
                        glColor3f( 1.0f-dim, 1.0f-dim, 0.f ); 
                        break;

                    case Statistic::WINDOW_SWAP_BARRIER:
                        glColor3f( 1.0f-dim, 0.f, 0.f ); 
                        break;
                    
                    case Statistic::WINDOW_THROTTLE_FRAMERATE:
                        glColor3f( 1.0f, 0.f, 1.f ); 
                        break; 
                           
                    case Statistic::CONFIG_START_FRAME:
                        glColor3f( .5f-dim, 1.0f-dim, .5f-dim ); 
                        z = 0.1f; 
                        break;
                    case Statistic::CONFIG_FINISH_FRAME:
                        glColor3f( .5f-dim, .5f-dim, .5f-dim ); 
                        break;

                    default:
                        glColor3f( 1.0f-dim, 1.0f-dim, 1.0f-dim ); 
                        z = 0.2f; 
                        break;
                }

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

        glBegin( GL_QUADS );
        glColor3f( .5f-dim, 1.0f-dim, .5f-dim );
        glVertex3f( frameMin+1.0f, pvp.getYEnd(), 0.3f );
        glVertex3f( frameMin,      pvp.getYEnd(), 0.3f );
        glVertex3f( frameMin,      nextY,         0.3f );
        glVertex3f( frameMin+1.0f, nextY,         0.3f );

        glColor3f( .5f-dim, .5f-dim, .5f-dim );
        glVertex3f( frameMax+1.0f, pvp.getYEnd(), 0.3f );
        glVertex3f( frameMax,      pvp.getYEnd(), 0.3f );
        glVertex3f( frameMax,      nextY,         0.3f );
        glVertex3f( frameMax+1.0f, nextY,         0.3f );
        glEnd();

        dim += .1f;
    }

    for( std::map< uint32_t, EntityData >::const_iterator i = entities.begin();
         i != entities.end(); ++i )
    {
        const EntityData& data = i->second;
        ostringstream text;

        text << data.name;
        if( data.nIdle > 0 )
            text << "  " << data.idle / data.nIdle << "% idle";
            
        glColor3f( 1.f, 1.f, 1.f );
        glRasterPos3f( 100.f, data.yPos-SPACE-12.0f, 0.99f );
        font.draw( text.str( ));
    }

    glColor3f( 1.f, 1.f, 1.f );
    ostringstream scaleText;
    scaleText << ": " << scale << "ms/pixel";
    font.draw( scaleText.str( ));

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


//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
net::CommandResult Channel::_cmdConfigInit( net::Command& command )
{
    const ChannelConfigInitPacket* packet = 
        command.getPacket<ChannelConfigInitPacket>();
    EQLOG( LOG_INIT ) << "TASK channel config init " << packet << endl;

    ChannelConfigInitReplyPacket reply;
    _error.clear();

    if( _window->isRunning( ))
    {
        _state = STATE_INITIALIZING;
        if( packet->pvp.isValid( ))
            _setPixelViewport( packet->pvp );
        else
            _setViewport( packet->vp );
        
        _name     = packet->name;
        _tasks    = packet->tasks;
        _color    = packet->color;
        _drawable = packet->drawable;
        _nativeContext.view = packet->view;
        _initialSize.x = _nativeContext.pvp.w;
        _initialSize.y = _nativeContext.pvp.h;

        memcpy( _iAttributes, packet->iAttributes, IATTR_ALL * sizeof(int32_t));

        reply.result = configInit( packet->initID );

        reply.nearPlane   = _nativeContext.frustum.nearPlane;
        reply.farPlane    = _nativeContext.frustum.farPlane;

        if( reply.result )
            _state = STATE_RUNNING;
    }
    else
        reply.result = false;

    EQLOG( LOG_INIT ) << "TASK channel config init reply " << &reply << endl;
    send( command.getNode(), reply, _error );
    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdConfigExit( net::Command& command )
{
    const ChannelConfigExitPacket* packet =
        command.getPacket<ChannelConfigExitPacket>();
    EQLOG( LOG_INIT ) << "Exit channel " << packet << endl;

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
    const ChannelFrameStartPacket* packet = 
        command.getPacket<ChannelFrameStartPacket>();
    EQVERB << "handle channel frame start " << packet << endl;

    //_grabFrame( packet->frameNumber ); single-threaded
    _nativeContext.view.version = packet->viewVersion;
    _nativeContext.overdraw     = packet->overdraw;

    bindFrameBuffer();
    frameStart( packet->frameID, packet->frameNumber );

    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdFrameFinish( net::Command& command )
{
    const ChannelFrameFinishPacket* packet =
        command.getPacket<ChannelFrameFinishPacket>();
    EQLOG( LOG_TASKS ) << "TASK frame finish " << getName() <<  " " << packet
                       << endl;

    frameFinish( packet->frameID, packet->frameNumber );

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
    EQLOG( LOG_TASKS ) << "TASK clear " << getName() <<  " " << packet << endl;

    ChannelStatistics event( Statistic::CHANNEL_CLEAR, this );

    _setRenderContext( packet->context );
    frameClear( packet->context.frameID );
    _currentContext = &_nativeContext;

    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdFrameDraw( net::Command& command )
{
    ChannelFrameDrawPacket* packet = 
        command.getPacket<ChannelFrameDrawPacket>();
    EQLOG( LOG_TASKS ) << "TASK draw " << getName() <<  " " << packet << endl;

    ChannelStatistics event( Statistic::CHANNEL_DRAW, this );

    _setRenderContext( packet->context );
    frameDraw( packet->context.frameID );
    _currentContext = &_nativeContext;

    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdFrameDrawFinish( net::Command& command )
{
    ChannelFrameDrawFinishPacket* packet = 
        command.getPacket< ChannelFrameDrawFinishPacket >();
    EQLOG( LOG_TASKS ) << "TASK draw finish " << getName() <<  " " << packet
                       << endl;

    ChannelStatistics event( Statistic::CHANNEL_DRAW_FINISH, this );
    frameDrawFinish( packet->frameID, packet->frameNumber );

    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdFrameAssemble( net::Command& command )
{
    ChannelFrameAssemblePacket* packet = 
        command.getPacket<ChannelFrameAssemblePacket>();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK assemble " << getName() <<  " " 
                                       << packet << endl;

    ChannelStatistics event( Statistic::CHANNEL_ASSEMBLE, this );
    _setRenderContext( packet->context );

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
    _currentContext = &_nativeContext;

    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdFrameReadback( net::Command& command )
{
    ChannelFrameReadbackPacket* packet = 
        command.getPacket<ChannelFrameReadbackPacket>();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK readback " << getName() <<  " " 
                                       << packet << endl;

    ChannelStatistics event( Statistic::CHANNEL_READBACK, this );
    _currentContext = &packet->context;

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
    _currentContext = &_nativeContext;
    return net::COMMAND_HANDLED;
}

net::CommandResult Channel::_cmdFrameTransmit( net::Command& command )
{
    const ChannelFrameTransmitPacket* packet = 
        command.getPacket<ChannelFrameTransmitPacket>();
    EQLOG( LOG_TASKS | LOG_ASSEMBLY ) << "TASK transmit " << getName() <<  " " 
                                      << packet << endl;

#ifndef EQ_ASYNC_TRANSMIT
    ChannelStatistics event( Statistic::CHANNEL_TRANSMIT, this );
#endif

    net::Session* session   = getSession();
    net::NodePtr  localNode = session->getLocalNode();
    net::NodePtr  server    = session->getServer();
    Pipe*         pipe      = getPipe();
    Frame*        frame     = pipe->getFrame( packet->frame,
                                              packet->context.eye );

    for( uint32_t i=0; i<packet->nNodes; ++i )
    {
        net::NodeID nodeID = packet->nodes[i];
        nodeID.convertToHost();

        net::NodePtr toNode = localNode->connect( nodeID );
        EQLOG( LOG_ASSEMBLY ) << "channel \"" << getName() << "\" transmit " 
                              << frame << " to " << nodeID << endl;

#ifdef EQ_ASYNC_TRANSMIT
        getNode()->transmitter.send( frame->getData(), toNode, 
                                     getPipe()->getCurrentFrame( ));
#else
        ChannelStatistics nodeEvent( Statistic::CHANNEL_TRANSMIT_NODE, this );
        ChannelStatistics compressEvent( Statistic::CHANNEL_COMPRESS, this );
        frame->transmit( toNode, compressEvent.event );
#endif
    }

    return net::COMMAND_HANDLED;
}
}
