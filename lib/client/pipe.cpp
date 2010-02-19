
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

#include "pipe.h"

#include "client.h"
#include "commands.h"
#include "config.h"
#include "frame.h"
#include "global.h"
#include "log.h"
#include "nodeFactory.h"
#include "pipeStatistics.h"
#include "pipeVisitor.h"
#include "packets.h"
#include "server.h"
#include "task.h"
#include "view.h"
#include "X11Connection.h"
#include "window.h"

#ifdef GLX
#  include "glXMessagePump.h"
#  include "glXPipe.h"
#endif
#ifdef WGL
#  include "wglMessagePump.h"
#  include "wglPipe.h"
#endif
#ifdef AGL
#  include "aglMessagePump.h"
#  include "aglPipe.h"
#endif

#include "computeContext.h"
#ifdef EQ_USE_CUDA
#  include "cudaContext.h"
#endif

#include <eq/net/command.h>
#include <sstream>

namespace eq
{
namespace
{
static const Window* _ntCurrentWindow = 0;
}

/** @cond IGNORE */
typedef net::CommandFunc<Pipe> PipeFunc;
/** @endcond */

Pipe::Pipe( Node* parent )
        : _osPipe( 0 )
        , _node( parent )
        , _windowSystem( WINDOW_SYSTEM_NONE )
        , _tasks( TASK_NONE )
        , _port( EQ_UNDEFINED_UINT32 )
        , _device( EQ_UNDEFINED_UINT32 )
        , _state( STATE_STOPPED )
        , _currentFrame( 0 )
        , _frameTime( 0 )
        , _waitTime( 0 )
        , _thread( 0 )
        , _pipeThreadQueue( 0 )
        , _currentWindow( 0 )
		, _computeContext( 0 )
{
    parent->_addPipe( this );
    EQINFO << " New eq::Pipe @" << (void*)this << std::endl;
}

Pipe::~Pipe()
{
    _node->_removePipe( this );
    delete _thread;
    _thread = 0;
}

Config* Pipe::getConfig()
{
    EQASSERT( _node );
    return (_node ? _node->getConfig() : 0);
}
const Config* Pipe::getConfig() const
{
    EQASSERT( _node );
    return (_node ? _node->getConfig() : 0);
}

ClientPtr Pipe::getClient()
{
    EQASSERT( _node );
    return (_node ? _node->getClient() : 0);
}

ServerPtr Pipe::getServer()
{
    EQASSERT( _node );
    return (_node ? _node->getServer() : 0);
}

int64_t Pipe::getFrameTime() const
{
    return getConfig()->getTime() - _frameTime;
}

namespace
{
template< class C >
VisitorResult _accept( C* pipe, PipeVisitor& visitor )
{ 
    VisitorResult result = visitor.visitPre( pipe );
    if( result != TRAVERSE_CONTINUE )
        return result;

    const WindowVector& windows = pipe->getWindows();
    for( WindowVector::const_iterator i = windows.begin(); 
         i != windows.end(); ++i )
    {
        switch( (*i)->accept( visitor ))
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

    switch( visitor.visitPost( pipe ))
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
}

VisitorResult Pipe::accept( PipeVisitor& visitor )
{
    return _accept( this, visitor );
}

VisitorResult Pipe::accept( PipeVisitor& visitor ) const
{
    return _accept( this, visitor );
}

void Pipe::attachToSession( const uint32_t id, const uint32_t instanceID, 
                            net::Session* session )
{
    net::Object::attachToSession( id, instanceID, session );
    
    net::CommandQueue* queue = getPipeThreadQueue();

    registerCommand( CMD_PIPE_CONFIG_INIT, 
                     PipeFunc( this, &Pipe::_cmdConfigInit ), queue );
    registerCommand( CMD_PIPE_CONFIG_EXIT, 
                     PipeFunc( this, &Pipe::_cmdConfigExit ), queue );
    registerCommand( CMD_PIPE_CREATE_WINDOW,
                     PipeFunc( this, &Pipe::_cmdCreateWindow ), queue );
    registerCommand( CMD_PIPE_DESTROY_WINDOW, 
                     PipeFunc( this, &Pipe::_cmdDestroyWindow ), queue );
    registerCommand( CMD_PIPE_FRAME_START,
                     PipeFunc( this, &Pipe::_cmdFrameStart ), queue );
    registerCommand( CMD_PIPE_FRAME_FINISH,
                     PipeFunc( this, &Pipe::_cmdFrameFinish ), queue );
    registerCommand( CMD_PIPE_FRAME_DRAW_FINISH, 
                     PipeFunc( this, &Pipe::_cmdFrameDrawFinish ), queue );
    registerCommand( CMD_PIPE_FRAME_START_CLOCK,
                     PipeFunc( this, &Pipe::_cmdFrameStartClock ), 0 );
}

void Pipe::_addWindow( Window* window )
{
    EQASSERT( window->getPipe() == this );
    _windows.push_back( window );
}

void Pipe::_removeWindow( Window* window )
{
    WindowVector::iterator iter = find( _windows.begin(), _windows.end(),
                                        window );
    EQASSERT( iter != _windows.end( ))
    
    _windows.erase( iter );
}

eq::Window* Pipe::_findWindow( const uint32_t id )
{
    for( WindowVector::const_iterator i = _windows.begin(); 
         i != _windows.end(); ++i )
    {
        Window* window = *i;
        if( window->getID() == id )
            return window;
    }
    return 0;
}

bool Pipe::supportsWindowSystem( const WindowSystem windowSystem ) const
{
    switch( windowSystem )
    {
#ifdef GLX
        case WINDOW_SYSTEM_GLX: return true;
#endif
#ifdef AGL
        case WINDOW_SYSTEM_AGL: return true;
#endif
#ifdef WGL
        case WINDOW_SYSTEM_WGL: return true;
#endif
        default:                return false;
    }
}

WindowSystem Pipe::selectWindowSystem() const
{
    for( WindowSystem i=WINDOW_SYSTEM_NONE; i<WINDOW_SYSTEM_ALL; 
         i = (WindowSystem)((int)i+1) )
    {
        if( supportsWindowSystem( i ))
            return i;
    }
    EQABORT( "No supported window system found" );
    return WINDOW_SYSTEM_NONE;
}

void Pipe::_setupCommandQueue()
{
#ifdef EQ_USE_DEPRECATED
    if( !useMessagePump( ))
        return;
#endif

    EQASSERT( _windowSystem != WINDOW_SYSTEM_NONE );
    EQINFO << "Set up pipe message pump for " << _windowSystem << std::endl;

    Config* config = getConfig();
    config->setupMessagePump( this );

    // Non-threaded pipes have no pipe thread message pump
    if( !_thread )
        return;
    
    EQASSERT( _pipeThreadQueue );
    EQASSERT( !_pipeThreadQueue->getMessagePump( ));

    _pipeThreadQueue->setMessagePump( createMessagePump( ));
}

void Pipe::_exitCommandQueue()
{
    // Non-threaded pipes have no pipe thread message pump
    if( !_thread )
        return;
    
    EQASSERT( _pipeThreadQueue );

    MessagePump* pump = _pipeThreadQueue->getMessagePump();

    _pipeThreadQueue->setMessagePump( 0 );
    delete pump;
}

MessagePump* Pipe::createMessagePump()
{
    switch( _windowSystem )
    {
#ifdef GLX
        case WINDOW_SYSTEM_GLX:
            return new GLXMessagePump();
#endif
#ifdef WGL
        case WINDOW_SYSTEM_WGL:
            return new WGLMessagePump();
#endif
#ifdef AGL
        case WINDOW_SYSTEM_AGL:
            return new AGLMessagePump();
#endif

        default:
            EQUNREACHABLE;
            return 0;
    }
}

void Pipe::_runThread()
{
    EQINFO << "Entered pipe thread" << std::endl;
    base::Thread::setDebugName( typeid( *this ).name( ));
    CHECK_THREAD( _pipeThread );

    Config* config = getConfig();
    EQASSERT( config );
    EQASSERT( _pipeThreadQueue );

    while( _thread->isRunning( ))
    {
        const int64_t startWait = config->getTime();
        net::Command* command = _pipeThreadQueue->pop();
        _waitTime += ( config->getTime() - startWait );

        switch( config->invokeCommand( *command ))
        {
            case net::COMMAND_HANDLED:
            case net::COMMAND_DISCARD:
                break;

            case net::COMMAND_ERROR:
                EQABORT( "Error handling command packet" );
                break;

            default:
                EQABORT( "Unknown command result" );
                break;
        }
        command->release();
    }

    EQUNREACHABLE; // since we are exited from _cmdConfigExit
}

net::CommandQueue* Pipe::getPipeThreadQueue()
{
    if( !_thread )
        return _node->getNodeThreadQueue();

    return _pipeThreadQueue;
}

Frame* Pipe::getFrame( const net::ObjectVersion& frameVersion, const Eye eye )
{
    Frame* frame = _frames[ frameVersion.identifier ];

    if( !frame )
    {
        net::Session* session = getSession();
        frame = new Frame();
        
        EQCHECK( session->mapObject( frame, frameVersion.identifier ));
        _frames[ frameVersion.identifier ] = frame;
    }
    
    frame->sync( frameVersion.version );

    const net::ObjectVersion& data = frame->getDataVersion( eye );
    EQASSERT( data.identifier != EQ_ID_INVALID );
    FrameData* frameData = getNode()->getFrameData( data ); 
    EQASSERT( frameData );

    frame->setData( frameData );
    return frame;
}

void Pipe::flushFrames()
{
    net::Session* session = getSession();

    for( FrameHash::const_iterator i = _frames.begin(); i != _frames.end(); ++i)
    {
        Frame* frame = i->second;

        frame->flush();
        session->unmapObject( frame );
        delete frame;
    }
    _frames.clear();
}

const View* Pipe::getView( const net::ObjectVersion& viewVersion ) const
{
    // Yie-ha: we want to have a const-interface to get a view on the render
    //         clients, but view mapping is by definition non-const.
    return const_cast< Pipe* >( this )->getView( viewVersion );
}

View* Pipe::getView( const net::ObjectVersion& viewVersion )
{
    CHECK_THREAD( _pipeThread );
    if( viewVersion.identifier == EQ_ID_INVALID )
        return 0;

    View* view = _views[ viewVersion.identifier ];

    if( !view )
    {
        NodeFactory* nodeFactory = Global::getNodeFactory();
        view = nodeFactory->createView();
        view->_pipe = this;

        net::Session* session = const_cast< net::Session* >( getSession( ));
        EQCHECK( session->mapObject( view, viewVersion.identifier ));

        _views[ viewVersion.identifier ] = view;
    }
    
    view->sync( viewVersion.version );
    return view;
}

void Pipe::_releaseViews()
{
    CHECK_THREAD( _pipeThread );
    for( bool changed = true; changed; )
    {
        changed = false;
        for( ViewHash::iterator i = _views.begin(); 
             i != _views.end(); ++i )
        {
            View* view = i->second;
            if( view->getVersion() + 20 > view->getHeadVersion( ))
                continue;

            // release view to avoid memory leaks due to deltas piling up.
            net::Session* session = getSession();
            session->unmapObject( view );
            
            _views.erase( i );

            NodeFactory* nodeFactory = Global::getNodeFactory();
            nodeFactory->releaseView( view );

            changed = true;
            break;
        }
    }
}

void Pipe::_flushViews()
{
    CHECK_THREAD( _pipeThread );
    NodeFactory*  nodeFactory = Global::getNodeFactory();
    net::Session* session     = getSession();

    for( ViewHash::const_iterator i = _views.begin(); i != _views.end(); ++i)
    {
        View* view = i->second;

        session->unmapObject( view );
        nodeFactory->releaseView( view );
    }
    _views.clear();
}

bool Pipe::isCurrent( const Window* window ) const
{
    if( isThreaded( ))
        return ( window == _currentWindow );
    return ( window == _ntCurrentWindow );
}

void Pipe::setCurrent( const Window* window ) const
{
    if( isThreaded( ))
        _currentWindow = window;
    else
        _ntCurrentWindow = window;
}

void Pipe::startThread()
{
    _pipeThreadQueue = new CommandQueue;
    _thread          = new PipeThread( this );

    _thread->start();
}

void Pipe::joinThread()
{
    if( !_thread )
        return;

    _thread->join();
    delete _thread;
    _thread = 0;

    delete _pipeThreadQueue;
    _pipeThreadQueue = 0;
}

void Pipe::waitExited() const
{
    _state.waitEQ( STATE_STOPPED );
}

bool Pipe::isRunning() const
{
    return (_state == STATE_RUNNING);
}

void Pipe::waitFrameFinished( const uint32_t frameNumber ) const
{
    _finishedFrame.waitGE( frameNumber );
}

void Pipe::waitFrameLocal( const uint32_t frameNumber ) const
{
    _unlockedFrame.waitGE( frameNumber );
}

uint32_t Pipe::getFinishedFrame() const
{
    return _finishedFrame.get();
}


//---------------------------------------------------------------------------
// pipe-thread methods
//---------------------------------------------------------------------------
bool Pipe::configInit( const uint32_t initID )
{
    CHECK_THREAD( _pipeThread );

    EQASSERT( !_osPipe );

    OSPipe* osPipe = 0;

    switch( _windowSystem )
    {
#ifdef GLX
        case WINDOW_SYSTEM_GLX:
            EQINFO << "Using GLXPipe" << std::endl;
            osPipe = new GLXPipe( this );
            break;
#endif

#ifdef AGL
        case WINDOW_SYSTEM_AGL:
            EQINFO << "Using AGLPipe" << std::endl;
            osPipe = new AGLPipe( this );
            break;
#endif

#ifdef WGL
        case WINDOW_SYSTEM_WGL:
            EQINFO << "Using WGLPipe" << std::endl;
            osPipe = new WGLPipe( this );
            break;
#endif

        default:
            EQERROR << "Unknown window system: " << _windowSystem << std::endl;
            setErrorMessage( "Unknown window system" );
            return false;
    }

    EQASSERT( osPipe );
    if( !osPipe->configInit( ))
    {
        setErrorMessage( "OS Pipe initialization failed: " + 
                         osPipe->getErrorMessage( ));
        EQERROR << _error << std::endl;
        delete osPipe;
        return false;
    }

    setOSPipe( osPipe );
	
    // -------------------------------------------------------------------------
    EQASSERT(!_computeContext);

    // for now we only support CUDA
#ifdef EQ_USE_CUDA
    if( _cudaGLInterop )
    {
        EQINFO << "Initializing CUDAContext" << std::endl;
        ComputeContext* computeCtx = new CUDAContext( this );
		
        if( !computeCtx->configInit() )
        {
            setErrorMessage( "GPU Computing context initialization failed: " + 
                computeCtx->getErrorMessage( ));
            EQERROR << _error << std::endl;
            delete computeCtx;
            return false;
        }	
        setComputeContext( computeCtx );
    }
    else
        EQINFO << "No CUDA context initialized " << std::endl;
#endif

    return true;
}

bool Pipe::configExit()
{
    CHECK_THREAD( _pipeThread );

    if( _computeContext )
    {
        _computeContext->configExit();
        delete _computeContext;
        _computeContext = 0;
    }	
	
    if( _osPipe )
    {
        _osPipe->configExit( );

        delete _osPipe;
        _osPipe = 0;
        return true;
    }
    //else

    EQWARN << "Window system "<< _windowSystem << " not initialized" 
           << std::endl;
    return false;
}


void Pipe::frameStart( const uint32_t frameID, const uint32_t frameNumber ) 
{ 
    CHECK_THREAD( _pipeThread );

    const Node* node = getNode();
    switch( node->getIAttribute( Node::IATTR_THREAD_MODEL ))
    {
        case ASYNC:      // No sync, release immediately
            releaseFrameLocal( frameNumber ); 
            break;

        case DRAW_SYNC:  // Sync, release in frameDrawFinish
        case LOCAL_SYNC: // Sync, release in frameFinish
            node->waitFrameStarted( frameNumber );
            break;

        default:
            EQUNIMPLEMENTED;
    }

    startFrame( frameNumber );
}

void Pipe::frameDrawFinish( const uint32_t frameID, const uint32_t frameNumber )
{
    const Node* node = getNode();
    switch( node->getIAttribute( Node::IATTR_THREAD_MODEL ))
    {
        case ASYNC:      // released in frameStart
            break;

        case DRAW_SYNC:  // release
            releaseFrameLocal( frameNumber ); 
            break;

        case LOCAL_SYNC: // release in frameFinish
            break;

        default:
            EQUNIMPLEMENTED;
    }
}

void Pipe::frameFinish( const uint32_t frameID, const uint32_t frameNumber )
{
    const Node* node = getNode();
    switch( node->getIAttribute( Node::IATTR_THREAD_MODEL ))
    {
        case ASYNC:      // released in frameStart
            break;

        case DRAW_SYNC:  // released in frameDrawFinish
            break;

        case LOCAL_SYNC: // release
            releaseFrameLocal( frameNumber ); 
            break;

        default:
            EQUNIMPLEMENTED;
    }

    // Global release
    releaseFrame( frameNumber );
}

void Pipe::startFrame( const uint32_t frameNumber )
{ 
    CHECK_THREAD( _pipeThread );
    _currentFrame = frameNumber; 
    EQLOG( LOG_TASKS ) << "---- Started Frame ---- "<< frameNumber << std::endl;
}

void Pipe::releaseFrame( const uint32_t frameNumber )
{ 
    CHECK_THREAD( _pipeThread );
    _finishedFrame = frameNumber; 
    EQLOG( LOG_TASKS ) << "---- Finished Frame --- "<< frameNumber << std::endl;
}

void Pipe::releaseFrameLocal( const uint32_t frameNumber )
{ 
    CHECK_THREAD( _pipeThread );
    EQASSERT( _unlockedFrame + 1 == frameNumber );

    _unlockedFrame = frameNumber;
    EQLOG( LOG_TASKS ) << "---- Unlocked Frame --- " << _unlockedFrame.get()
                       << std::endl;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
net::CommandResult Pipe::_cmdCreateWindow(  net::Command& command  )
{
    const PipeCreateWindowPacket* packet = 
        command.getPacket<PipeCreateWindowPacket>();
    EQLOG( LOG_INIT ) << "Create window " << packet << std::endl;

    Window* window = Global::getNodeFactory()->createWindow( this );
    getConfig()->attachObject( window, packet->windowID, EQ_ID_INVALID );
    
    EQASSERT( !_windows.empty( ));
    return net::COMMAND_HANDLED;
}

net::CommandResult Pipe::_cmdDestroyWindow(  net::Command& command  )
{
    const PipeDestroyWindowPacket* packet =
        command.getPacket<PipeDestroyWindowPacket>();
    EQLOG( LOG_INIT ) << "Destroy window " << packet << std::endl;

    Window* window = _findWindow( packet->windowID );
    EQASSERT( window );

    // re-set shared windows accordingly
    Window* newSharedWindow = 0;
    for( WindowVector::const_iterator i = _windows.begin(); 
         i != _windows.end(); ++i )
    {
        Window* candidate = *i;
        
        if( candidate == window )
            continue; // ignore

        if( candidate->getSharedContextWindow() == window )
        {
            if( newSharedWindow )
                candidate->setSharedContextWindow( newSharedWindow );
            else
            {
                newSharedWindow = candidate;
                newSharedWindow->setSharedContextWindow( candidate );
            }
        }

        EQASSERT( candidate->getSharedContextWindow() != window );
    }

    Config* config = getConfig();
    config->detachObject( window );
    Global::getNodeFactory()->releaseWindow( window );

    return net::COMMAND_HANDLED;
}

net::CommandResult Pipe::_cmdConfigInit( net::Command& command )
{
    CHECK_THREAD( _pipeThread );
    const PipeConfigInitPacket* packet = 
        command.getPacket<PipeConfigInitPacket>();
    EQLOG( LOG_INIT ) << "Init pipe " << packet << std::endl;

    PipeConfigInitReplyPacket reply;
    _error.clear();

    _node->waitInitialized();

    if( _node->isRunning( ))
    {
        _name          = packet->name;
        _port          = packet->port;
        _device        = packet->device;
        _tasks         = packet->tasks;
        _pvp           = packet->pvp;
		_cudaGLInterop = packet->cudaGLInterop;
 
        _currentFrame  = packet->frameNumber;
        _finishedFrame = packet->frameNumber;
        _unlockedFrame = packet->frameNumber;
        
        _state = STATE_INITIALIZING;

        _windowSystem = selectWindowSystem();
        _setupCommandQueue();

        reply.result  = configInit( packet->initID );
    }
    else
        reply.result = false;

    EQLOG( LOG_INIT ) << "TASK pipe config init reply " << &reply << std::endl;

    net::NodePtr node = command.getNode();

    if( !_osPipe || !reply.result )
    {
        send( node, reply, _error );
        return net::COMMAND_HANDLED;
    }

    _state = STATE_RUNNING;

    reply.pvp = _pvp;
    send( node, reply );
    return net::COMMAND_HANDLED;
}

net::CommandResult Pipe::_cmdConfigExit( net::Command& command )
{
    CHECK_THREAD( _pipeThread );
    const PipeConfigExitPacket* packet = 
        command.getPacket<PipeConfigExitPacket>();
    EQLOG( LOG_INIT ) << "TASK pipe config exit " << packet << std::endl;

    PipeConfigExitReplyPacket reply;
    reply.result = configExit();
    _flushViews();

    _state = STATE_STOPPED;

    send( command.getNode(), reply );

    if( packet->exitThread )
    {
        EQASSERT( _thread );

        // cleanup
        command.release();
        _pipeThreadQueue->flush();
        _exitCommandQueue();

        EQINFO << "Leaving pipe thread" << std::endl;
        _thread->exit();
        EQUNREACHABLE;
    }

    return net::COMMAND_HANDLED;
}

net::CommandResult Pipe::_cmdFrameStartClock( net::Command& command )
{
    EQVERB << "start frame clock" << std::endl;
    _frameTimeMutex.set();
    _frameTimes.push_back( getConfig()->getTime( ));
    _frameTimeMutex.unset();
    return net::COMMAND_HANDLED;
}

net::CommandResult Pipe::_cmdFrameStart( net::Command& command )
{
    CHECK_THREAD( _pipeThread );
    const PipeFrameStartPacket* packet = 
        command.getPacket<PipeFrameStartPacket>();
    EQVERB << "handle pipe frame start " << packet << std::endl;
    EQLOG( LOG_TASKS ) << "---- TASK start frame ---- " << packet << std::endl;

    const int64_t lastFrameTime = _frameTime;

    _frameTimeMutex.set();
    EQASSERT( !_frameTimes.empty( ));

    _frameTime = _frameTimes.front();
    _frameTimes.pop_front();
    _frameTimeMutex.unset();

    if( lastFrameTime > 0 )
    {
        PipeStatistics waitEvent( Statistic::PIPE_IDLE, this );
        waitEvent.event.data.statistic.idleTime  = _waitTime;
        waitEvent.event.data.statistic.totalTime = _frameTime - lastFrameTime;
    }
    _waitTime = 0;

    const uint32_t frameNumber = packet->frameNumber;
    EQASSERTINFO( _currentFrame + 1 == frameNumber,
                  "current " << _currentFrame << " start " << frameNumber );

    frameStart( packet->frameID, frameNumber );
    return net::COMMAND_HANDLED;
}

net::CommandResult Pipe::_cmdFrameFinish( net::Command& command )
{
    CHECK_THREAD( _pipeThread );
    const PipeFrameFinishPacket* packet =
        command.getPacket<PipeFrameFinishPacket>();
    EQLOG( LOG_TASKS ) << "---- TASK finish frame --- " << packet << std::endl;

    const uint32_t frameNumber = packet->frameNumber;
    EQASSERTINFO( _currentFrame >= frameNumber, 
                  "current " << _currentFrame << " finish " << frameNumber );

    frameFinish( packet->frameID, frameNumber );
    
    EQASSERTINFO( _finishedFrame >= frameNumber, 
                  "Pipe::frameFinish() did not release frame " << frameNumber );

    if( _unlockedFrame < frameNumber )
    {
        EQWARN << "Finished frame was not locally unlocked, enforcing unlock" 
               << std::endl << "    unlocked " << _unlockedFrame.get()
               << " done " << frameNumber << std::endl;
        releaseFrameLocal( frameNumber );
    }

    if( _finishedFrame < frameNumber )
    {
        EQWARN << "Finished frame was not released, enforcing unlock"
               << std::endl;
        releaseFrame( frameNumber );
    }

    _releaseViews();
    return net::COMMAND_HANDLED;
}

net::CommandResult Pipe::_cmdFrameDrawFinish( net::Command& command )
{
    CHECK_THREAD( _pipeThread );
    PipeFrameDrawFinishPacket* packet = 
        command.getPacket< PipeFrameDrawFinishPacket >();
    EQLOG( LOG_TASKS ) << "TASK draw finish " << getName() <<  " " << packet
                       << std::endl;

    frameDrawFinish( packet->frameID, packet->frameNumber );
    return net::COMMAND_HANDLED;
}

}
