
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c)      2010, Cedric Stalder <cedric.stalder@gmail.com> 
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
#include "client.h"
#include "commands.h"
#include "config.h"
#include "configEvent.h"
#include "event.h"
#include "global.h"
#include "log.h"
#include "nodeFactory.h"
#include "pipe.h"
#include "osWindow.h"
#include "packets.h"
#include "server.h"
#include "windowStatistics.h"
#include <eq/fabric/elementVisitor.h>

#ifdef AGL
#  include "aglWindow.h"
#endif
#ifdef GLX
#  include "glXWindow.h"
#endif
#ifdef WGL
#  include "wglWindow.h"
#endif

#include <eq/fabric/task.h>
#include <eq/net/barrier.h>
#include <eq/net/command.h>
#include <eq/base/sleep.h>

namespace eq
{

typedef fabric::Window< Pipe, Window, Channel > Super;

/** @cond IGNORE */
typedef net::CommandFunc<Window> WindowFunc;
/** @endcond */

namespace
{
const char* _smallFontKey  = "eq_small_font";
const char* _mediumFontKey = "eq_medium_font";
}

Window::Window( Pipe* parent )
        : Super( parent )
        , _sharedContextWindow( 0 ) // default set below
        , _osWindow( 0 )
        , _state( STATE_STOPPED )
        , _objectManager( 0 )
        , _lastTime ( 0.0 )
        , _avgFPS ( 0.0 )
        , _lastSwapTime( 0 )
{
    const WindowVector& windows = parent->getWindows();
    if( windows.empty( ))
        setSharedContextWindow( this );
    else
        setSharedContextWindow( windows.front( ));

    EQINFO << " New eq::Window @" << (void*)this << std::endl;
}

Window::~Window()
{
    Pipe* pipe = getPipe();
    EQASSERT( pipe );

    if( pipe->isCurrent( this ))
        pipe->setCurrent( 0 );

    delete _objectManager;
    _objectManager = 0;
}

void Window::attachToSession( const uint32_t id, 
                              const uint32_t instanceID, 
                              net::Session* session )
{
    net::Object::attachToSession( id, instanceID, session );

    net::CommandQueue* queue = getPipe()->getPipeThreadQueue();

    registerCommand( CMD_WINDOW_CREATE_CHANNEL, 
                     WindowFunc( this, &Window::_cmdCreateChannel ), 
                     queue );
    registerCommand( CMD_WINDOW_DESTROY_CHANNEL,
                     WindowFunc( this, &Window::_cmdDestroyChannel ), 
                     queue );
    registerCommand( CMD_WINDOW_CONFIG_INIT,
                     WindowFunc( this, &Window::_cmdConfigInit ), 
                     queue );
    registerCommand( CMD_WINDOW_CONFIG_EXIT, 
                     WindowFunc( this, &Window::_cmdConfigExit ), 
                     queue );
    registerCommand( CMD_WINDOW_FRAME_START,
                     WindowFunc( this, &Window::_cmdFrameStart ), 
                     queue );
    registerCommand( CMD_WINDOW_FRAME_FINISH,
                     WindowFunc( this, &Window::_cmdFrameFinish ), 
                     queue );
    registerCommand( CMD_WINDOW_FINISH, 
                     WindowFunc( this, &Window::_cmdFinish), queue );
    registerCommand( CMD_WINDOW_THROTTLE_FRAMERATE, 
                    WindowFunc( this, &Window::_cmdThrottleFramerate ),
                     queue );
    registerCommand( CMD_WINDOW_BARRIER, 
                     WindowFunc( this, &Window::_cmdBarrier ), 
                     queue );
    registerCommand( CMD_WINDOW_NV_BARRIER, 
                     WindowFunc( this, &Window::_cmdNVBarrier ), 
                     queue );
    registerCommand( CMD_WINDOW_SWAP, 
                     WindowFunc( this, &Window::_cmdSwap), queue );
    registerCommand( CMD_WINDOW_FRAME_DRAW_FINISH, 
                     WindowFunc( this, &Window::_cmdFrameDrawFinish ), 
                     queue );
}

void Window::_updateFPS()
{
    const float curTime      = static_cast< float >( getConfig()->getTime( ));
    const float curInterval  = curTime - _lastTime;

    const bool   isFirstFrame = _lastTime == 0.0f;
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

net::CommandQueue* Window::getPipeThreadQueue()
{ 
    Pipe* pipe = getPipe();
    EQASSERT( pipe );
    return pipe->getPipeThreadQueue(); 
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
    return ( pipe ? pipe->getConfig() : 0);
}
Config* Window::getConfig() 
{
    Pipe* pipe = getPipe();
    EQASSERT( pipe );
    return ( pipe ? pipe->getConfig() : 0);
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
    CHECK_THREAD( _pipeThread );
    _renderContexts[BACK].push_back( context );
}

bool Window::getRenderContext( const int32_t x, const int32_t y,
                               RenderContext& context ) const
{
    CHECK_THREAD( _pipeThread );
    if( !_osWindow )
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

void Window::setOSWindow( OSWindow* window )
{
    _osWindow = window;

    if( !window )
        return;

    // Initialize context-specific data
    makeCurrent();
    
    DrawableConfig drawableConfig;
    _osWindow->queryDrawableConfig( drawableConfig );
    _setDrawableConfig( drawableConfig );
    _setupObjectManager();
}

const OSPipe* Window::getOSPipe() const
{
    const Pipe* pipe = getPipe();
    EQASSERT( pipe );
    return pipe->getOSPipe();
}

OSPipe* Window::getOSPipe()
{
    Pipe* pipe = getPipe();
    EQASSERT( pipe );
    return pipe->getOSPipe();
}

void Window::frameFinish( const uint32_t frameID, const uint32_t frameNumber )
{
    releaseFrame( frameNumber );
    flush();
    _updateFPS();
}

//----------------------------------------------------------------------
// configInit
//----------------------------------------------------------------------
bool Window::configInit( const uint32_t initID )
{
    if( !getPixelViewport().isValid( ))
    {
        setErrorMessage( "Window pixel viewport invalid - pipe init failed?" );
        return false;
    }

    EQASSERT( !_osWindow );

    if( !configInitOSWindow( initID )) return false;
    if( !configInitGL( initID ))       return false;

    return true;
}

bool Window::configInitOSWindow( const uint32_t initID )
{
    const Pipe* pipe     = getPipe();
    OSWindow*   osWindow = 0;

    switch( pipe->getWindowSystem( ))
    {
#ifdef GLX
        case WINDOW_SYSTEM_GLX:
            EQINFO << "Using GLXWindow" << std::endl;
            osWindow = new GLXWindow( this );
            break;
#endif

#ifdef AGL
        case WINDOW_SYSTEM_AGL:
            EQINFO << "Using AGLWindow" << std::endl;
            osWindow = new AGLWindow( this );
            break;
#endif

#ifdef WGL
        case WINDOW_SYSTEM_WGL:
            EQINFO << "Using WGLWindow" << std::endl;
            osWindow = new WGLWindow( this );
            break;
#endif

        default:
            EQERROR << "Window system " << pipe->getWindowSystem() 
                    << " not implemented or supported" << std::endl;
            return false;
    }

    EQASSERT( osWindow );
    if( !osWindow->configInit( ))
    {
        EQWARN << "OS Window initialization failed: " << getErrorMessage() << std::endl;
        delete osWindow;
        return false;
    }

    setOSWindow( osWindow );
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
    if( _objectManager && _objectManager->getSharedUsage() == 1 )
    {
        _objectManager->deleteEqBitmapFont( _smallFontKey );
        _objectManager->deleteEqBitmapFont( _mediumFontKey );
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

    EQASSERT( _objectManager->getEqBitmapFont( _smallFontKey ));
    return _objectManager->getEqBitmapFont( _smallFontKey );
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
    return _objectManager->getEqBitmapFont( _mediumFontKey );
}

bool Window::configInitGL( const uint32_t initID )
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
    return configExitOSWindow() && ret;
}

bool Window::configExitOSWindow()
{
    _releaseObjectManager();

    if( _osWindow )
    {
        _osWindow->configExit( );

        delete _osWindow;
        _osWindow = 0;
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

    _osWindow->makeCurrent();
    // _pipe->setCurrent done by OSWindow::makeCurrent
}

void Window::bindFrameBuffer() const
{
    _osWindow->bindFrameBuffer( );
}

void Window::swapBuffers()
{
    _osWindow->swapBuffers();
    EQVERB << "----- SWAP -----" << std::endl;
}

GLEWContext* Window::glewGetContext()
{ 
    return _osWindow->glewGetContext();
}

const GLEWContext* Window::glewGetContext() const
{ 
    return _osWindow->glewGetContext();
}

void Window::_enterBarrier( net::ObjectVersion barrier )
{
    EQLOG( net::LOG_BARRIER ) << "swap barrier " << barrier << std::endl;
    Node* node = getNode();
    net::Barrier* netBarrier = node->getBarrier( barrier );

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
        case Event::POINTER_MOTION:
        case Event::POINTER_BUTTON_PRESS:
        case Event::POINTER_BUTTON_RELEASE:
        case Event::STATISTIC:
            break;

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
net::CommandResult Window::_cmdCreateChannel( net::Command& command )
{
    const WindowCreateChannelPacket* packet = 
        command.getPacket<WindowCreateChannelPacket>();
    EQLOG( LOG_INIT ) << "Create channel " << packet << std::endl;

    Channel* channel = Global::getNodeFactory()->createChannel( this );
    Config* config = getConfig();
    EQCHECK( config->mapObject( channel, packet->channelID ));

    return net::COMMAND_HANDLED;
}

net::CommandResult Window::_cmdDestroyChannel( net::Command& command ) 
{
    const WindowDestroyChannelPacket* packet =
        command.getPacket<WindowDestroyChannelPacket>();
    EQLOG( LOG_INIT ) << "Destroy channel " << packet << std::endl;

    Channel* channel = _findChannel( packet->channelID );
    EQASSERT( channel );

    Config*  config  = getConfig();
    config->detachObject( channel );
    Global::getNodeFactory()->releaseChannel( channel );

    return net::COMMAND_HANDLED;
}

net::CommandResult Window::_cmdConfigInit( net::Command& command )
{
    const WindowConfigInitPacket* packet = 
        command.getPacket<WindowConfigInitPacket>();
    EQLOG( LOG_INIT ) << "TASK window config init " << packet << std::endl;

    WindowConfigInitReplyPacket reply;
    setErrorMessage( std::string( ));

    if( getPipe()->isRunning( ))
    {
        _state = STATE_INITIALIZING;
        reply.result = configInit( packet->initID );
    }
    else
        reply.result = false;

    EQLOG( LOG_INIT ) << "TASK window config init reply " << &reply << std::endl;

    net::NodePtr node = command.getNode();
    if( !reply.result )
    {
        send( node, reply, getErrorMessage() );
        return net::COMMAND_HANDLED;
    }

    commit();
    send( node, reply );

    _state = STATE_RUNNING;
    return net::COMMAND_HANDLED;
}

net::CommandResult Window::_cmdConfigExit( net::Command& command )
{
    const WindowConfigExitPacket* packet =
        command.getPacket<WindowConfigExitPacket>();
    EQLOG( LOG_INIT ) << "TASK window config exit " << packet << std::endl;

    WindowConfigExitReplyPacket reply;
    
    if( _state == STATE_STOPPED )
        reply.result = true;
    else
    {
        if( getPipe()->isRunning( ) && _osWindow )
        {
            makeCurrent();
            getPipe()->flushFrames();
        }
        // else emergency exit, no context available.

        reply.result = configExit();
    }

    _state = STATE_STOPPED;
    send( command.getNode(), reply );
    return net::COMMAND_HANDLED;
}

net::CommandResult Window::_cmdFrameStart( net::Command& command )
{
    CHECK_THREAD( _pipeThread );

    const WindowFrameStartPacket* packet = 
        command.getPacket<WindowFrameStartPacket>();
    EQLOG( LOG_TASKS ) << "TASK frame start " << getName() <<  " " << packet
                       << std::endl;

    //_grabFrame( packet->frameNumber ); single-threaded
    sync( packet->version );

    EQASSERT( _osWindow );
    const DrawableConfig& drawableConfig = getDrawableConfig();
    if( drawableConfig.doublebuffered )
        _renderContexts[FRONT].swap( _renderContexts[BACK] );
    _renderContexts[BACK].clear();

    makeCurrent();

    frameStart( packet->frameID, packet->frameNumber );
    return net::COMMAND_HANDLED;
}

net::CommandResult Window::_cmdFrameFinish( net::Command& command )
{
    const WindowFrameFinishPacket* packet =
        command.getPacket<WindowFrameFinishPacket>();
    EQVERB << "handle window frame sync " << packet << std::endl;

    makeCurrent();
    frameFinish( packet->frameID, packet->frameNumber );
    commit();
    return net::COMMAND_HANDLED;
}

net::CommandResult Window::_cmdFinish(net::Command& command ) 
{
    makeCurrent();

    WindowStatistics stat( Statistic::WINDOW_FINISH, this );
    finish();

    return net::COMMAND_HANDLED;
}

net::CommandResult  Window::_cmdThrottleFramerate( net::Command& command )
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
        base::sleep( static_cast< uint32_t >( timeLeft ));
    }

    _lastSwapTime = getConfig()->getTime();
    return net::COMMAND_HANDLED;
}
        
net::CommandResult Window::_cmdBarrier( net::Command& command )
{
    const WindowBarrierPacket* packet = 
        command.getPacket<WindowBarrierPacket>();
    EQVERB << "handle barrier " << packet << std::endl;
    EQLOG( LOG_TASKS ) << "TASK swap barrier  " << getName() << std::endl;
    
    _enterBarrier( packet->barrier );
    return net::COMMAND_HANDLED;
}

net::CommandResult Window::_cmdNVBarrier( net::Command& command )
{
    const WindowNVBarrierPacket* packet = 
        command.getPacket<WindowNVBarrierPacket>();
    EQLOG( LOG_TASKS ) << "TASK join NV_swap_group" << std::endl;
    
    EQASSERT( _osWindow );
    _osWindow->joinNVSwapBarrier( packet->group, packet->barrier );

    _enterBarrier( packet->netBarrier );
    return net::COMMAND_HANDLED;
}

net::CommandResult Window::_cmdSwap( net::Command& command ) 
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
    return net::COMMAND_HANDLED;
}

net::CommandResult Window::_cmdFrameDrawFinish( net::Command& command )
{
    WindowFrameDrawFinishPacket* packet = 
        command.getPacket< WindowFrameDrawFinishPacket >();
    EQLOG( LOG_TASKS ) << "TASK draw finish " << getName() <<  " " << packet
                       << std::endl;

    frameDrawFinish( packet->frameID, packet->frameNumber );
    return net::COMMAND_HANDLED;
}

}

#include "../fabric/window.cpp"
template class eq::fabric::Window< eq::Pipe, eq::Window, eq::Channel >;
