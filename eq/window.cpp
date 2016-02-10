
/* Copyright (c) 2005-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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
#include "config.h"
#include "error.h"
#include "gl.h"
#include "global.h"
#include "log.h"
#include "node.h"
#include "nodeFactory.h"
#include "pipe.h"
#include "server.h"
#include "systemPipe.h"
#include "systemWindow.h"
#include "windowStatistics.h"

#include <eq/util/objectManager.h>
#include <eq/fabric/commands.h>
#include <eq/fabric/elementVisitor.h>
#include <eq/fabric/event.h>
#include <eq/fabric/leafVisitor.h>
#include <eq/fabric/task.h>

#include <co/barrier.h>
#include <co/exception.h>
#include <co/objectICommand.h>
#include <lunchbox/sleep.h>

namespace eq
{

typedef fabric::Window< Pipe, Window, Channel, WindowSettings > Super;

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
        , _transferWindow( 0 )
        , _systemWindow( 0 )
        , _state( STATE_STOPPED )
        , _objectManager( 0 )
        , _lastTime ( 0.0f )
        , _avgFPS ( 0.0f )
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
    LBASSERT( getChannels().empty( ));
}

void Window::attach( const uint128_t& id, const uint32_t instanceID )
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
    registerCommand( fabric::CMD_WINDOW_FLUSH,
                     WindowFunc( this, &Window::_cmdFlush), queue );
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

void Window::notifyViewportChanged()
{
    Super::notifyViewportChanged();
    if( !isRunning( ))
        return;

    // Commit immediately so that the server has the new data before the app
    // does send the startFrame() after a resize event.
    const uint128_t version = commit();
    if( version != co::VERSION_NONE )
        send( getServer(), fabric::CMD_OBJECT_SYNC );
}

void Window::_updateFPS()
{
    const float curTime = float( getConfig()->getTime( ));
    const float curInterval = curTime - _lastTime;
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
    else // average FPS over time
    {
        // We calculate weighted sum of average frame rate with current frame
        // rate to prevent FPS count flickering.
        //
        // Weighted sum calculation here is the following:
        _avgFPS = curFPS * ( _avgFPS + 1.f ) / ( curFPS + 1.f );

        // The higher current frame rate, the less it affects averaged FR. This
        // is equivalent of averaging over many frames, i.e. when rendering is
        // fast, we suppress FPS counter flickering stronger.
    }

    WindowStatistics stat( Statistic::WINDOW_FPS, this );
    stat.event.data.statistic.currentFPS = curFPS;
    stat.event.data.statistic.averageFPS = _avgFPS;
}


void Window::drawFPS()
{
    std::ostringstream fpsText;
    fpsText << std::setprecision(3) << getFPS() << " FPS";

    const util::BitmapFont* font = getSmallFont();
    const PixelViewport& pvp = getPixelViewport();

    glRasterPos3f( pvp.w - 60.f, pvp.h - 16.f , 0.99f );
    glColor3f( .8f, .8f, .8f );

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

uint32_t Window::getCurrentFrame() const
{
    return getPipe()->getCurrentFrame();
}

const Node* Window::getNode() const
{
    const Pipe* pipe = getPipe();
    LBASSERT( pipe );
    return ( pipe ? pipe->getNode() : 0 );
}
Node* Window::getNode()
{
    Pipe* pipe = getPipe();
    LBASSERT( pipe );
    return ( pipe ? pipe->getNode() : 0 );
}

const Config* Window::getConfig() const
{
    const Pipe* pipe = getPipe();
    LBASSERT( pipe );
    return ( pipe ? pipe->getConfig() : 0 );
}
Config* Window::getConfig()
{
    Pipe* pipe = getPipe();
    LBASSERT( pipe );
    return ( pipe ? pipe->getConfig() : 0 );
}

ClientPtr Window::getClient()
{
    Pipe* pipe = getPipe();
    LBASSERT( pipe );
    return ( pipe ? pipe->getClient() : 0 );
}

ServerPtr Window::getServer()
{
    Pipe* pipe = getPipe();
    LBASSERT( pipe );
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
    LB_TS_THREAD( _pipeThread );
    _renderContexts[BACK].push_back( context );
}

bool Window::getRenderContext( const int32_t x, const int32_t y,
                               RenderContext& context ) const
{
    LB_TS_THREAD( _pipeThread );
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

void Window::setSharedContextWindow( const Window* sharedContextWindow )
{
    _sharedContextWindow = sharedContextWindow;
}

const Window* Window::getSharedContextWindow() const
{
    return _sharedContextWindow;
}

uint32_t Window::getColorFormat() const
{
    return getSettings().getColorFormat();
}

void Window::flush() const
{
    LBASSERT( _systemWindow );
    if( _systemWindow )
        _systemWindow->flush();
}

void Window::finish() const
{
    LBASSERT( _systemWindow );
    if( _systemWindow )
        _systemWindow->finish();
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
    LBASSERT( pipe );
    return pipe->getSystemPipe();
}

SystemPipe* Window::getSystemPipe()
{
    Pipe* pipe = getPipe();
    LBASSERT( pipe );
    return pipe->getSystemPipe();
}

void Window::frameStart( const uint128_t&, const uint32_t frameNumber )
{
    startFrame( frameNumber );
}

void Window::frameDrawFinish( const uint128_t&, const uint32_t frameNumber )
{
    releaseFrameLocal( frameNumber );

    // https://github.com/Eyescale/Equalizer/issues/95
    if( getNode()->getPipes().size() > 1 )
        finish();
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
        sendError( ERROR_WINDOW_PVP_INVALID );
        return false;
    }

    LBASSERT( !_systemWindow );

    int glMajorVersion = 1;
    int glMinorVersion = 1;
    if( getPipe()->getSystemPipe()->getMaxOpenGLVersion() != AUTO )
    {
        float maj, min;
        min = modff( getPipe()->getSystemPipe()->getMaxOpenGLVersion(), &maj );
        glMajorVersion = static_cast< int >( maj );
        glMinorVersion = static_cast< int >( min*10.f );
    }

    if( getIAttribute( WindowSettings::IATTR_HINT_OPENGL_MAJOR ) == AUTO )
        setIAttribute( WindowSettings::IATTR_HINT_OPENGL_MAJOR, glMajorVersion);
    if( getIAttribute( WindowSettings::IATTR_HINT_OPENGL_MINOR ) == AUTO )
        setIAttribute( WindowSettings::IATTR_HINT_OPENGL_MINOR, glMinorVersion);

    return configInitSystemWindow( initID ) && configInitGL( initID );
}

bool Window::configInitSystemWindow( const uint128_t& )
{
    const Pipe* pipe = getPipe();
    WindowSettings settings = getSettings();
    const SystemWindow* sysWindow = _sharedContextWindow ?
                                    _sharedContextWindow->getSystemWindow() : 0;
    settings.setSharedContextWindow( sysWindow );
    SystemWindow* systemWindow =
        pipe->getWindowSystem().createWindow( this, settings );

    LBASSERT( systemWindow );
    if( !systemWindow->configInit( ))
    {
        LBWARN << "System window initialization failed" << std::endl;
        systemWindow->configExit();
        delete systemWindow;
        return false;
    }

    setPixelViewport( systemWindow->getPixelViewport( ));
    setSystemWindow( systemWindow );
    return true;
}

void Window::_setupObjectManager()
{
    if( !glewGetContext( ))
        return;

    _releaseObjectManager();

    const Window* sharedWindow = getSharedContextWindow();
    if( sharedWindow && sharedWindow != this )
        _objectManager = sharedWindow->_objectManager;
    else
    {
        util::ObjectManager om( glewGetContext( ));
        _objectManager = om;
    }
}

void Window::_releaseObjectManager()
{
    _objectManager.deleteEqBitmapFont( _smallFontKey );
    _objectManager.deleteEqBitmapFont( _mediumFontKey );
    if( !_objectManager.isShared( ))
        _objectManager.deleteAll();
    _objectManager.clear();
}

const util::BitmapFont* Window::getSmallFont()
{
    util::BitmapFont* font = _objectManager.getEqBitmapFont( _smallFontKey );
    if( !font )
    {
        font = _objectManager.newEqBitmapFont( _smallFontKey );
        font->init( getPipe()->getWindowSystem(), "" );
    }
    return font;
}

const util::BitmapFont* Window::getMediumFont()
{
    util::BitmapFont* font = _objectManager.getEqBitmapFont( _mediumFontKey );
    if( !font )
    {
        font = _objectManager.newEqBitmapFont( _mediumFontKey );
        font->init( getPipe()->getWindowSystem(), "", 20 );
    }
    return font;
}

bool Window::configInitGL( const uint128_t& )
{
    const bool coreProfile = getIAttribute(
                WindowSettings::IATTR_HINT_CORE_PROFILE ) == ON;
    if( !coreProfile )
    {
        EQ_GL_CALL( glEnable( GL_LIGHTING ));
        EQ_GL_CALL( glEnable( GL_LIGHT0 ));

        EQ_GL_CALL( glColorMaterial( GL_FRONT_AND_BACK,
                                     GL_AMBIENT_AND_DIFFUSE ));
        EQ_GL_CALL( glEnable( GL_COLOR_MATERIAL ));
    }

    EQ_GL_CALL( glEnable( GL_SCISSOR_TEST )); // to constrain channel viewport
    EQ_GL_CALL( glEnable( GL_DEPTH_TEST ));
    EQ_GL_CALL( glDepthFunc( GL_LESS ));
    EQ_GL_CALL( glClearDepth( 1.f ));

    EQ_GL_CALL( glClear( GL_COLOR_BUFFER_BIT ));
    swapBuffers();
    EQ_GL_CALL( glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ));

    return true;
}

bool Window::createTransferWindow()
{
    LB_TS_THREAD( _pipeThread );
    LBASSERT( _systemWindow );

    if( _transferWindow )
        return true;

    // create another (shared) osWindow with no drawable
    WindowSettings settings = getSettings();
    settings.setIAttribute( WindowSettings::IATTR_HINT_DRAWABLE, OFF );
    const SystemWindow* sysWindow = _sharedContextWindow ?
                                    _sharedContextWindow->getSystemWindow() : 0;
    settings.setSharedContextWindow( sysWindow );
    const Pipe* pipe = getPipe();
    _transferWindow = pipe->getWindowSystem().createWindow( this, settings );

    if( _transferWindow )
    {
        if( !_transferWindow->configInit( ))
        {
            LBWARN << "Transfer window initialization failed" << std::endl;
            delete _transferWindow;
            _transferWindow = 0;
        }
        else
        {
            // #177: It looks like the driver realizes the context on the first
            // makeCurrent
            _transferWindow->makeCurrent();
            _transferWindow->doneCurrent();
        }
    }
    else
        LBERROR << "Window system " << pipe->getWindowSystem()
                << " not implemented or supported" << std::endl;

    makeCurrent();

    LBVERB << "Transfer window initialization finished" << std::endl;
    return _transferWindow != 0;
}

const GLEWContext* Window::getTransferGlewContext() const
{
    LBASSERT( _transferWindow );
    if( _transferWindow )
        return _transferWindow->glewGetContext();
    return 0;
}

void Window::deleteTransferWindow()
{
    if( !_transferWindow )
        return;

    _transferWindow->configExit();
    delete _transferWindow;
    _transferWindow = 0;
}

SystemWindow* Window::getTransferWindow()
{
    return _transferWindow;
}

const SystemWindow* Window::getTransferWindow() const
{
    return _transferWindow;
}

//----------------------------------------------------------------------
// configExit
//----------------------------------------------------------------------
bool Window::configExit()
{
    if( !_systemWindow )
        return true;

    const bool ret = configExitGL();
    return configExitSystemWindow() && ret;
}

bool Window::configExitSystemWindow()
{
    // _transferWindow has to be deleted from the same thread it was
    // initialized
    LBASSERT( !_transferWindow );

    _releaseObjectManager();

    if( _systemWindow )
    {
        _systemWindow->configExit();

        delete _systemWindow;
        _systemWindow = 0;
    }
    return true;
}

void Window::makeCurrent( const bool useCache ) const
{
    LBASSERT( _systemWindow );
    if( _systemWindow )
        _systemWindow->makeCurrent( useCache );
}

void Window::doneCurrent() const
{
    LBASSERT( _systemWindow );
    if( _systemWindow )
        _systemWindow->doneCurrent();
}

void Window::bindFrameBuffer() const
{
    LBASSERT( _systemWindow );
    if( _systemWindow )
        _systemWindow->bindFrameBuffer();
}

void Window::bindDrawFrameBuffer() const
{
    LBASSERT( _systemWindow );
    if( _systemWindow )
        _systemWindow->bindDrawFrameBuffer();
}

void Window::updateFrameBuffer() const
{
    LBASSERT( _systemWindow );
    if( _systemWindow )
        _systemWindow->updateFrameBuffer();
}

void Window::swapBuffers()
{
    _systemWindow->swapBuffers();
    LBLOG( co::LOG_BARRIER ) << "Swap buffers done" << getName() << std::endl;
}

const GLEWContext* Window::glewGetContext() const
{
    return _systemWindow ? _systemWindow->glewGetContext() : 0;
}

void Window::_enterBarrier( co::ObjectVersion barrier )
{
    LBLOG( co::LOG_BARRIER ) << "swap barrier " << barrier << " " << getName()
                             << std::endl;
    Node* node = getNode();
    co::Barrier* netBarrier = node->getBarrier( barrier );
    if( !netBarrier )
        return;

    WindowStatistics stat( Statistic::WINDOW_SWAP_BARRIER, this );
    Config* config = getConfig();
    const uint32_t timeout = config->getTimeout()/2;
    LBCHECK( netBarrier->enter( timeout ));
}

void Window::_updateEvent( Event& event )
{
    // TODO 2.0 event interface will stream these and remove them from Event
    event.time = getConfig()->getTime();
    event.originator = getID();
    event.serial = getSerial();

    switch( event.type )
    {
        case Event::WINDOW_POINTER_MOTION:
        case Event::WINDOW_POINTER_BUTTON_PRESS:
        case Event::WINDOW_POINTER_BUTTON_RELEASE:
        case Event::WINDOW_POINTER_WHEEL:
        {
            const int32_t xPos = event.pointer.x;
            const int32_t yPos = event.pointer.y;

            if( !getRenderContext( xPos, yPos, event.context ))
                LBVERB << "No rendering context for pointer event at "
                       << xPos << ", " << yPos << std::endl;
        }
    }
}


//======================================================================
// event methods
//======================================================================

EventOCommand Window::sendError( const uint32_t error )
{
    return getConfig()->sendError( Event::WINDOW_ERROR, Error( error, getID()));
}

bool Window::processEvent( const Event& event )
{
    // see comment in _updateEvent
    _updateEvent( const_cast< Event& >( event ));

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
                return true; // ignore
            // else fall through
        case Event::WINDOW_EXPOSE:
        case Event::WINDOW_CLOSE:
        case Event::STATISTIC:
        case Event::MAGELLAN_AXIS:
        case Event::MAGELLAN_BUTTON:
            break;

        case Event::WINDOW_POINTER_GRAB:
            _grabbedChannels = _getEventChannels( event.pointer );
            break;
        case Event::WINDOW_POINTER_UNGRAB:
            _grabbedChannels.clear();
            break;

        case Event::WINDOW_POINTER_MOTION:
        case Event::WINDOW_POINTER_BUTTON_PRESS:
        case Event::WINDOW_POINTER_BUTTON_RELEASE:
        case Event::WINDOW_POINTER_WHEEL:
        {
            const Channels& channels = _getEventChannels( event.pointer );
            for( Channels::const_iterator i = channels.begin();
                 i != channels.end(); ++i )
            {
                Channel* channel = *i;
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
                  case Event::WINDOW_POINTER_WHEEL:
                    channelEvent.type = Event::CHANNEL_POINTER_WHEEL;
                    break;
                  default:
                    LBWARN << "Unhandled window event of type " << event.type
                           << std::endl;
                    LBUNIMPLEMENTED;
                }

                // convert y to GL notation (Channel PVP uses GL coordinates)
                const PixelViewport& pvp = getPixelViewport();
                const int32_t y = pvp.h - event.pointer.y;
                const PixelViewport& channelPVP =
                    channel->getNativePixelViewport();

                channelEvent.originator = channel->getID();
                channelEvent.serial = channel->getSerial();
                channelEvent.pointer.x -= channelPVP.x;
                channelEvent.pointer.y = channelPVP.h - y + channelPVP.y;
                channel->processEvent( channelEvent );
            }
            break;
        }

        case Event::WINDOW_SCREENSAVER:
            switch( getIAttribute( WindowSettings::IATTR_HINT_SCREENSAVER ))
            {
                case OFF:
                    return true; // screen saver stays inactive
                case ON:
                    return false; // screen saver becomes active
                default: // AUTO
                    if( getDrawableConfig().doublebuffered &&
                        getIAttribute( WindowSettings::IATTR_HINT_DRAWABLE ) == WINDOW )
                    {
                        return true; // screen saver stays inactive
                    }
                    return false;
            }

        case Event::UNKNOWN:
            // unknown window-system native event, which was not handled
            return false;

        default:
            LBWARN << "Unhandled window event of type " << event.type
                   << std::endl;
            LBUNIMPLEMENTED;
    }

    Config* config = getConfig();
    ConfigEvent configEvent;
    configEvent.data = event;
    config->sendEvent( configEvent );
    return true;
}

Channels Window::_getEventChannels( const PointerEvent& event )
{
    if( !_grabbedChannels.empty( ))
        return _grabbedChannels;

    Channels result;
    const Channels& channels = getChannels();
    for( ChannelsCIter i = channels.begin(); i != channels.end(); ++i )
    {
        Channel* channel = *i;
        if( !channel->isDestination( ))
            continue;

        const PixelViewport& pvp = getPixelViewport();
        const PixelViewport& channelPVP = channel->getNativePixelViewport();

        // convert y to GL notation (Channel PVP uses GL coordinates)
        const int32_t y = pvp.h - event.y;

        if( channelPVP.isInside( event.x, y ))
            result.push_back( channel );
    }
    return result;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
bool Window::_cmdCreateChannel( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    const uint128_t& channelID = command.read< uint128_t >();

    LBLOG( LOG_INIT ) << "Create channel " << command  << " id " << channelID
                      << std::endl;

    Channel* channel = Global::getNodeFactory()->createChannel( this );
    channel->init(); // not in ctor, virtual method

    Config* config = getConfig();
    LBCHECK( config->mapObject( channel, channelID ));
    LBASSERT( channel->getSerial() != CO_INSTANCE_INVALID );

    return true;
}

bool Window::_cmdDestroyChannel( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    LBLOG( LOG_INIT ) << "Destroy channel " << command << std::endl;

    Channel* channel = _findChannel( command.read< uint128_t >( ));
    LBASSERT( channel );

    const bool stopped = channel->isStopped();
    Config* config = getConfig();
    config->unmapObject( channel );
    channel->send( getServer(), fabric::CMD_CHANNEL_CONFIG_EXIT_REPLY )
        << stopped;
    Global::getNodeFactory()->releaseChannel( channel );

    return true;
}

bool Window::_cmdConfigInit( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    LBLOG( LOG_INIT ) << "TASK window config init " << command << std::endl;

    bool result = false;
    if( getPipe()->isRunning( ))
    {
        _state = STATE_INITIALIZING;
        result = configInit( command.read< uint128_t >( ));
        if( result )
            _state = STATE_RUNNING;
    }
    else
        sendError( ERROR_WINDOW_PIPE_NOTRUNNING );

    LBLOG( LOG_INIT ) << "TASK window config init reply " << std::endl;

    commit();
    send( command.getRemoteNode(), fabric::CMD_WINDOW_CONFIG_INIT_REPLY ) << result;
    return true;
}

bool Window::_cmdConfigExit( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    LBLOG( LOG_INIT ) << "TASK window config exit " << command << std::endl;

    if( _state != STATE_STOPPED )
    {
        if( getPipe()->isRunning( ) && _systemWindow )
        {
            makeCurrent();
            getPipe()->flushFrames( _objectManager );
        }
        // else emergency exit, no context available.

        _state = configExit() ? STATE_STOPPED : STATE_FAILED;
    }

    getPipe()->send( getLocalNode(),
                     fabric::CMD_PIPE_DESTROY_WINDOW ) << getID();
    return true;
}

bool Window::_cmdFrameStart( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    LB_TS_THREAD( _pipeThread );

    const uint128_t& version = command.read< uint128_t >();
    const uint128_t& frameID = command.read< uint128_t >();
    const uint32_t frameNumber = command.read< uint32_t >();

    LBLOG( LOG_TASKS ) << "TASK frame start " << getName()
                       << " " << command << " frame " << frameNumber
                       << " id " << frameID << std::endl;

    //_grabFrame( frameNumber ); single-threaded
    sync( version );

    const DrawableConfig& drawableConfig = getDrawableConfig();
    if( drawableConfig.doublebuffered )
        _renderContexts[FRONT].swap( _renderContexts[BACK] );
    _renderContexts[BACK].clear();

    makeCurrent();
    frameStart( frameID, frameNumber );
    return true;
}

bool Window::_cmdFrameFinish( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    LBVERB << "handle window frame sync " << command << std::endl;

    const uint128_t& frameID = command.read< uint128_t >();
    const uint32_t frameNumber = command.read< uint32_t >();

    makeCurrent();
    frameFinish( frameID, frameNumber );
    return true;
}

bool Window::_cmdFlush( co::ICommand& )
{
    flush();
    return true;
}

bool Window::_cmdFinish( co::ICommand& )
{
    WindowStatistics stat( Statistic::WINDOW_FINISH, this );
    makeCurrent();
    finish();
    return true;
}

bool  Window::_cmdThrottleFramerate( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    LBLOG( LOG_TASKS ) << "TASK throttle framerate " << getName() << " "
                       << command << std::endl;

    // throttle to given framerate
    const int64_t elapsed  = getConfig()->getTime() - _lastSwapTime;
    const float minFrameTime = command.read< float >();
    const float timeLeft = minFrameTime - static_cast<float>( elapsed );

    if( timeLeft >= 1.f )
    {
        WindowStatistics stat( Statistic::WINDOW_THROTTLE_FRAMERATE, this );
        lunchbox::sleep( static_cast< uint32_t >( timeLeft ));
    }

    _lastSwapTime = getConfig()->getTime();
    return true;
}

bool Window::_cmdBarrier( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    const co::ObjectVersion& barrier = command.read< co::ObjectVersion >();

    LBVERB << "handle barrier " << command << " barrier " << barrier
           << std::endl;
    LBLOG( LOG_TASKS ) << "TASK swap barrier  " << getName() << std::endl;

    _enterBarrier( barrier );
    return true;
}

bool Window::_cmdNVBarrier( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    LBLOG( LOG_TASKS ) << "TASK join NV_swap_group" << std::endl;
    LBASSERT( _systemWindow );

    const co::ObjectVersion& netBarrier = command.read< co::ObjectVersion >();
    const uint32_t group = command.read< uint32_t >();
    const uint32_t barrier = command.read< uint32_t >();

    makeCurrent();
    _systemWindow->joinNVSwapBarrier( group, barrier );
    _enterBarrier( netBarrier );
    return true;
}

bool Window::_cmdSwap( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );

    LBLOG( LOG_TASKS ) << "TASK swap buffers " << getName() << " " << command
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

bool Window::_cmdFrameDrawFinish( co::ICommand& cmd )
{
    co::ObjectICommand command( cmd );
    const uint128_t& frameID = command.read< uint128_t >();
    const uint32_t frameNumber = command.read< uint32_t >();

    LBLOG( LOG_TASKS ) << "TASK draw finish " << getName() <<  " " << command
                       << " frame " << frameNumber << " id " << frameID
                       << std::endl;

    frameDrawFinish( frameID, frameNumber );
    return true;
}

}

#include <eq/fabric/window.ipp>
template class eq::fabric::Window< eq::Pipe, eq::Window, eq::Channel,
                                   eq::WindowSettings >;

/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                                                 const eq::Super& );
/** @endcond */
