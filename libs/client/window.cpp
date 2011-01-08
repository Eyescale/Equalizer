
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c) 2009-2011, Cedric Stalder <cedric.stalder@gmail.com> 
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

#include "window.h"

#include "channel.h"
#include "channelPackets.h"
#include "client.h"
#include "config.h"
#include "configEvent.h"
#include "event.h"
#include "error.h"
#include "global.h"
#include "log.h"
#include "node.h"
#include "nodeFactory.h"
#include "pipe.h"
#include "pipePackets.h"
#include "server.h"
#include "systemWindow.h"
#include "windowPackets.h"
#include "windowStatistics.h"
#ifdef AGL
#  include "aglWindow.h"
#endif
#ifdef GLX
#  include "glXWindow.h"
#endif
#ifdef WGL
#  include "wglWindow.h"
#endif

#include <eq/fabric/elementVisitor.h>
#include <eq/fabric/task.h>
#include <co/barrier.h>
#include <co/command.h>
#include <co/base/sleep.h>

namespace eq
{

typedef fabric::Window< Pipe, Window, Channel > Super;

/** @cond IGNORE */
typedef co::CommandFunc<Window> WindowFunc;
/** @endcond */

namespace
{
const char* _smallFontKey  = "eq_small_font";
const char* _mediumFontKey = "eq_medium_font";
}

Window::Window( Pipe* parent )
        : Super( parent )
        , _sharedContextWindow( 0 ) // default set below
        , _systemWindow( 0 )
        , _state( STATE_STOPPED )
        , _objectManager( 0 )
        , _lastTime ( 0.0 )
        , _avgFPS ( 0.0 )
        , _lastSwapTime( 0 )
{
    const Windows& windows = parent->getWindows();
    if( windows.empty( ))
        setSharedContextWindow( this );
    else
        setSharedContextWindow( windows.front( ));
}

Window::~Window()
{
    EQASSERT( getChannels().empty( ));

    Pipe* pipe = getPipe();
    EQASSERT( pipe );

    if( pipe->isCurrent( this ))
        pipe->setCurrent( 0 );

    delete _objectManager;
    _objectManager = 0;
}

void Window::attach( const co::base::UUID& id, const uint32_t instanceID )
{
    Super::attach( id, instanceID );

    co::CommandQueue* queue = getPipeThreadQueue();

    registerCommand( fabric::CMD_WINDOW_CREATE_CHANNEL, 
                     WindowFunc( this, &Window::_cmdCreateChannel ), queue );
    registerCommand( fabric::CMD_WINDOW_DESTROY_CHANNEL,
                     WindowFunc( this, &Window::_cmdDestroyChannel ), queue );
    registerCommand( fabric::CMD_WINDOW_CONFIG_INIT,
                     WindowFunc( this, &Window::_cmdConfigInit ), queue );
    registerCommand( fabric::CMD_WINDOW_CONFIG_EXIT, 
                     WindowFunc( this, &Window::_cmdConfigExit ), queue );
    registerCommand( fabric::CMD_WINDOW_FRAME_START,
                     WindowFunc( this, &Window::_cmdFrameStart ), queue );
    registerCommand( fabric::CMD_WINDOW_FRAME_FINISH,
                     WindowFunc( this, &Window::_cmdFrameFinish ), queue );
    registerCommand( fabric::CMD_WINDOW_FINISH, 
                     WindowFunc( this, &Window::_cmdFinish), queue );
    registerCommand( fabric::CMD_WINDOW_THROTTLE_FRAMERATE, 
                     WindowFunc( this, &Window::_cmdThrottleFramerate ),
                     queue );
    registerCommand( fabric::CMD_WINDOW_BARRIER, 
                     WindowFunc( this, &Window::_cmdBarrier ), queue );
    registerCommand( fabric::CMD_WINDOW_NV_BARRIER, 
                     WindowFunc( this, &Window::_cmdNVBarrier ), queue );
    registerCommand( fabric::CMD_WINDOW_SWAP, 
                     WindowFunc( this, &Window::_cmdSwap), queue );
    registerCommand( fabric::CMD_WINDOW_FRAME_DRAW_FINISH, 
                     WindowFunc( this, &Window::_cmdFrameDrawFinish ), queue );
}

void Window::_updateFPS()
{
    const float curTime      = static_cast< float >( getConfig()->getTime( ));
    const float curInterval  = curTime - _lastTime;

    const bool isFirstFrame = _lastTime == 0.0f;
    _lastTime = curTime;

    if( isFirstFrame || curInterval < 1e-3f )
        return;

    const float curFPS = 1000.0f / curInterval;

    if( curFPS < 1.0f || // don't average FPS if rendering is too slow
        // or if current frame rate differs a lot from average (rendering loop
        // was paused)
        ( _avgFPS > 10.f * curFPS || 10.f * _avgFPS < curFPS ))
    {
        _avgFPS = curFPS;
        return;
    }
    //else  average FPS over time

    // We calculate weighted sum of average frame rate with current frame rate
    // to prevent FPS count flickering.
    //
    // Weighted sum calculation here is the following:
    _avgFPS = curFPS * ( _avgFPS + 1.f ) / ( curFPS + 1.f );

    // The higher current frame rate, the less it affects averaged FR. This is
    // equivalent of averaging over many frames, i.e. when rendering is fast, we
    // suppress FPS counter flickering stronger.
}


void Window::drawFPS()
{
    std::ostringstream fpsText;
    fpsText << std::setprecision(3) << getFPS() << " FPS";

    const Font* font = getSmallFont();
    const PixelViewport& pvp = getPixelViewport();

    glRasterPos3f( pvp.w - 60.f, 10.f , 0.99f );
    glColor3f( 1.f, 1.f, 1.f );

    font->draw( fpsText.str( ));
}

co::CommandQueue* Window::getPipeThreadQueue()
{ 
    return getPipe()->getPipeThreadQueue(); 
}

co::CommandQueue* Window::getCommandThreadQueue()
{ 
    return getPipe()->getCommandThreadQueue(); 
}

const Node* Window::getNode() const 
{
    const Pipe* pipe = getPipe();
    EQASSERT( pipe );
    return ( pipe ? pipe->getNode() : 0 );
}
Node* Window::getNode()
{
    Pipe* pipe = getPipe();
    EQASSERT( pipe );
    return ( pipe ? pipe->getNode() : 0 );
}

const Config* Window::getConfig() const
{
    const Pipe* pipe = getPipe();
    EQASSERT( pipe );
    return ( pipe ? pipe->getConfig() : 0 );
}
Config* Window::getConfig() 
{
    Pipe* pipe = getPipe();
    EQASSERT( pipe );
    return ( pipe ? pipe->getConfig() : 0 );
}

ClientPtr Window::getClient()
{
    Pipe* pipe = getPipe();
    EQASSERT( pipe );
    return ( pipe ? pipe->getClient() : 0 );
}

ServerPtr Window::getServer() 
{
    Pipe* pipe = getPipe();
    EQASSERT( pipe );
    return ( pipe ? pipe->getServer() : 0 );
}

//======================================================================
// pipe-thread methods
//======================================================================

//----------------------------------------------------------------------
// render context
//----------------------------------------------------------------------
void Window::_addRenderContext( const RenderContext& context )
{
    EQ_TS_THREAD( _pipeThread );
    _renderContexts[BACK].push_back( context );
}

bool Window::getRenderContext( const int32_t x, const int32_t y,
                               RenderContext& context ) const
{
    EQ_TS_THREAD( _pipeThread );
    if( !_systemWindow )
        return false;

    const DrawableConfig& drawableConfig = getDrawableConfig();
    const unsigned which = drawableConfig.doublebuffered ? FRONT : BACK;

    std::vector< RenderContext >::const_reverse_iterator i   =
        _renderContexts[which].rbegin(); 
    std::vector< RenderContext >::const_reverse_iterator end =
        _renderContexts[which].rend();

    // invert y to follow GL convention
    const int32_t glY = getPixelViewport().h - y; 

    for( ; i != end; ++i )
    {
        const RenderContext& candidate = *i;
        if( candidate.pvp.isInside( x, glY ))
        {
            context = candidate;
            return true;
        }
    }
    return false;
}

uint32_t Window::getColorFormat() const
{
    switch( getIAttribute( Window::IATTR_PLANES_COLOR ))
    {
        case RGBA32F:  return GL_RGBA32F;
        case RGBA16F:  return GL_RGBA16F;
        default:       return GL_RGBA;
    }
}

void Window::setSystemWindow( SystemWindow* window )
{
    _systemWindow = window;

    if( !window )
        return;

    // Initialize context-specific data
    makeCurrent();
    DrawableConfig config;
    _systemWindow->queryDrawableConfig( config );
    _setDrawableConfig( config );
    _setupObjectManager();
}

const SystemPipe* Window::getSystemPipe() const
{
    const Pipe* pipe = getPipe();
    EQASSERT( pipe );
    return pipe->getSystemPipe();
}

SystemPipe* Window::getSystemPipe()
{
    Pipe* pipe = getPipe();
    EQASSERT( pipe );
    return pipe->getSystemPipe();
}

void Window::frameStart( const uint128_t&, const uint32_t frameNumber ) 
{
    startFrame( frameNumber );
}

void Window::frameDrawFinish( const uint128_t&, const uint32_t frameNumber )
{
    releaseFrameLocal( frameNumber );
}

void Window::frameFinish( const uint128_t&, const uint32_t frameNumber )
{
    releaseFrame( frameNumber );
    flush();
    _updateFPS();
}

void Window::startFrame( const uint32_t ) { /* currently nop */ }
void Window::releaseFrame( const uint32_t ) { /* currently nop */ }
void Window::releaseFrameLocal( const uint32_t ) { /* nop */ }

//----------------------------------------------------------------------
// configInit
//----------------------------------------------------------------------
bool Window::configInit( const uint128_t& initID )
{
    if( !getPixelViewport().isValid( ))
    {
        setError( ERROR_WINDOW_PVP_INVALID );
        return false;
    }

    EQASSERT( !_systemWindow );

    if( !configInitSystemWindow( initID )) return false;
    if( !configInitGL( initID ))       return false;

    return true;
}

bool Window::configInitSystemWindow( const uint128_t& )
{
    const Pipe* pipe = getPipe();
    SystemWindow* systemWindow = 0;

    switch( pipe->getWindowSystem( ))
    {
#ifdef GLX
        case WINDOW_SYSTEM_GLX:
            EQINFO << "Using GLXWindow" << std::endl;
            systemWindow = new GLXWindow( this );
            break;
#endif

#ifdef AGL
        case WINDOW_SYSTEM_AGL:
            EQINFO << "Using AGLWindow" << std::endl;
            systemWindow = new AGLWindow( this );
            break;
#endif

#ifdef WGL
        case WINDOW_SYSTEM_WGL:
            EQINFO << "Using WGLWindow" << std::endl;
            systemWindow = new WGLWindow( this );
            break;
#endif

        default:
            setError( ERROR_WINDOWSYSTEM_UNKNOWN );
            return false;
    }

    EQASSERT( systemWindow );
    if( !systemWindow->configInit( ))
    {
        EQWARN << "System window initialization failed: " << getError()
               << std::endl;
        delete systemWindow;
        return false;
    }

    setSystemWindow( systemWindow );
    return true;
}

void Window::_setupObjectManager()
{
    if( !glewGetContext( ))
        return;

    _releaseObjectManager();

    Window*    sharedWindow = getSharedContextWindow();
    ObjectManager* sharedOM = sharedWindow ? sharedWindow->getObjectManager():0;

    if( sharedOM )
        _objectManager = new ObjectManager( glewGetContext(), sharedOM );
    else
        _objectManager = new ObjectManager( glewGetContext( ));
}

void Window::_releaseObjectManager()
{
    if( _objectManager )
    {
        _objectManager->deleteEqBitmapFont( _smallFontKey );
        _objectManager->deleteEqBitmapFont( _mediumFontKey );
        if( !_objectManager->isShared( ))
            _objectManager->deleteAll();
    }

    delete _objectManager;
    _objectManager = 0;
}

const Window::Font* Window::getSmallFont()
{
    EQASSERT( _objectManager );
    if( !_objectManager )
        return 0;

    Window::Font* font = _objectManager->getEqBitmapFont( _smallFontKey );
    if( !font )
    {
        font = _objectManager->newEqBitmapFont( _smallFontKey );
        font->init( getPipe()->getWindowSystem(), "" );
    }
    return font;
}

const Window::Font* Window::getMediumFont()
{
    EQASSERT( _objectManager );
    if( !_objectManager )
        return 0;

    Window::Font* font = _objectManager->getEqBitmapFont( _mediumFontKey );
    if( !font )
    {
        font = _objectManager->newEqBitmapFont( _mediumFontKey );
        font->init( getPipe()->getWindowSystem(), "", 20 );
    }
    return font;
}

bool Window::configInitGL( const uint128_t& )
{
    glEnable( GL_SCISSOR_TEST ); // needed to constrain channel viewport
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LESS );

    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );

    glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
    glEnable( GL_COLOR_MATERIAL );

    glClearDepth( 1.f );
    //glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );

    glClear( GL_COLOR_BUFFER_BIT );
    swapBuffers();
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    return true;
}

//----------------------------------------------------------------------
// configExit
//----------------------------------------------------------------------
bool Window::configExit()
{
    const bool ret = configExitGL();
    return configExitSystemWindow() && ret;
}

bool Window::configExitSystemWindow()
{
    _releaseObjectManager();

    if( _systemWindow )
    {
        _systemWindow->configExit( );

        delete _systemWindow;
        _systemWindow = 0;
    }
    
    Pipe* pipe = getPipe();
    if( pipe->isCurrent( this ))
        pipe->setCurrent( 0 );

    return true;
}

void Window::makeCurrent( const bool useCache ) const
{
    if( useCache && getPipe()->isCurrent( this ))
        return;

    _systemWindow->makeCurrent();
    // _pipe->setCurrent done by SystemWindow::makeCurrent
}

void Window::bindFrameBuffer() const
{
    _systemWindow->bindFrameBuffer( );
}

void Window::swapBuffers()
{
    _systemWindow->swapBuffers();
    EQVERB << "----- SWAP -----" << std::endl;
}

const GLEWContext* Window::glewGetContext() const
{ 
    return _systemWindow->glewGetContext();
}

void Window::_enterBarrier( co::ObjectVersion barrier )
{
    EQLOG( co::LOG_BARRIER ) << "swap barrier " << barrier << std::endl;
    Node* node = getNode();
    co::Barrier* netBarrier = node->getBarrier( barrier );

    WindowStatistics stat( Statistic::WINDOW_SWAP_BARRIER, this );
    netBarrier->enter();
}


//======================================================================
// event-handler methods
//======================================================================
bool Window::processEvent( const Event& event )
{
    ConfigEvent configEvent;
    switch( event.type )
    {
        case Event::WINDOW_HIDE:
            setPixelViewport( PixelViewport( 0, 0, 0, 0 ));
            break;

        case Event::WINDOW_SHOW:
        case Event::WINDOW_RESIZE:
            setPixelViewport( PixelViewport( event.resize.x, event.resize.y, 
                                             event.resize.w, event.resize.h ));
            break;

        case Event::KEY_PRESS:
        case Event::KEY_RELEASE:
            if( event.key.key == KC_VOID )
                return true; //ignore
            // else fall through
        case Event::WINDOW_EXPOSE:
        case Event::WINDOW_CLOSE:
        case Event::WINDOW_POINTER_WHEEL:
        case Event::STATISTIC:
            break;

        case Event::WINDOW_POINTER_MOTION:
        case Event::WINDOW_POINTER_BUTTON_PRESS:
        case Event::WINDOW_POINTER_BUTTON_RELEASE:
        {
            // dispatch pointer events to destination channel, if any
            const Channels& channels = getChannels();
            for( Channels::const_iterator i = channels.begin();
                 i != channels.end(); ++i )
            {
                Channel* channel = *i;
                if( !channel->getNativeView( ))
                    continue;

                const PixelViewport& pvp = getPixelViewport();
                const PixelViewport& channelPVP =
                    channel->getNativePixelViewport();
                
                // convert y to GL notation (Channel PVP uses GL coordinates)
                const int32_t y = pvp.h - event.pointer.y;
                if( !channelPVP.isInside( event.pointer.x, y ))
                    continue;

                Event channelEvent = event;
                switch( event.type )
                {
                case Event::WINDOW_POINTER_MOTION:
                    channelEvent.type = Event::CHANNEL_POINTER_MOTION;
                    break;
                case Event::WINDOW_POINTER_BUTTON_PRESS:
                    channelEvent.type = Event::CHANNEL_POINTER_BUTTON_PRESS;
                    break;
                case Event::WINDOW_POINTER_BUTTON_RELEASE:
                    channelEvent.type = Event::CHANNEL_POINTER_BUTTON_RELEASE;
                    break;
                default:
                    EQWARN << "Unhandled window event of type " << event.type
                           << std::endl;
                    EQUNIMPLEMENTED;
                }

                EQASSERT( channel->getID() != co::base::UUID::ZERO );
                channelEvent.originator = channel->getID();
                channelEvent.pointer.x -= channelPVP.x;
                channelEvent.pointer.y = channelPVP.h - y + channelPVP.y;
                channel->processEvent( channelEvent );
            }
            break;
        }

        case Event::WINDOW_SCREENSAVER:
            switch( getIAttribute( IATTR_HINT_SCREENSAVER ))
            {
                case OFF:
                    return true; // screen saver stays inactive
                case ON:
                    return false; // screen saver becomes active
                default: // AUTO
                    if( getDrawableConfig().doublebuffered &&
                        getIAttribute( IATTR_HINT_DRAWABLE ) == WINDOW )
                    {
                        return true; // screen saver stays inactive
                    }
                    return false;
            }

        case Event::UNKNOWN:
            // unknown window-system native event, which was not handled
            return false;

        default:
            EQWARN << "Unhandled window event of type " << event.type
                   << std::endl;
            EQUNIMPLEMENTED;
    }

    configEvent.data = event;

    Config* config = getConfig();
    config->sendEvent( configEvent );
    return true;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
bool Window::_cmdCreateChannel( co::Command& command )
{
    const WindowCreateChannelPacket* packet = 
        command.getPacket<WindowCreateChannelPacket>();
    EQLOG( LOG_INIT ) << "Create channel " << packet << std::endl;

    Channel* channel = Global::getNodeFactory()->createChannel( this );
    channel->init(); // not in ctor, virtual method

    Config* config = getConfig();
    EQCHECK( config->mapObject( channel, packet->channelID ));

    return true;
}

bool Window::_cmdDestroyChannel( co::Command& command ) 
{
    const WindowDestroyChannelPacket* packet =
        command.getPacket<WindowDestroyChannelPacket>();
    EQLOG( LOG_INIT ) << "Destroy channel " << packet << std::endl;

    Channel* channel = _findChannel( packet->channelID );
    EQASSERT( channel );

    ChannelConfigExitReplyPacket reply( packet->channelID,
                                        channel->isStopped( ));
    Config* config = getConfig();
    config->unmapObject( channel );
    Global::getNodeFactory()->releaseChannel( channel );

    getServer()->send( reply ); // do not use Object::send()
    return true;
}

bool Window::_cmdConfigInit( co::Command& command )
{
    const WindowConfigInitPacket* packet = 
        command.getPacket<WindowConfigInitPacket>();
    EQLOG( LOG_INIT ) << "TASK window config init " << packet << std::endl;

    WindowConfigInitReplyPacket reply;
    setError( ERROR_NONE );

    if( getPipe()->isRunning( ))
    {
        _state = STATE_INITIALIZING;
        reply.result = configInit( packet->initID );
        if( reply.result )
            _state = STATE_RUNNING;
    }
    else
    {
        setError( ERROR_WINDOW_PIPE_NOTRUNNING );
        reply.result = false;
    }
    EQLOG( LOG_INIT ) << "TASK window config init reply " << &reply <<std::endl;

    co::NodePtr node = command.getNode();

    commit();
    send( node, reply );
    return true;
}

bool Window::_cmdConfigExit( co::Command& command )
{
    WindowConfigExitPacket* packet =
        command.getPacket<WindowConfigExitPacket>();
    EQLOG( LOG_INIT ) << "TASK window config exit " << packet << std::endl;

    if( _state != STATE_STOPPED )
    {
        if( getPipe()->isRunning( ) && _systemWindow )
        {
            makeCurrent();
            getPipe()->flushFrames();
        }
        // else emergency exit, no context available.

        _state = configExit() ? STATE_STOPPED : STATE_FAILED;
    }

    PipeDestroyWindowPacket destroyPacket( getID( ));
    getPipe()->send( getLocalNode(), destroyPacket );
    return true;
}

bool Window::_cmdFrameStart( co::Command& command )
{
    EQ_TS_THREAD( _pipeThread );

    const WindowFrameStartPacket* packet = 
        command.getPacket<WindowFrameStartPacket>();
    EQLOG( LOG_TASKS ) << "TASK frame start " << getName() <<  " " << packet
                       << std::endl;

    //_grabFrame( packet->frameNumber ); single-threaded
    sync( packet->version );

    EQASSERT( _systemWindow );
    const DrawableConfig& drawableConfig = getDrawableConfig();
    if( drawableConfig.doublebuffered )
        _renderContexts[FRONT].swap( _renderContexts[BACK] );
    _renderContexts[BACK].clear();

    makeCurrent();

    frameStart( packet->frameID, packet->frameNumber );
    return true;
}

bool Window::_cmdFrameFinish( co::Command& command )
{
    const WindowFrameFinishPacket* packet =
        command.getPacket<WindowFrameFinishPacket>();
    EQVERB << "handle window frame sync " << packet << std::endl;

    makeCurrent();
    frameFinish( packet->frameID, packet->frameNumber );
    commit();
    return true;
}

bool Window::_cmdFinish( co::Command& ) 
{
    WindowStatistics stat( Statistic::WINDOW_FINISH, this );
    makeCurrent();
    finish();

    return true;
}

bool  Window::_cmdThrottleFramerate( co::Command& command )
{
    WindowThrottleFramerate* packet = 
        command.getPacket< WindowThrottleFramerate >();
    EQLOG( LOG_TASKS ) << "TASK throttle framerate " << getName() << " "
                       << packet << std::endl;

    // throttle to given framerate
    const int64_t elapsed  = getConfig()->getTime() - _lastSwapTime;
    const float timeLeft = packet->minFrameTime - static_cast<float>( elapsed );
    
    if( timeLeft >= 1.f )
    {
        WindowStatistics stat( Statistic::WINDOW_THROTTLE_FRAMERATE, this );   
        co::base::sleep( static_cast< uint32_t >( timeLeft ));
    }

    _lastSwapTime = getConfig()->getTime();
    return true;
}
        
bool Window::_cmdBarrier( co::Command& command )
{
    const WindowBarrierPacket* packet = 
        command.getPacket<WindowBarrierPacket>();
    EQVERB << "handle barrier " << packet << std::endl;
    EQLOG( LOG_TASKS ) << "TASK swap barrier  " << getName() << std::endl;
    
    _enterBarrier( packet->barrier );
    return true;
}

bool Window::_cmdNVBarrier( co::Command& command )
{
    const WindowNVBarrierPacket* packet = 
        command.getPacket<WindowNVBarrierPacket>();
    EQLOG( LOG_TASKS ) << "TASK join NV_swap_group" << std::endl;
    
    EQASSERT( _systemWindow );
    _systemWindow->joinNVSwapBarrier( packet->group, packet->barrier );

    _enterBarrier( packet->netBarrier );
    return true;
}

bool Window::_cmdSwap( co::Command& command ) 
{
    WindowSwapPacket* packet = command.getPacket< WindowSwapPacket >();
    EQLOG( LOG_TASKS ) << "TASK swap buffers " << getName() << " " << packet
                       << std::endl;

    if( getDrawableConfig().doublebuffered )
    {
        // swap
        WindowStatistics stat( Statistic::WINDOW_SWAP, this );
        makeCurrent();
        swapBuffers();
    }
    return true;
}

bool Window::_cmdFrameDrawFinish( co::Command& command )
{
    WindowFrameDrawFinishPacket* packet = 
        command.getPacket< WindowFrameDrawFinishPacket >();
    EQLOG( LOG_TASKS ) << "TASK draw finish " << getName() <<  " " << packet
                       << std::endl;

    frameDrawFinish( packet->frameID, packet->frameNumber );
    return true;
}

}

#include "../fabric/window.ipp"
template class eq::fabric::Window< eq::Pipe, eq::Window, eq::Channel >;
/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                                                 const eq::Super& );
/** @endcond */
