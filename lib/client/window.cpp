
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include "configEvent.h"
#include "config.h"
#include "event.h"
#include "global.h"
#include "log.h"
#include "nodeFactory.h"
#include "pipe.h"
#include "osWindow.h"
#include "packets.h"
#include "server.h"
#include "task.h"
#include "windowStatistics.h"
#include "windowVisitor.h"

#ifdef AGL
#  include "aglWindow.h"
#endif
#ifdef GLX
#  include "glXWindow.h"
#endif
#ifdef WGL
#  include "wglWindow.h"
#endif

#include <eq/net/barrier.h>
#include <eq/net/command.h>
#include <eq/base/sleep.h>

using namespace eq::base;
using namespace std;

namespace eq
{

/** @cond IGNORE */
typedef net::CommandFunc<Window> WindowFunc;
/** @endcond */

#define MAKE_ATTR_STRING( attr ) ( string("EQ_WINDOW_") + #attr )
std::string Window::_iAttributeStrings[IATTR_ALL] = {
    MAKE_ATTR_STRING( IATTR_HINT_STEREO ),
    MAKE_ATTR_STRING( IATTR_HINT_DOUBLEBUFFER ),
    MAKE_ATTR_STRING( IATTR_HINT_FULLSCREEN ),
    MAKE_ATTR_STRING( IATTR_HINT_DECORATION ),
    MAKE_ATTR_STRING( IATTR_HINT_SWAPSYNC ),
    MAKE_ATTR_STRING( IATTR_HINT_DRAWABLE ),
    MAKE_ATTR_STRING( IATTR_HINT_STATISTICS ),
    MAKE_ATTR_STRING( IATTR_HINT_SCREENSAVER ),
    MAKE_ATTR_STRING( IATTR_PLANES_COLOR ),
    MAKE_ATTR_STRING( IATTR_PLANES_ALPHA ),
    MAKE_ATTR_STRING( IATTR_PLANES_DEPTH ),
    MAKE_ATTR_STRING( IATTR_PLANES_STENCIL ),
    MAKE_ATTR_STRING( IATTR_PLANES_ACCUM ),
    MAKE_ATTR_STRING( IATTR_PLANES_ACCUM_ALPHA ),
    MAKE_ATTR_STRING( IATTR_PLANES_SAMPLES ),
    MAKE_ATTR_STRING( IATTR_FILL1 ),
    MAKE_ATTR_STRING( IATTR_FILL2 )
};

Window::Window( Pipe* parent )
        : _pipe( parent )
        , _sharedContextWindow( 0 ) // default set by pipe
        , _osWindow( 0 )
        , _tasks( TASK_NONE )
        , _state( STATE_STOPPED )
        , _objectManager( 0 )
        , _lastTime ( 0.0 )
        , _avgFPS ( 0.0 )
        , _lastSwapTime( 0 )
{
    parent->_addWindow( this );
    EQINFO << " New eq::Window @" << (void*)this << endl;
}

Window::~Window()
{
    if( _pipe->isCurrent( this ))
        _pipe->setCurrent( 0 );

    _pipe->_removeWindow( this );

    delete _objectManager;
    _objectManager = 0;
}

void Window::attachToSession( const uint32_t id, 
                              const uint32_t instanceID, 
                              net::Session* session )
{
    net::Object::attachToSession( id, instanceID, session );

    net::CommandQueue* queue = _pipe->getPipeThreadQueue();

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


void Window::drawFPS() const
{
    ostringstream fpsText;
    fpsText << setprecision(3) << getFPS() << " FPS";

    const util::BitmapFont& font = getObjectManager()->getDefaultFont();
    const PixelViewport&    pvp  = getPixelViewport();

    glRasterPos3f( pvp.w - 60.f, 10.f , 0.99f );
    glColor3f( 1.f, 1.f, 1.f );

    font.draw( fpsText.str( ));
}

void Window::_addChannel( Channel* channel )
{
    EQASSERT( channel->getWindow() == this );
    _channels.push_back( channel );
}

void Window::_removeChannel( Channel* channel )
{
    ChannelVector::iterator i = find( _channels.begin(), _channels.end(), 
                                      channel );
    EQASSERT( i != _channels.end( ))
    
    _channels.erase( i );
}

Channel* Window::_findChannel( const uint32_t id )
{
    for( ChannelVector::const_iterator i = _channels.begin(); 
         i != _channels.end(); ++i )
    {
        Channel* channel = *i;
        if( channel->getID() == id )
            return channel;
    }
    return 0;
}

net::CommandQueue* Window::getPipeThreadQueue()
{ 
    EQASSERT( _pipe );
    return _pipe->getPipeThreadQueue(); 
}

const Node* Window::getNode() const 
{
    EQASSERT( _pipe );
    return ( _pipe ? _pipe->getNode() : 0 );
}
Node* Window::getNode()
{
    EQASSERT( _pipe );
    return ( _pipe ? _pipe->getNode() : 0 );
}

const Config* Window::getConfig() const
{
    EQASSERT( _pipe );
    return (_pipe ? _pipe->getConfig() : 0);
}
Config* Window::getConfig() 
{
    EQASSERT( _pipe );
    return (_pipe ? _pipe->getConfig() : 0);
}

ClientPtr Window::getClient()
{
    EQASSERT( _pipe );
    return ( _pipe ? _pipe->getClient() : 0 ); 
}
ServerPtr Window::getServer() 
{
    EQASSERT( _pipe );
    return ( _pipe ? _pipe->getServer() : 0 );
}

VisitorResult Window::accept( WindowVisitor& visitor )
{ 
    VisitorResult result = visitor.visitPre( this );
    if( result != TRAVERSE_CONTINUE )
        return result;

    for( ChannelVector::const_iterator i = _channels.begin(); 
         i != _channels.end(); ++i )
    {
        Channel* channel = *i;
        switch( channel->accept( visitor ))
        {
            case TRAVERSE_TERMINATE:
                return TRAVERSE_TERMINATE;

            case TRAVERSE_PRUNE:
                result = TRAVERSE_PRUNE;
                break;
                
            case TRAVERSE_CONTINUE:
            default:
                break;
        }
    }

    switch( visitor.visitPost( this ))
    {
        case TRAVERSE_TERMINATE:
            return TRAVERSE_TERMINATE;

        case TRAVERSE_PRUNE:
            return TRAVERSE_PRUNE;
                
        case TRAVERSE_CONTINUE:
        default:
            break;
    }

    return result;
}


//======================================================================
// pipe-thread methods
//======================================================================

//----------------------------------------------------------------------
// viewport
//----------------------------------------------------------------------
void Window::setPixelViewport( const PixelViewport& pvp )
{
    if( !_setPixelViewport( pvp ))
        return; // nothing changed

    WindowSetPVPPacket packet;
    packet.pvp = pvp;
    ServerPtr server = getServer();
    net::NodePtr node = server.get();
    send( node, packet );

    for( std::vector<Channel*>::iterator i = _channels.begin(); 
         i != _channels.end(); ++i )
    {
        (*i)->_notifyViewportChanged();
    }
}

bool Window::_setPixelViewport( const PixelViewport& pvp )
{
    if( pvp == _pvp || !pvp.isValid( ))
        return false;

    _pvp = pvp;
    _vp.invalidate();

    EQASSERT( _pipe );
    
    const PixelViewport& pipePVP = _pipe->getPixelViewport();
    if( pipePVP.isValid( ))
        _vp = pvp.getSubVP( pipePVP );

    EQINFO << "Window pvp set: " << _pvp << ":" << _vp << endl;
    return true;
}

void Window::_setViewport( const Viewport& vp )
{
    if( !vp.hasArea( ))
        return;
    
    _vp = vp;
    _pvp.invalidate();

    if( !_pipe )
        return;

    PixelViewport pipePVP = _pipe->getPixelViewport();
    if( pipePVP.isValid( ))
        _pvp = pipePVP.getSubPVP( vp );
    EQINFO << "Window vp set: " << _pvp << ":" << _vp << endl;
}

//----------------------------------------------------------------------
// render context
//----------------------------------------------------------------------
void Window::addRenderContext( const RenderContext& context )
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

    vector< RenderContext >::const_reverse_iterator i   =
        _renderContexts[which].rbegin(); 
    vector< RenderContext >::const_reverse_iterator end =
        _renderContexts[which].rend();

    const int32_t glY = _pvp.h - y; // invert y to follow GL convention

    for( ; i != end; ++i )
    {
        const RenderContext& candidate = *i;
        if( candidate.pvp.isPointInside( x, glY ))
        {
            context = candidate;
            return true;
        }
    }
    return false;
}

void Window::setIAttribute( const IAttribute attr, const int32_t value )
{
    _iAttributes[attr] = value;
}

int32_t  Window::getIAttribute( const IAttribute attr ) const
{
    return _iAttributes[attr];
}

const std::string&  Window::getIAttributeString( const IAttribute attr )
{
    return _iAttributeStrings[attr];
}

uint32_t Window::getColorType()
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
    if( _osWindow )
        _osWindow->exitGLEW();

    _osWindow = window;

    if( !window )
        return;

    // Initialize context-specific data
    makeCurrent();
    _osWindow->initGLEW();
    _queryDrawableConfig();
    _setupObjectManager();
}

const OSPipe* Window::getOSPipe() const
{
    EQASSERT( _pipe );
    return _pipe->getOSPipe();
}

OSPipe* Window::getOSPipe()
{
    EQASSERT( _pipe );
    return _pipe->getOSPipe();
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
    if( !_pvp.isValid( ))
    {
        setErrorMessage( "Window pixel viewport invalid - pipe init failed?" );
        return false;
    }

    EQASSERT( !_osWindow );

    if( !configInitOSWindow( initID )) return false;
    if( !configInitGL( initID ))       return false;

    EQ_GL_ERROR( "after Window::configInitGL" );
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
                    << " not implemented or supported" << endl;
            return false;
    }

    EQASSERT( osWindow );
    if( !osWindow->configInit( ))
    {
        EQWARN << "OS Window initialization failed: " << _error << endl;
        delete osWindow;
        return false;
    }

    setOSWindow( osWindow );
    return true;
}

void Window::_queryDrawableConfig()
{
    // GL version
    const char* glVersion = (const char*)glGetString( GL_VERSION );
    if( !glVersion ) // most likely no context - fail
    {
        EQWARN << "glGetString(GL_VERSION) returned 0, assuming GL version 1.1" 
            << endl;
        _drawableConfig.glVersion = 1.1f;
    }
    else
        _drawableConfig.glVersion = static_cast<float>( atof( glVersion ));

    // Framebuffer capabilities
    GLboolean result;
    glGetBooleanv( GL_STEREO,       &result );
    _drawableConfig.stereo = result;

    glGetBooleanv( GL_DOUBLEBUFFER, &result );
    _drawableConfig.doublebuffered = result;

    GLint stencilBits;
    glGetIntegerv( GL_STENCIL_BITS, &stencilBits );
    _drawableConfig.stencilBits = stencilBits;

    GLint alphaBits;
    glGetIntegerv( GL_ALPHA_BITS, &alphaBits );
    _drawableConfig.alphaBits = alphaBits;

    EQINFO << "Window drawable config: " << _drawableConfig << endl;
}

void Window::_setupObjectManager()
{
    _releaseObjectManager();

    Window*    sharedWindow = getSharedContextWindow();
    ObjectManager* sharedOM = sharedWindow ? sharedWindow->getObjectManager():0;

    if( sharedOM )
        _objectManager = new ObjectManager( this, sharedOM );
    else
        _objectManager = new ObjectManager( this );

    _objectManager->_smallFont.initFont();
    _objectManager->_mediumFont.initFont( util::BitmapFont::normal, 20 );
}

void Window::_releaseObjectManager()
{
    if( _objectManager && _objectManager->getSharedUsage() == 1 )
        _objectManager->deleteAll();

    delete _objectManager;
    _objectManager = 0;
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

    if( _pipe->isCurrent( this ))
        _pipe->setCurrent( 0 );

    return true;
}

void Window::makeCurrent( const bool useCache ) const
{
    if( useCache && _pipe->isCurrent( this ))
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
    EQVERB << "----- SWAP -----" << endl;
}

GLEWContext* Window::glewGetContext()
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

void Window::setErrorMessage( const std::string& message )
{
    _error = message;
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
                    if( _drawableConfig.doublebuffered &&
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
                   << endl;
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
    EQLOG( LOG_INIT ) << "Create channel " << packet << endl;

    Channel* channel = Global::getNodeFactory()->createChannel( this );
    getConfig()->attachObject( channel, packet->channelID, EQ_ID_INVALID );

    return net::COMMAND_HANDLED;
}

net::CommandResult Window::_cmdDestroyChannel( net::Command& command ) 
{
    const WindowDestroyChannelPacket* packet =
        command.getPacket<WindowDestroyChannelPacket>();
    EQLOG( LOG_INIT ) << "Destroy channel " << packet << endl;

    Channel* channel = _findChannel( packet->channelID );
    EQASSERT( channel )

    Config*  config  = getConfig();
    config->detachObject( channel );
    Global::getNodeFactory()->releaseChannel( channel );

    return net::COMMAND_HANDLED;
}

net::CommandResult Window::_cmdConfigInit( net::Command& command )
{
    const WindowConfigInitPacket* packet = 
        command.getPacket<WindowConfigInitPacket>();
    EQLOG( LOG_INIT ) << "TASK window config init " << packet << endl;

    WindowConfigInitReplyPacket reply;
    _error.clear();

    if( _pipe->isRunning( ))
    {
        _state = STATE_INITIALIZING;
        if( packet->pvp.isValid( ))
            _setPixelViewport( packet->pvp );
        else
            _setViewport( packet->vp );
        
        _name  = packet->name;
        _tasks = packet->tasks;
        
        memcpy( _iAttributes, packet->iAttributes, IATTR_ALL * sizeof(int32_t));

        reply.result = configInit( packet->initID );
    }
    else
        reply.result = false;

    EQLOG( LOG_INIT ) << "TASK window config init reply " << &reply << endl;

    net::NodePtr node = command.getNode();
    if( !reply.result )
    {
        send( node, reply, _error );
        return net::COMMAND_HANDLED;
    }

    reply.pvp            = _pvp;
    reply.drawableConfig = getDrawableConfig();
    send( node, reply );

    _state = STATE_RUNNING;
    return net::COMMAND_HANDLED;
}

net::CommandResult Window::_cmdConfigExit( net::Command& command )
{
    const WindowConfigExitPacket* packet =
        command.getPacket<WindowConfigExitPacket>();
    EQLOG( LOG_INIT ) << "TASK window config exit " << packet << endl;

    WindowConfigExitReplyPacket reply;
    
    if( _state == STATE_STOPPED )
        reply.result = true;
    else
    {
        if( _pipe->isRunning( ) && _osWindow )
        {
            EQ_GL_CALL( makeCurrent( ));
            _pipe->flushFrames();
        }
        // else emergency exit, no context available.

        reply.result = configExit();
    }

    send( command.getNode(), reply );
    _state = STATE_STOPPED;
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

    EQASSERT( _osWindow );
    const DrawableConfig& drawableConfig = getDrawableConfig();
    if( drawableConfig.doublebuffered )
        _renderContexts[FRONT].swap( _renderContexts[BACK] );
    _renderContexts[BACK].clear();

    EQ_GL_CALL( makeCurrent( ));

    frameStart( packet->frameID, packet->frameNumber );
    return net::COMMAND_HANDLED;
}

net::CommandResult Window::_cmdFrameFinish( net::Command& command )
{
    const WindowFrameFinishPacket* packet =
        command.getPacket<WindowFrameFinishPacket>();
    EQVERB << "handle window frame sync " << packet << endl;

    EQ_GL_CALL( makeCurrent( ));
    frameFinish( packet->frameID, packet->frameNumber );
    return net::COMMAND_HANDLED;
}

net::CommandResult Window::_cmdFinish(net::Command& command ) 
{
    EQ_GL_CALL( makeCurrent( ));

    WindowStatistics stat( Statistic::WINDOW_FINISH, this );
    finish();

    return net::COMMAND_HANDLED;
}

net::CommandResult  Window::_cmdThrottleFramerate( net::Command& command )
{
    WindowThrottleFramerate* packet = 
        command.getPacket< WindowThrottleFramerate >();
    EQLOG( LOG_TASKS ) << "TASK throttle framerate " << getName() << " "
                       << packet << endl;

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
    EQVERB << "handle barrier " << packet << endl;
    EQLOG( LOG_TASKS ) << "TASK swap barrier  " << getName() << endl;
    
    _enterBarrier( packet->barrier );
    return net::COMMAND_HANDLED;
}

net::CommandResult Window::_cmdNVBarrier( net::Command& command )
{
    const WindowNVBarrierPacket* packet = 
        command.getPacket<WindowNVBarrierPacket>();
    EQLOG( LOG_TASKS ) << "TASK join NV_swap_group" << endl;
    
    EQASSERT( _osWindow );
    _osWindow->joinNVSwapBarrier( packet->group, packet->barrier );

    _enterBarrier( packet->netBarrier );
    return net::COMMAND_HANDLED;
}

net::CommandResult Window::_cmdSwap( net::Command& command ) 
{
    WindowSwapPacket* packet = command.getPacket< WindowSwapPacket >();
    EQLOG( LOG_TASKS ) << "TASK swap buffers " << getName() << " " << packet
                       << endl;

    if( _drawableConfig.doublebuffered )
    {
        // swap
        WindowStatistics stat( Statistic::WINDOW_SWAP, this );
        EQ_GL_CALL( makeCurrent( ));
        swapBuffers();
    }
    return net::COMMAND_HANDLED;
}

net::CommandResult Window::_cmdFrameDrawFinish( net::Command& command )
{
    WindowFrameDrawFinishPacket* packet = 
        command.getPacket< WindowFrameDrawFinishPacket >();
    EQLOG( LOG_TASKS ) << "TASK draw finish " << getName() <<  " " << packet
                       << endl;

    frameDrawFinish( packet->frameID, packet->frameNumber );
    return net::COMMAND_HANDLED;
}

}
