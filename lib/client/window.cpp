
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "window.h"

#include "channel.h"
#include "commands.h"
#include "configEvent.h"
#include "event.h"
#include "event.h"
#include "global.h"
#include "log.h"
#include "nodeFactory.h"
#include "osWindow.h"
#include "packets.h"
#include "task.h"
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

#include <eq/net/barrier.h>
#include <eq/net/command.h>
#include <eq/base/sleep.h>

using namespace eq::base;
using namespace std;
using eq::net::CommandFunc;

namespace eq
{

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
    MAKE_ATTR_STRING( IATTR_PLANES_SAMPLES )
};

Window::Window( Pipe* parent )
        : _sharedContextWindow( 0 ) // default set by pipe
        , _osWindow( 0 )
        , _pipe( parent )
        , _tasks( TASK_NONE )
        , _lastSwapTime( 0 )
{
    parent->_addWindow( this );
    EQINFO << " New eq::Window @" << (void*)this << endl;
}

Window::~Window()
{
    _pipe->_removeWindow( this );
}

void Window::attachToSession( const uint32_t id, 
                              const uint32_t instanceID, 
                              net::Session* session )
{
    net::Object::attachToSession( id, instanceID, session );

    net::CommandQueue* queue = _pipe->getPipeThreadQueue();

    registerCommand( CMD_WINDOW_CREATE_CHANNEL, 
                     CommandFunc<Window>( this, &Window::_cmdCreateChannel ), 
                     queue );
    registerCommand( CMD_WINDOW_DESTROY_CHANNEL,
                     CommandFunc<Window>( this, &Window::_cmdDestroyChannel ), 
                     queue );
    registerCommand( CMD_WINDOW_CONFIG_INIT,
                     CommandFunc<Window>( this, &Window::_cmdConfigInit ), 
                     queue );
    registerCommand( CMD_WINDOW_CONFIG_EXIT, 
                     CommandFunc<Window>( this, &Window::_cmdConfigExit ), 
                     queue );
    registerCommand( CMD_WINDOW_FRAME_START,
                     CommandFunc<Window>( this, &Window::_cmdFrameStart ), 
                     queue );
    registerCommand( CMD_WINDOW_FRAME_FINISH,
                     CommandFunc<Window>( this, &Window::_cmdFrameFinish ), 
                     queue );
    registerCommand( CMD_WINDOW_FINISH, 
                     CommandFunc<Window>( this, &Window::_cmdFinish), queue );
    registerCommand( CMD_WINDOW_BARRIER, 
                     CommandFunc<Window>( this, &Window::_cmdBarrier ), queue );
    registerCommand( CMD_WINDOW_SWAP, 
                     CommandFunc<Window>( this, &Window::_cmdSwap), queue );
    registerCommand( CMD_WINDOW_FRAME_DRAW_FINISH, 
                     CommandFunc<Window>( this, &Window::_cmdFrameDrawFinish ), 
                     queue );
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

WindowVisitor::Result Window::accept( WindowVisitor* visitor )
{ 
    ChannelVisitor::Result result = visitor->visitPre( this );
    if( result != ChannelVisitor::TRAVERSE_CONTINUE )
        return result;

    for( ChannelVector::const_iterator i = _channels.begin(); 
         i != _channels.end(); ++i )
    {
        Channel* channel = *i;
        switch( channel->accept( visitor ))
        {
            case ChannelVisitor::TRAVERSE_TERMINATE:
                return ChannelVisitor::TRAVERSE_TERMINATE;

            case ChannelVisitor::TRAVERSE_PRUNE:
                result = ChannelVisitor::TRAVERSE_PRUNE;
                break;
                
            case ChannelVisitor::TRAVERSE_CONTINUE:
            default:
                break;
        }
    }

    switch( visitor->visitPost( this ))
    {
        case NodeVisitor::TRAVERSE_TERMINATE:
	  return NodeVisitor::TRAVERSE_TERMINATE;

        case NodeVisitor::TRAVERSE_PRUNE:
	  return NodeVisitor::TRAVERSE_PRUNE;
	  break;
                
        case NodeVisitor::TRAVERSE_CONTINUE:
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
    net::NodePtr node = RefPtr_static_cast< Server, net::Node >( getServer( ));
    send( node, packet );

    for( std::vector<Channel*>::iterator i = _channels.begin(); 
         i != _channels.end(); ++i )

        (*i)->_notifyViewportChanged();
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
    // AGL events are dispatched from the main thread, which therefore
    // potentially accesses _renderContexts concurrently with the pipe
    // thread. The _osWindow->getContextLock( ) is only active for AGL windows.
    // The proper solution would be to implement our own event queue which
    // dispatches events from the main to the pipe thread, to be handled
    // there. Since the event dispatch is buried in the messagePump, using this
    // lock is easier. Eventually we should implement it, though.
    EQASSERT( _osWindow );
    ScopedMutex< SpinLock > mutex( _osWindow->getContextLock( ));
    _renderContexts[BACK].push_back( context );
}

const RenderContext* Window::getRenderContext( const int32_t x, 
                                               const int32_t y ) const
{
    if( !_osWindow )
        return 0;

    ScopedMutex< SpinLock > mutex( _osWindow->getContextLock( ));

    const DrawableConfig& drawableConfig = getDrawableConfig();
    const unsigned which = drawableConfig.doublebuffered ? FRONT : BACK;

    vector< RenderContext >::const_reverse_iterator i   =
        _renderContexts[which].rbegin(); 
    vector< RenderContext >::const_reverse_iterator end =
        _renderContexts[which].rend();

    for( ; i != end; ++i )
    {
        const RenderContext& context = *i;

        if( context.pvp.isPointInside( x, y ))
            return &context;
    }
    return 0;
}

void Window::setOSWindow( OSWindow* window )
{
    _osWindow = window;

    if( !window )
        return;

    // Initialize context-specific data
    makeCurrent();

    _queryDrawableConfig();
    _setupObjectManager();
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
        EQWARN << "OS Window initialization failed" << endl;
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

    Window* sharedWindow = getSharedContextWindow();
    if( sharedWindow )
        _objectManager = sharedWindow->getObjectManager();

    if( !_objectManager.isValid( ))
    {
        _objectManager = new ObjectManager( this );
        _objectManager->_font.initFont();
    }
}

void Window::_releaseObjectManager()
{
    if( _objectManager.isValid() && _objectManager->getRefCount() == 1 )
        _objectManager->deleteAll();

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
    return true;
}

void Window::makeCurrent() const
{
    _osWindow->makeCurrent( );
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

//======================================================================
// event-handler methods
//======================================================================
bool Window::processEvent( const Event& event )
{
    ConfigEvent configEvent;
    switch( event.type )
    {
        case Event::WINDOW_HIDE:
            EQINFO << event << endl;
            setPixelViewport( PixelViewport( 0, 0, 0, 0 ));
            break;

        case Event::WINDOW_SHOW:
        case Event::WINDOW_RESIZE:
            EQINFO << event << endl;
            setPixelViewport( PixelViewport( event.resize.x, event.resize.y, 
                                             event.resize.w, event.resize.h ));
            break;

        case Event::KEY_PRESS:
        case Event::KEY_RELEASE:
            if( event.key.key == KC_VOID )
                return true; //ignore
            // else fall through
        case Event::EXPOSE:
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
    EQINFO << "Handle create channel " << packet << endl;

    Channel* channel = Global::getNodeFactory()->createChannel( this );
    getConfig()->attachObject( channel, packet->channelID );

    return net::COMMAND_HANDLED;
}

net::CommandResult Window::_cmdDestroyChannel( net::Command& command ) 
{
    const WindowDestroyChannelPacket* packet =
        command.getPacket<WindowDestroyChannelPacket>();
    EQINFO << "Handle destroy channel " << packet << endl;

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
    EQLOG( LOG_TASKS ) << "TASK window config init " << packet << endl;

    if( packet->pvp.isValid( ))
        _setPixelViewport( packet->pvp );
    else
        _setViewport( packet->vp );

    _name  = packet->name;
    _tasks = packet->tasks;
    memcpy( _iAttributes, packet->iAttributes, IATTR_ALL * sizeof( int32_t ));
    _error.clear();

    WindowConfigInitReplyPacket reply;
    reply.result = configInit( packet->initID );
    EQLOG( LOG_TASKS ) << "TASK window config init reply " << &reply << endl;

    net::NodePtr node = command.getNode();
    if( !reply.result )
    {
        send( node, reply, _error );
        return net::COMMAND_HANDLED;
    }

    reply.pvp            = _pvp;
    reply.drawableConfig = getDrawableConfig();
    send( node, reply );
    return net::COMMAND_HANDLED;
}

net::CommandResult Window::_cmdConfigExit( net::Command& command )
{
    const WindowConfigExitPacket* packet =
        command.getPacket<WindowConfigExitPacket>();
    EQLOG( LOG_TASKS ) << "TASK configExit " << getName() <<  " " << packet 
                       << endl;

    if( _pipe->isInitialized( ) && _osWindow )
        EQ_GL_CALL( makeCurrent( ));
    // else emergency exit, no context available.

    WindowConfigExitReplyPacket reply;
    reply.result = configExit();

    send( command.getNode(), reply );

    return net::COMMAND_HANDLED;
}

net::CommandResult Window::_cmdFrameStart( net::Command& command )
{
    const WindowFrameStartPacket* packet = 
        command.getPacket<WindowFrameStartPacket>();
    EQVERB << "handle window frame start " << packet << endl;

    //_grabFrame( packet->frameNumber ); single-threaded

    {
        EQASSERT( _osWindow );
        ScopedMutex< SpinLock > mutex( _osWindow->getContextLock( ));

        const DrawableConfig& drawableConfig = getDrawableConfig();
        if( drawableConfig.doublebuffered )
            _renderContexts[FRONT].swap( _renderContexts[BACK] );
        _renderContexts[BACK].clear();
    }
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

net::CommandResult Window::_cmdBarrier( net::Command& command )
{
    const WindowBarrierPacket* packet = 
        command.getPacket<WindowBarrierPacket>();
    EQVERB << "handle barrier " << packet << endl;
    EQLOG( LOG_TASKS ) << "TASK swap barrier  " << getName() << endl;
    EQLOG( net::LOG_BARRIER ) << "swap barrier " << packet->barrierID
                                << " v" << packet->barrierVersion <<endl;
    
    Node*           node    = getNode();
    net::Barrier* barrier = node->getBarrier( packet->barrierID, 
                                              packet->barrierVersion );

    WindowStatistics stat( Statistic::WINDOW_SWAP_BARRIER, this );
    barrier->enter();
    return net::COMMAND_HANDLED;
}

net::CommandResult Window::_cmdSwap( net::Command& command ) 
{
    WindowSwapPacket* packet = command.getPacket< WindowSwapPacket >();
    EQLOG( LOG_TASKS ) << "TASK swap buffers " << getName() << " " << packet
                       << endl;

    // throttle to given framerate
    const int64_t elapsed  = getConfig()->getTime() - _lastSwapTime;
    const float   timeLeft = packet->minFrameTime - 
                             static_cast< float >( elapsed );
    if( timeLeft >= 1.f )
        base::sleep( static_cast< uint32_t >( timeLeft ));

    _lastSwapTime = getConfig()->getTime();

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

std::ostream& operator << ( std::ostream& os,
                            const Window::DrawableConfig& config )
{
    os << "GL" << config.glVersion;
    if( config.stereo )
        os << "|ST";
    if( config.doublebuffered )
        os << "|DB";
    if( config.stencilBits )
        os << "|st" << config.stencilBits;
    if( config.alphaBits )
        os << "|a" << config.alphaBits;
    return os;
}
}
