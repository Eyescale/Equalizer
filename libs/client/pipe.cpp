
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder<cedric.stalder@gmail.com>
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
#include "config.h"
#include "frame.h"
#include "frameData.h"
#include "global.h"
#include "log.h"
#include "node.h"
#include "nodeFactory.h"
#include "nodePackets.h"
#include "pipePackets.h"
#include "pipeStatistics.h"
#include "server.h"
#include "view.h"
#include "window.h"
#include "windowPackets.h"

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

#include <eq/fabric/elementVisitor.h>
#include <eq/fabric/task.h>
#include <co/command.h>
#include <sstream>

namespace eq
{
    typedef fabric::Pipe< Node, Pipe, Window, PipeVisitor > Super;

namespace
{
static const Window* _ntCurrentWindow = 0;
}

/** @cond IGNORE */
typedef co::CommandFunc<Pipe> PipeFunc;
/** @endcond */

Pipe::Pipe( Node* parent )
        : Super( parent )
        , _systemPipe( 0 )
        , _windowSystem( WINDOW_SYSTEM_NONE )
        , _state( STATE_STOPPED )
        , _currentFrame( 0 )
        , _frameTime( 0 )
        , _waitTime( 0 )
        , _thread( 0 )
        , _pipeThreadQueue( 0 )
        , _currentWindow( 0 )
        , _computeContext( 0 )
{
}

Pipe::~Pipe()
{
    EQASSERT( getWindows().empty( ));
    delete _thread;
    _thread = 0;
}

Config* Pipe::getConfig()
{
    Node* node = getNode();
    EQASSERT( node );
    return ( node ? node->getConfig() : 0);
}
const Config* Pipe::getConfig() const
{
    const Node* node = getNode();
    EQASSERT( node );
    return ( node ? node->getConfig() : 0);
}

ClientPtr Pipe::getClient()
{
    Node* node = getNode();
    EQASSERT( node );
    return ( node ? node->getClient() : 0);
}

ServerPtr Pipe::getServer()
{
    Node* node = getNode();
    EQASSERT( node );
    return ( node ? node->getServer() : 0);
}

void Pipe::attach( const co::base::UUID& id, const uint32_t instanceID )
{
    Super::attach( id, instanceID );
    
    co::CommandQueue* queue = getPipeThreadQueue();

    registerCommand( fabric::CMD_PIPE_CONFIG_INIT, 
                     PipeFunc( this, &Pipe::_cmdConfigInit ), queue );
    registerCommand( fabric::CMD_PIPE_CONFIG_EXIT, 
                     PipeFunc( this, &Pipe::_cmdConfigExit ), queue );
    registerCommand( fabric::CMD_PIPE_CREATE_WINDOW,
                     PipeFunc( this, &Pipe::_cmdCreateWindow ), queue );
    registerCommand( fabric::CMD_PIPE_DESTROY_WINDOW, 
                     PipeFunc( this, &Pipe::_cmdDestroyWindow ), queue );
    registerCommand( fabric::CMD_PIPE_FRAME_START,
                     PipeFunc( this, &Pipe::_cmdFrameStart ), queue );
    registerCommand( fabric::CMD_PIPE_FRAME_FINISH,
                     PipeFunc( this, &Pipe::_cmdFrameFinish ), queue );
    registerCommand( fabric::CMD_PIPE_FRAME_DRAW_FINISH, 
                     PipeFunc( this, &Pipe::_cmdFrameDrawFinish ), queue );
    registerCommand( fabric::CMD_PIPE_FRAME_START_CLOCK,
                     PipeFunc( this, &Pipe::_cmdFrameStartClock ), 0 );
    registerCommand( fabric::CMD_PIPE_EXIT_THREAD,
                     PipeFunc( this, &Pipe::_cmdExitThread ), queue );
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
    EQASSERT( _windowSystem != WINDOW_SYSTEM_NONE );
    EQINFO << "Set up pipe message pump for " << _windowSystem << std::endl;

    Config* config = getConfig();
    config->setupMessagePump( this );

    if( !_thread ) // Non-threaded pipes have no pipe thread message pump
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

MessagePump* Pipe::getMessagePump()
{
    EQ_TS_THREAD( _pipeThread );
    if( _pipeThreadQueue )
        return _pipeThreadQueue->getMessagePump();
    return 0;
}

void Pipe::_runThread()
{
    EQ_TS_THREAD( _pipeThread );
    EQINFO << "Entered pipe thread" << std::endl;

    Config* config = getConfig();
    EQASSERT( config );
    EQASSERT( _pipeThreadQueue );

    while( _pipeThreadQueue )
    {
        const int64_t startWait = config->getTime();
        co::Command* command = _pipeThreadQueue->pop();
        _waitTime += ( config->getTime() - startWait );

        EQASSERT( command );
        EQCHECK( command->invoke( ));
        command->release();
    }

    EQINFO << "Leaving pipe thread" << std::endl;
}

co::CommandQueue* Pipe::getPipeThreadQueue()
{
    if( _thread )
        return _pipeThreadQueue;

    return getNode()->getMainThreadQueue();
}

co::CommandQueue* Pipe::getMainThreadQueue()
{
    return getServer()->getMainThreadQueue();
}

co::CommandQueue* Pipe::getCommandThreadQueue()
{
    return getServer()->getCommandThreadQueue();
}

Frame* Pipe::getFrame( const co::ObjectVersion& frameVersion, const Eye eye,
                       const bool isOutput )
{
    EQ_TS_THREAD( _pipeThread );
    Frame* frame = _frames[ frameVersion.identifier ];

    if( !frame )
    {
        ClientPtr client = getClient();
        frame = new Frame();
        
        EQCHECK( client->mapObject( frame, frameVersion ));
        _frames[ frameVersion.identifier ] = frame;
    }
    
    frame->sync( frameVersion.version );

    const co::ObjectVersion& data = frame->getDataVersion( eye );
    EQLOG( LOG_ASSEMBLY ) << "Use " << data << std::endl;

    EQASSERT( data.identifier.isGenerated( ));
    FrameData* frameData = getNode()->getFrameData( data ); 
    EQASSERT( frameData );

    if( isOutput )
    {    
        if( !frameData->isAttached() )
        { 
            ClientPtr client = getClient();
            EQCHECK( client->mapObject( frameData, data ));
        }
        else if( frameData->getVersion() < data.version )
            frameData->sync( data.version );

        _outputFrameDatas[ data.identifier ] = frameData;
    }

    frame->setData( frameData );
    return frame;
}

void Pipe::flushFrames()
{
    EQ_TS_THREAD( _pipeThread );
    ClientPtr client = getClient();
    for( FrameHash::const_iterator i = _frames.begin(); i != _frames.end(); ++i)
    {
        Frame* frame = i->second;

        frame->setData( 0 ); // 'output' datas cleared below and from node
        frame->flush();
        client->unmapObject( frame );
        delete frame;
    }
    _frames.clear();

    for( FrameDataHash::const_iterator i = _outputFrameDatas.begin();
         i != _outputFrameDatas.end(); ++i)
    {
        FrameData* data = i->second;
        data->flush();
    }
    _outputFrameDatas.clear();
}

const View* Pipe::getView( const co::ObjectVersion& viewVersion ) const
{
    // Yie-ha: we want to have a const-interface to get a view on the render
    //         clients, but view mapping is by definition non-const.
    return const_cast< Pipe* >( this )->getView( viewVersion );
}

View* Pipe::getView( const co::ObjectVersion& viewVersion )
{
    EQ_TS_THREAD( _pipeThread );
    if( viewVersion.identifier ==co::base::UUID::ZERO )
        return 0;

    View* view = _views[ viewVersion.identifier ];
    if( !view )
    {
        NodeFactory* nodeFactory = Global::getNodeFactory();
        view = nodeFactory->createView( 0 );
        view->_pipe = this;
        EQASSERT( view );
        ClientPtr client = getClient();
        EQCHECK( client->mapObject( view, viewVersion ));

        _views[ viewVersion.identifier ] = view;
    }
    
    view->sync( viewVersion.version );
    return view;
}

void Pipe::_releaseViews()
{
    EQ_TS_THREAD( _pipeThread );
    for( bool changed = true; changed; )
    {
        changed = false;
        for( ViewHash::iterator i = _views.begin(); i != _views.end(); ++i )
        {
            View* view = i->second;
            view->commit();
            if( view->getVersion() + 20 > view->getHeadVersion( ))
                continue;

            // release unused view to avoid memory leaks due to deltas piling up
            ClientPtr client = getClient();
            client->unmapObject( view );
            
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
    EQ_TS_THREAD( _pipeThread );
    NodeFactory*  nodeFactory = Global::getNodeFactory();
    ClientPtr client = getClient();

    for( ViewHash::const_iterator i = _views.begin(); i != _views.end(); ++i)
    {
        View* view = i->second;

        client->unmapObject( view );
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

    PipeExitThreadPacket packet;
    send( getLocalNode(), packet );

    _thread->join();
    delete _thread;
    _thread = 0;
}

void Pipe::waitExited() const
{
    _state.waitGE( STATE_STOPPED );
}

bool Pipe::isRunning() const
{
    return (_state == STATE_RUNNING);
}

bool Pipe::isStopped() const
{
    return (_state == STATE_STOPPED);
}

void Pipe::notifyMapped()
{
    EQASSERT( _state == STATE_STOPPED );
    _state = STATE_MAPPED;
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
bool Pipe::configInit( const uint128_t& initID )
{
    EQ_TS_THREAD( _pipeThread );

    EQASSERT( !_systemPipe );

    if ( !configInitSystemPipe( initID ))
        return false;   

    // -------------------------------------------------------------------------
    EQASSERT(!_computeContext);

    // for now we only support CUDA
#ifdef EQ_USE_CUDA
    if( getIAttribute( IATTR_HINT_CUDA_GL_INTEROP ) == eq::ON )
    {
        EQINFO << "Initializing CUDAContext" << std::endl;
        ComputeContext* computeCtx = new CUDAContext( this );

        if( !computeCtx->configInit() )
        {
            EQASSERT( getError() != ERROR_NONE );
            EQWARN << "GPU Computing context initialization failed: "
                   << getError() << std::endl;
            delete computeCtx;
            return false;
        }
        setComputeContext( computeCtx );
    }
#endif

    return true;
}

bool Pipe::configInitSystemPipe( const uint128_t& )
{
    SystemPipe* systemPipe = 0;

    switch( _windowSystem )
    {
#ifdef GLX
        case WINDOW_SYSTEM_GLX:
            EQINFO << "Using GLXPipe" << std::endl;
            systemPipe = new GLXPipe( this );
            break;
#endif

#ifdef AGL
        case WINDOW_SYSTEM_AGL:
            EQINFO << "Using AGLPipe" << std::endl;
            systemPipe = new AGLPipe( this );
            break;
#endif

#ifdef WGL
        case WINDOW_SYSTEM_WGL:
            EQINFO << "Using WGLPipe" << std::endl;
            systemPipe = new WGLPipe( this );
            break;
#endif

        default:
            setError( ERROR_WINDOWSYSTEM_UNKNOWN );
            return false;
    }

    EQASSERT( systemPipe );
    if( !systemPipe->configInit( ))
    {
        EQASSERT( getError() != ERROR_NONE );
        EQERROR << "System pipe context initialization failed: "
                << getError() << std::endl;
        delete systemPipe;
        return false;
    }

    setSystemPipe( systemPipe );
    return true;
}

bool Pipe::configExit()
{
    EQ_TS_THREAD( _pipeThread );

    if( _computeContext )
    {
        _computeContext->configExit();
        delete _computeContext;
        _computeContext = 0;
    }

    if( _systemPipe )
    {
        _systemPipe->configExit( );
        delete _systemPipe;
        _systemPipe = 0;
    }
    return true;
}


void Pipe::frameStart( const uint128_t&, const uint32_t frameNumber ) 
{
    EQ_TS_THREAD( _pipeThread );

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

void Pipe::frameDrawFinish( const uint128_t&, const uint32_t frameNumber )
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

void Pipe::frameFinish( const uint128_t&, const uint32_t frameNumber )
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
    EQ_TS_THREAD( _pipeThread );
    _currentFrame = frameNumber; 
    EQLOG( LOG_TASKS ) << "---- Started Frame ---- "<< frameNumber << std::endl;
}

void Pipe::releaseFrame( const uint32_t frameNumber )
{ 
    EQ_TS_THREAD( _pipeThread );
    _finishedFrame = frameNumber; 
    EQLOG( LOG_TASKS ) << "---- Finished Frame --- "<< frameNumber << std::endl;
}

void Pipe::releaseFrameLocal( const uint32_t frameNumber )
{ 
    EQ_TS_THREAD( _pipeThread );
    EQASSERTINFO( _unlockedFrame + 1 == frameNumber,
                  _unlockedFrame << ", " << frameNumber );

    _unlockedFrame = frameNumber;
    EQLOG( LOG_TASKS ) << "---- Unlocked Frame --- " << _unlockedFrame.get()
                       << std::endl;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
bool Pipe::_cmdCreateWindow(  co::Command& command  )
{
    const PipeCreateWindowPacket* packet = 
        command.getPacket<PipeCreateWindowPacket>();
    EQLOG( LOG_INIT ) << "Create window " << packet << std::endl;

    Window* window = Global::getNodeFactory()->createWindow( this );
    window->init(); // not in ctor, virtual method

    Config* config = getConfig();
    EQCHECK( config->mapObject( window, packet->windowID ));
    
    return true;
}

bool Pipe::_cmdDestroyWindow(  co::Command& command  )
{
    const PipeDestroyWindowPacket* packet =
        command.getPacket<PipeDestroyWindowPacket>();
    EQLOG( LOG_INIT ) << "Destroy window " << packet << std::endl;

    Window* window = _findWindow( packet->windowID );
    EQASSERT( window );

    // re-set shared windows accordingly
    Window* newSharedWindow = 0;
    const Windows& windows = getWindows(); 
    for( Windows::const_iterator i = windows.begin(); i != windows.end(); ++i )
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

    WindowConfigExitReplyPacket reply( packet->windowID, window->isStopped( ));

    Config* config = getConfig();
    config->unmapObject( window );
    Global::getNodeFactory()->releaseWindow( window );

    getServer()->send( reply ); // do not use Object::send()
    return true;
}

bool Pipe::_cmdConfigInit( co::Command& command )
{
    EQ_TS_THREAD( _pipeThread );
    const PipeConfigInitPacket* packet = 
        command.getPacket<PipeConfigInitPacket>();
    EQLOG( LOG_INIT ) << "Init pipe " << packet << std::endl;

    _state.waitEQ( STATE_MAPPED );

    PipeConfigInitReplyPacket reply;
    setError( ERROR_NONE );

    Node* node = getNode();
    EQASSERT( node );
    node->waitInitialized();

    if( node->isRunning( ))
    {
        _currentFrame  = packet->frameNumber;
        _finishedFrame = packet->frameNumber;
        _unlockedFrame = packet->frameNumber;
        
        _state = STATE_INITIALIZING;

        _windowSystem = selectWindowSystem();
        reply.result = configInit( packet->initID );

        _setupCommandQueue();
        if( reply.result )
            _state = STATE_RUNNING;
    }
    else
    {
        setError( ERROR_PIPE_NODE_NOTRUNNING );
        reply.result = false;
    }

    EQLOG( LOG_INIT ) << "TASK pipe config init reply " << &reply << std::endl;

    co::NodePtr nodePtr = command.getNode();

    commit();
    send( nodePtr, reply );
    return true;
}

bool Pipe::_cmdConfigExit( co::Command& command )
{
    EQ_TS_THREAD( _pipeThread );
    const PipeConfigExitPacket* packet = 
        command.getPacket<PipeConfigExitPacket>();
    EQLOG( LOG_INIT ) << "TASK pipe config exit " << packet << std::endl;

    // send before node gets a chance to send its destroy packet
    NodeDestroyPipePacket destroyPacket( getID( ));
    getNode()->send( getLocalNode(), destroyPacket );

    _state = configExit() ? STATE_STOPPED : STATE_FAILED;
    _flushViews();
    return true;
}

bool Pipe::_cmdExitThread( co::Command& command )
{
    EQASSERT( _thread );

    // cleanup
    _pipeThreadQueue->flush();
    _exitCommandQueue();
    delete _pipeThreadQueue;
    _pipeThreadQueue = 0;

    return true;
}

bool Pipe::_cmdFrameStartClock( co::Command& )
{
    EQVERB << "start frame clock" << std::endl;
    _frameTimeMutex.set();
    _frameTimes.push_back( getConfig()->getTime( ));
    _frameTimeMutex.unset();
    return true;
}

bool Pipe::_cmdFrameStart( co::Command& command )
{
    EQ_TS_THREAD( _pipeThread );
    const PipeFrameStartPacket* packet = 
        command.getPacket<PipeFrameStartPacket>();
    EQVERB << "handle pipe frame start " << packet << std::endl;
    EQLOG( LOG_TASKS ) << "---- TASK start frame ---- " << packet << std::endl;
    sync( packet->version );
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
    return true;
}

bool Pipe::_cmdFrameFinish( co::Command& command )
{
    EQ_TS_THREAD( _pipeThread );
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
    commit();
    return true;
}

bool Pipe::_cmdFrameDrawFinish( co::Command& command )
{
    EQ_TS_THREAD( _pipeThread );
    PipeFrameDrawFinishPacket* packet = 
        command.getPacket< PipeFrameDrawFinishPacket >();
    EQLOG( LOG_TASKS ) << "TASK draw finish " << getName() <<  " " << packet
                       << std::endl;

    frameDrawFinish( packet->frameID, packet->frameNumber );
    return true;
}

}

#include "../fabric/pipe.ipp"
template class eq::fabric::Pipe< eq::Node, eq::Pipe, eq::Window,
                                 eq::PipeVisitor >;
/** @cond IGNORE */
template EQFABRIC_API std::ostream& eq::fabric::operator << ( std::ostream&,
                                                 const eq::Super& );
/** @endcond */
