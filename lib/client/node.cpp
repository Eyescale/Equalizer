
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <pthread.h>

#include "node.h"

#include "client.h"
#include "commands.h"
#include "frameData.h"
#include "global.h"
#include "log.h"
#include "nodeFactory.h"
#include "packets.h"
#include "pipe.h"
#include "server.h"

#include <eq/base/scopedMutex.h>
#include <eq/net/command.h>
#include <eq/net/connection.h>

using namespace eqBase;
using namespace std;
using eqNet::CommandFunc;

namespace eq
{
Node::Node( Config* parent )
        : _config( parent )
        , _unlockedFrame( 0 )
        , _finishedFrame( 0 )
{
    eqNet::CommandQueue* queue        = parent->getNodeThreadQueue();

    registerCommand( CMD_NODE_CREATE_PIPE, 
                     CommandFunc<Node>( this, &Node::_cmdCreatePipe ), queue );
    registerCommand( CMD_NODE_DESTROY_PIPE,
                     CommandFunc<Node>( this, &Node::_cmdDestroyPipe ), queue );
    registerCommand( CMD_NODE_CONFIG_INIT, 
                     CommandFunc<Node>( this, &Node::_cmdConfigInit ), queue );
    registerCommand( CMD_NODE_CONFIG_EXIT,
                     CommandFunc<Node>( this, &Node::_cmdConfigExit ), queue );
    registerCommand( CMD_NODE_FRAME_START,
                     CommandFunc<Node>( this, &Node::_cmdFrameStart ), queue );
    registerCommand( CMD_NODE_FRAME_FINISH,
                     CommandFunc<Node>( this, &Node::_cmdFrameFinish ), queue );
    registerCommand( CMD_NODE_FRAME_FINISH_EARLY,
                     CommandFunc<Node>( this, &Node::_cmdFrameFinishEarly ), 
                     queue );
    registerCommand( CMD_NODE_FRAME_DRAW_FINISH, 
                     CommandFunc<Node>( this, &Node::_cmdFrameDrawFinish ),
                     queue );

    parent->_addNode( this );
    EQINFO << " New eq::Node @" << (void*)this << endl;
}

Node::~Node()
{
    _config->_removeNode( this );
    EQINFO << " Delete eq::Node @" << (void*)this << endl;
    if( !_dataQueue.empty( ))
        EQWARN << "Node data queue not empty in destructor" << endl;
}

void Node::_addPipe( Pipe* pipe )
{
    EQASSERT( pipe->getNode() == this );
    _pipes.push_back( pipe );
}

void Node::_removePipe( Pipe* pipe )
{
    PipeVector::iterator iter = find( _pipes.begin(), _pipes.end(), pipe );
    EQASSERT( iter != _pipes.end( ))
    
    _pipes.erase( iter );
}

Pipe* Node::_findPipe( const uint32_t id )
{
    for( PipeVector::const_iterator i = _pipes.begin(); i != _pipes.end(); 
         ++i )
    {
        Pipe* pipe = *i;
        if( pipe->getID() == id )
            return pipe;
    }
    return 0;
}

eqNet::Barrier* Node::getBarrier( const uint32_t id, const uint32_t version )
{
    _barriersMutex.set();
    eqNet::Barrier* barrier = _barriers[ id ];

    if( !barrier )
    {
        eqNet::Session* session = getSession();

        barrier = new eqNet::Barrier;
        barrier->makeThreadSafe();
        const bool mapped = session->mapObject( barrier, id );
        EQASSERT( mapped );

        _barriers[ id ] = barrier;
    }
    _barriersMutex.unset();

    barrier->sync( version );
    return barrier;
}

FrameData* Node::getFrameData( const eqNet::ObjectVersion& dataVersion )
{
    _frameDatasMutex.set();
    FrameData* frameData = _frameDatas[ dataVersion.id ];

    if( !frameData )
    {
        eqNet::Session* session = getSession();
        
        frameData = new FrameData;
        frameData->makeThreadSafe();
        const bool mapped = session->mapObject( frameData, dataVersion.id );
        EQASSERT( mapped );

        _frameDatas[ dataVersion.id ] = frameData;
    }
    _frameDatasMutex.unset();

    frameData->sync( dataVersion.version );
    return frameData;
}

void Node::_finishFrame( const uint32_t frameNumber ) const
{
    for( PipeVector::const_iterator i = _pipes.begin(); i != _pipes.end(); 
         ++i )
    {
        const Pipe* pipe = *i;
        pipe->waitFrameLocal( frameNumber );
        pipe->waitFrameFinished( frameNumber );
    }
}

void Node::_frameFinish( const uint32_t frameID, const uint32_t frameNumber )
{
    frameFinish( frameID, frameNumber );
    EQLOG( LOG_TASKS ) << "---- Finished Frame --- " << frameNumber << endl;

    if( _unlockedFrame < frameNumber )
    {
        EQWARN << "Finished frame was not locally unlocked, enforcing unlock" 
               << endl;
        releaseFrameLocal( frameNumber );
    }

    if( _finishedFrame < frameNumber )
    {
        EQWARN << "Finished frame was not released, enforcing unlock" << endl;
        releaseFrame( frameNumber );
    }
}

void Node::releaseFrame( const uint32_t frameNumber )
{
    EQASSERTINFO( _currentFrame >= frameNumber, 
                  "current " << _currentFrame << " release " << frameNumber );
    EQASSERTINFO( _finishedFrame < frameNumber, 
                  "finished " << _currentFrame << " release " << frameNumber );
    _finishedFrame = frameNumber;

    NodeFrameFinishReplyPacket packet;
    packet.frameNumber = frameNumber;

    RefPtr< eqNet::Node > node = 
        RefPtr_static_cast< Server, eqNet::Node >( _config->getServer( ));
    send( node, packet );
}

void Node::releaseFrameLocal( const uint32_t frameNumber )
{
    EQASSERT( _unlockedFrame <= frameNumber );
    _unlockedFrame = frameNumber;
    
    Config* config = getConfig();
    EQASSERT( config->getNodes().size() == 1 );
    EQASSERT( config->getNodes()[0] == this );
    config->releaseFrameLocal( frameNumber );
}

void Node::frameDrawFinish( const uint32_t frameID, const uint32_t frameNumber )
{ 
    for( PipeVector::const_iterator i = _pipes.begin(); i != _pipes.end(); ++i )
    {
        const Pipe* pipe = *i;
        pipe->waitFrameLocal( frameNumber );
    }

    releaseFrameLocal( frameNumber );
}

void Node::_flushObjects()
{
    eqNet::Session* session = getSession();

    _barriersMutex.set();
    for( eqNet::IDHash< eqNet::Barrier* >::const_iterator i =_barriers.begin(); 
         i != _barriers.end(); ++ i )
    {
        eqNet::Barrier* barrier = i->second;
        session->unmapObject( barrier );
        delete barrier;
    }
    _barriers.clear();
    _barriersMutex.unset();

    _frameDatasMutex.set();
    for( eqNet::IDHash< FrameData* >::const_iterator i = _frameDatas.begin(); 
         i != _frameDatas.end(); ++ i )
    {
        FrameData* frameData = i->second;
        session->unmapObject( frameData );
        delete frameData;
    }
    _frameDatas.clear();
    _frameDatasMutex.unset();
}

#ifdef EQ_TRANSMISSION_API
const void* Node::receiveData( uint64_t* size )
{
    eqNet::Command* command = _dataQueue.pop();
    const ConfigDataPacket* packet = command->getPacket<ConfigDataPacket>();
    EQASSERT( packet->datatype == eqNet::DATATYPE_EQNET_SESSION );
    EQASSERT( packet->command == CMD_CONFIG_DATA );

    if( size )
        *size = packet->dataSize;
    return packet->data;
}

const void* Node::tryReceiveData( uint64_t* size )
{
    eqNet::Command* command = _dataQueue.tryPop();
    if( !command )
        return 0;

    const ConfigDataPacket* packet = command->getPacket<ConfigDataPacket>();
    EQASSERT( packet->datatype == eqNet::DATATYPE_EQNET_SESSION );
    EQASSERT( packet->command == CMD_CONFIG_DATA );

    if( size )
        *size = packet->dataSize;
    return packet->data;
}

bool Node::hasData() const
{
    return !_dataQueue.empty();
}
#endif // EQ_TRANSMISSION_API

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
eqNet::CommandResult Node::_cmdCreatePipe( eqNet::Command& command )
{
    const NodeCreatePipePacket* packet = 
        command.getPacket<NodeCreatePipePacket>();
    EQINFO << "Handle create pipe " << packet << endl;

    CHECK_THREAD( _nodeThread );
    EQASSERT( packet->pipeID != EQ_ID_INVALID );

    Pipe* pipe = Global::getNodeFactory()->createPipe( this );

    if( packet->threaded )
        pipe->startThread();

    _config->attachObject( pipe, packet->pipeID );

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_cmdDestroyPipe( eqNet::Command& command )
{
    const NodeDestroyPipePacket* packet = 
        command.getPacket<NodeDestroyPipePacket>();
    EQINFO << "Handle destroy pipe " << packet << endl;

    CHECK_THREAD( _nodeThread );
    Pipe* pipe = _findPipe( packet->pipeID );
    pipe->joinThread();
    _config->detachObject( pipe );

    delete pipe;
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_cmdConfigInit( eqNet::Command& command )
{
    const NodeConfigInitPacket* packet = 
        command.getPacket<NodeConfigInitPacket>();
    EQLOG( LOG_TASKS ) << "TASK node config init " << packet << endl;

    CHECK_THREAD( _nodeThread );
    _name = packet->name;
    _currentFrame  = 0;
    _unlockedFrame = 0;
    _finishedFrame = 0;

    _error.clear();
    NodeConfigInitReplyPacket reply;
    reply.result = configInit( packet->initID );
    _initialized = true; // even if init failed we need to unlock the pipes

    EQLOG( LOG_TASKS ) << "TASK node config init reply " << &reply << endl;
    send( command.getNode(), reply, _error );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_cmdConfigExit( eqNet::Command& command )
{
    const NodeConfigExitPacket* packet = 
        command.getPacket<NodeConfigExitPacket>();
    EQINFO << "handle node config exit " << packet << endl;

    CHECK_THREAD( _nodeThread );
    for( PipeVector::const_iterator i = _pipes.begin(); i != _pipes.end(); 
         ++i )
    {
        Pipe* pipe = *i;
        pipe->waitExited();
    }
    
    NodeConfigExitReplyPacket reply;
    reply.result = configExit();

    _initialized = false;
    _flushObjects();

    send( command.getNode(), reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_cmdFrameStart( eqNet::Command& command )
{
    CHECK_THREAD( _nodeThread );
    const NodeFrameStartPacket* packet = 
        command.getPacket<NodeFrameStartPacket>();
    EQVERB << "handle node frame start " << packet << endl;

    const uint32_t frameNumber = packet->frameNumber;
    if( _currentFrame >= frameNumber ) // already done be finish early (below)
        return eqNet::COMMAND_HANDLED;

    EQLOG( LOG_TASKS ) << "----- Begin Frame ----- " << frameNumber << endl;

    frameStart( packet->frameID, frameNumber );
    EQASSERTINFO( _currentFrame >= frameNumber, 
                  "Node::frameStart() did not start frame " << frameNumber );

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_cmdFrameFinish( eqNet::Command& command )
{
    CHECK_THREAD( _nodeThread );
    const NodeFrameFinishPacket* packet = 
        command.getPacket<NodeFrameFinishPacket>();
    EQVERB << "handle node frame finish " << packet << endl;

    if( _finishedFrame >= packet->frameNumber ) // already done
        return eqNet::COMMAND_HANDLED;

    _finishFrame( packet->frameNumber );
    _frameFinish( packet->frameID, packet->frameNumber );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_cmdFrameFinishEarly( eqNet::Command& command )
{
    CHECK_THREAD( _nodeThread );
    EQASSERT( _currentFrame >= _finishedFrame );

    const NodeFrameFinishEarlyPacket* packet = 
        command.getPacket<NodeFrameFinishEarlyPacket>();
    EQVERB << "handle node frame finish early " << packet << endl;

    const uint32_t frameNumber = packet->frameNumber;
    if( _finishedFrame >= frameNumber ) // already done
        return eqNet::COMMAND_HANDLED;

    if( _currentFrame < frameNumber )
    {
        // It is possible that the finishEarly arrives before the corresponding
        // frameStart, e.g., in eqPly where pipe threads are not synchronized
        // and send their finish to the server, which sends the node finishEarly
        // _before_ the node thread managed to process the startFrame.
        EQASSERT( _currentFrame.get() + 1 == frameNumber );
        
        // Make sure frameStart is called before frameFinish
        frameStart( packet->frameID, frameNumber );
    }

    // Not needed: _finishFrame( frameNumber );
    _frameFinish( packet->frameID, frameNumber );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Node::_cmdFrameDrawFinish( eqNet::Command& command )
{
    NodeFrameDrawFinishPacket* packet = 
        command.getPacket< NodeFrameDrawFinishPacket >();
    EQLOG( LOG_TASKS ) << "TASK draw finish " << getName() <<  " " << packet
                       << endl;

    frameDrawFinish( packet->frameID, packet->frameNumber );
    return eqNet::COMMAND_HANDLED;
}
}
