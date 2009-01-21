
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "pipe.h"

#include "commands.h"
#include "global.h"
#include "log.h"
#include "nodeFactory.h"
#include "packets.h"
#include "task.h"
#include "X11Connection.h"
#include "window.h"

#ifdef GLX
#  include "glXPipe.h"
#endif
#ifdef WGL
#  include "wglPipe.h"
#endif
#ifdef AGL
#  include "aglPipe.h"
#endif

#include <eq/net/command.h>
#include <sstream>

using namespace eq::base;
using namespace std;
using eq::net::CommandFunc;

namespace eq
{
namespace
{
static const Window* _ntCurrentWindow = 0;
}

typedef net::CommandFunc<Pipe> PipeFunc;

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
        , _thread( 0 )
        , _pipeThreadQueue( 0 )
        , _currentWindow( 0 )
{
    parent->_addPipe( this );
    EQINFO << " New eq::Pipe @" << (void*)this << endl;
}

Pipe::~Pipe()
{
    _node->_removePipe( this );
    delete _thread;
    _thread = 0;
}

PipeVisitor::Result Pipe::accept( PipeVisitor* visitor )
{ 
    WindowVisitor::Result result = visitor->visitPre( this );
    if( result != WindowVisitor::TRAVERSE_CONTINUE )
        return result;

    for( WindowVector::const_iterator i = _windows.begin(); 
         i != _windows.end(); ++i )
    {
        Window* window = *i;
        switch( window->accept( visitor ))
        {
            case WindowVisitor::TRAVERSE_TERMINATE:
                return WindowVisitor::TRAVERSE_TERMINATE;

            case WindowVisitor::TRAVERSE_PRUNE:
                result = WindowVisitor::TRAVERSE_PRUNE;
                break;
                
            case WindowVisitor::TRAVERSE_CONTINUE:
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
    registerCommand( CMD_PIPE_FRAME_NO_DRAW, 
                     PipeFunc( this, &Pipe::_cmdFrameNoDraw ), 0 );
    registerCommand( CMD_PIPE_STOP_THREAD, 
                     PipeFunc( this, &Pipe::_cmdStopThread ), queue );
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
    EQASSERTINFO( 0, "No supported window system found" );
    return WINDOW_SYSTEM_NONE;
}

void Pipe::_setupCommandQueue()
{
    EQASSERT( _windowSystem != WINDOW_SYSTEM_NONE );

    // Switch the node thread message pumps for non-threaded and AGL pipes
    if( !_thread || _windowSystem == WINDOW_SYSTEM_AGL )
    {
        if( !useMessagePump( ))
            return;

        Config* config = getConfig();
        config->setWindowSystem( _windowSystem );
        return;
    }
    
    EQASSERT( _pipeThreadQueue );
    
    if( useMessagePump( ))
    {
        _pipeThreadQueue->setWindowSystem( _windowSystem );
        EQINFO << "Pipe message pump set up for " << _windowSystem << endl;
    }
}


void* Pipe::_runThread()
{
    EQINFO << "Entered pipe thread" << endl;
    CHECK_THREAD( _pipeThread );

    Config* config = getConfig();
    EQASSERT( config );
    EQASSERT( _pipeThreadQueue );

    while( _thread->isRunning( ))
    {
        net::Command* command = _pipeThreadQueue->pop();
        switch( config->invokeCommand( *command ))
        {
            case net::COMMAND_HANDLED:
            case net::COMMAND_DISCARD:
                break;

            case net::COMMAND_ERROR:
                EQERROR << "Error handling command packet" << endl;
                abort();
            default:
                EQERROR << "Unknown command result" << endl;
                abort();
        }
        _pipeThreadQueue->release( command );
    }

    EQUNREACHABLE; // since we are exited from _cmdConfigExit
    return EXIT_SUCCESS;
}

net::CommandQueue* Pipe::getPipeThreadQueue()
{
    if( !_thread )
        return _node->getNodeThreadQueue();

    return _pipeThreadQueue;
}

Frame* Pipe::getFrame( const net::ObjectVersion& frameVersion, const Eye eye )
{
    Frame* frame = _frames[ frameVersion.id ];

    if( !frame )
    {
        net::Session* session = getSession();
        frame = new Frame();
        
        EQCHECK( session->mapObject( frame, frameVersion.id ));
        _frames[ frameVersion.id ] = frame;
    }
    
    frame->sync( frameVersion.version );

    const net::ObjectVersion& data = frame->getDataVersion( eye );
    EQASSERT( data.id != EQ_ID_INVALID );
    FrameData* frameData = getNode()->getFrameData( data ); 
    EQASSERT( frameData );

    frame->setData( frameData );
    return frame;
}

void Pipe::_flushFrames()
{
    net::Session* session = getSession();

    for( FrameHash::const_iterator i = _frames.begin(); i != _frames.end(); ++i)
    {
        Frame* frame = i->second;
        session->unmapObject( frame );
        delete frame;
    }
    _frames.clear();
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

WGLEWContext* Pipe::wglewGetContext()
{ 
    return _osPipe->wglewGetContext();
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
            EQINFO << "Pipe: using GLXWindow" << std::endl;
            osPipe = new GLXPipe( this );
            break;
#endif

#ifdef AGL
        case WINDOW_SYSTEM_AGL:
            EQINFO << "Pipe: using AGLWindow" << std::endl;
            osPipe = new AGLPipe( this );
            break;
#endif

#ifdef WGL
        case WINDOW_SYSTEM_WGL:
            EQINFO << "Pipe: using WGLWindow" << std::endl;
            osPipe = new WGLPipe( this );
            break;
#endif

        default:
            EQERROR << "Unknown windowing system: " << _windowSystem << endl;
            setErrorMessage( "Unknown windowing system" );
            return false;
    }

    EQASSERT( osPipe );
    if( !osPipe->configInit( ))
    {
        setErrorMessage( "OS Pipe initialization failed: " + 
                         osPipe->getErrorMessage( ));
        EQERROR << _error << endl;
        delete osPipe;
        return false;
    }

    setOSPipe( osPipe );
    return true;
}

bool Pipe::configExit()
{
    CHECK_THREAD( _pipeThread );

    if( _osPipe )
    {
        _osPipe->configExit( );

        delete _osPipe;
        _osPipe = 0;
        return true;
    }
    //else

    EQWARN << "Window system "<< _windowSystem <<" was not initialized" << endl;
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

void Pipe::frameNoDraw( const uint32_t frameID, const uint32_t frameNumber )
{
    const Node* node = getNode();
    switch( node->getIAttribute( Node::IATTR_THREAD_MODEL ))
    {
        case ASYNC:      // release
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
    EQLOG( LOG_TASKS ) << "---- Started Frame ---- " << frameNumber << endl;
}

void Pipe::releaseFrame( const uint32_t frameNumber )
{ 
    CHECK_THREAD( _pipeThread );
    _finishedFrame = frameNumber; 
    EQLOG( LOG_TASKS ) << "---- Finished Frame --- " << frameNumber << endl;
}

void Pipe::releaseFrameLocal( const uint32_t frameNumber )
{ 
    CHECK_THREAD( _pipeThread );

    if( _unlockedFrame >= frameNumber ) 
        return; // DUP (nodraw + drawfinish)

    if( _unlockedFrame + 1 == frameNumber )
    {
        ++_unlockedFrame;

        // Catch up with saved future released frames
        stde::usort( _unlockedFrames );
        while( !_unlockedFrames.empty() &&
               _unlockedFrames[0] == _unlockedFrame + 1 )
        {
            ++_unlockedFrame;
            _unlockedFrames.pop_front();
        }
    }
    else
        _unlockedFrames.push_back( frameNumber ); // not yet ready - save
    
    EQLOG( LOG_TASKS ) << "---- Unlocked Frame --- " << _unlockedFrame.get()
                       << endl;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
net::CommandResult Pipe::_cmdCreateWindow(  net::Command& command  )
{
    const PipeCreateWindowPacket* packet = 
        command.getPacket<PipeCreateWindowPacket>();
    EQINFO << "Handle create window " << packet << endl;

    Window* window = Global::getNodeFactory()->createWindow( this );
    getConfig()->attachObject( window, packet->windowID );
    
    EQASSERT( !_windows.empty( ));
    if( window != _windows[0] )
        window->setSharedContextWindow( _windows[0] );

    return net::COMMAND_HANDLED;
}

net::CommandResult Pipe::_cmdDestroyWindow(  net::Command& command  )
{
    const PipeDestroyWindowPacket* packet =
        command.getPacket<PipeDestroyWindowPacket>();
    EQINFO << "Handle destroy window " << packet << endl;

    Window* window = _findWindow( packet->windowID );
    EQASSERT( window );

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
    EQLOG( LOG_TASKS ) << "TASK pipe config init " << packet << endl;

    _name   = packet->name;
    _port   = packet->port;
    _device = packet->device;
    _tasks  = packet->tasks;
    _pvp    = packet->pvp;

    _currentFrame  = 0;
    _finishedFrame = 0;
    _unlockedFrame = 0;

    PipeConfigInitReplyPacket reply;
    _node->waitInitialized();
    _state = STATE_INITIALIZING;

    _windowSystem = selectWindowSystem();
    _setupCommandQueue();

    _error.clear();
    reply.result  = configInit( packet->initID );
    EQLOG( LOG_TASKS ) << "TASK pipe config init reply " << &reply << endl;

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
    EQLOG( LOG_TASKS ) << "TASK configExit " << getName() <<  " " << packet 
                       << endl;

    PipeConfigExitReplyPacket reply;
    reply.result = configExit();

    _state = STATE_STOPPED;
    _flushFrames();

    send( command.getNode(), reply );
    return net::COMMAND_HANDLED;
}

net::CommandResult Pipe::_cmdFrameStartClock( net::Command& command )
{
	EQVERB << "start frame clock" << endl;
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
    EQVERB << "handle pipe frame start " << packet << endl;
    EQLOG( LOG_TASKS ) << "---- TASK start frame ---- " << packet << endl;

    const uint32_t frameNumber = packet->frameNumber;
    EQASSERTINFO( _currentFrame + 1 == frameNumber,
                  "current " << _currentFrame << " start " << frameNumber );

    _frameTimeMutex.set();
    EQASSERT( !_frameTimes.empty( ));

    _frameTime = _frameTimes.front();
    _frameTimes.pop_front();
    _frameTimeMutex.unset();

    frameStart( packet->frameID, frameNumber );
    return net::COMMAND_HANDLED;
}

net::CommandResult Pipe::_cmdFrameFinish( net::Command& command )
{
    CHECK_THREAD( _pipeThread );
    const PipeFrameFinishPacket* packet =
        command.getPacket<PipeFrameFinishPacket>();
    EQVERB << "handle pipe frame finish " << packet << endl;
    EQLOG( LOG_TASKS ) << "---- TASK finish frame --- " << packet << endl;

    const uint32_t frameNumber = packet->frameNumber;
    EQASSERTINFO( _currentFrame >= frameNumber, 
                  "current " << _currentFrame << " finish " << frameNumber );

    frameFinish( packet->frameID, frameNumber );
    
    EQASSERTINFO( _finishedFrame >= frameNumber, 
                  "Pipe::frameFinish() did not release frame " << frameNumber );

    if( _unlockedFrame < frameNumber )
    {
        EQWARN << "Finished frame was not locally unlocked, enforcing unlock" 
               << endl << "    unlocked " << _unlockedFrame.get() << " done "
               << frameNumber << endl;
        releaseFrameLocal( frameNumber );
    }

    if( _finishedFrame < frameNumber )
    {
        EQWARN << "Finished frame was not released, enforcing unlock" << endl;
        releaseFrame( frameNumber );
    }

    return net::COMMAND_HANDLED;
}

net::CommandResult Pipe::_cmdFrameDrawFinish( net::Command& command )
{
    CHECK_THREAD( _pipeThread );
    PipeFrameDrawFinishPacket* packet = 
        command.getPacket< PipeFrameDrawFinishPacket >();
    EQLOG( LOG_TASKS ) << "TASK draw finish " << getName() <<  " " << packet
                       << endl;

    frameDrawFinish( packet->frameID, packet->frameNumber );
    return net::COMMAND_HANDLED;
}

net::CommandResult Pipe::_cmdFrameNoDraw( net::Command& command )
{
    if( !command.isDispatched( ))
    {
        net::CommandQueue* queue = getPipeThreadQueue();
        queue->pushFront( command );

        EQLOG( LOG_TASKS ) << "pushed TASK no draw " << getName() << endl;
        return net::COMMAND_HANDLED;
    }

    CHECK_THREAD( _pipeThread );
    PipeFrameNoDrawPacket* packet = command.getPacket< PipeFrameNoDrawPacket>();
    EQLOG( LOG_TASKS ) << "TASK no draw " << getName() <<  " " << packet
                       << endl;

    frameNoDraw( packet->frameID, packet->frameNumber );
    return net::COMMAND_HANDLED;
}

net::CommandResult Pipe::_cmdStopThread( net::Command& command )
{
    EQASSERT( _thread );

    // cleanup
    _pipeThreadQueue->release( &command );
    _pipeThreadQueue->flush();

    EQINFO << "Leaving pipe thread" << endl;
    _thread->exit( EXIT_SUCCESS );
    EQUNREACHABLE;
    return net::COMMAND_HANDLED;
}
}
