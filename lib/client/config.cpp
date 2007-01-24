
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "config.h"

#include "configEvent.h"
#include "frame.h"
#include "frameData.h"
#include "global.h"
#include "node.h"
#include "nodeFactory.h"
#include "object.h"
#include "packets.h"
#include "server.h"

#include <eq/net/command.h>
#include <eq/net/global.h>

using namespace eq;
using namespace eqBase;
using namespace std;

Config::Config()
        : Session( true )
{
    registerCommand( CMD_CONFIG_CREATE_NODE,
                   eqNet::CommandFunc<Config>( this, &Config::_cmdCreateNode ));
    registerCommand( CMD_CONFIG_DESTROY_NODE,
                  eqNet::CommandFunc<Config>( this, &Config::_cmdDestroyNode ));
    registerCommand( CMD_CONFIG_INIT_REPLY, 
                    eqNet::CommandFunc<Config>( this, &Config::_cmdInitReply ));
    registerCommand( CMD_CONFIG_EXIT_REPLY, 
                    eqNet::CommandFunc<Config>( this, &Config::_cmdExitReply ));
    registerCommand( CMD_CONFIG_START_FRAME_REPLY, 
              eqNet::CommandFunc<Config>( this, &Config::_cmdStartFrameReply ));
    registerCommand( CMD_CONFIG_END_FRAME_REPLY, 
                eqNet::CommandFunc<Config>( this, &Config::_cmdEndFrameReply ));
    registerCommand( CMD_CONFIG_FINISH_FRAMES_REPLY, 
            eqNet::CommandFunc<Config>( this, &Config::_cmdFinishFramesReply ));
    registerCommand( CMD_CONFIG_EVENT, 
                     eqNet::CommandFunc<Config>( this, &Config::_cmdEvent ));

    _headMatrix = 0;
}

Config::~Config()
{
    _appNodeID = eqNet::NodeID::ZERO;
    _appNode   = 0;
}
void Config::_addNode( Node* node )
{
    node->_config = this;
}

void Config::_removeNode( Node* node )
{
    node->_config = 0;
}

bool Config::init( const uint32_t initID )
{
    ConfigInitPacket packet;
    packet.requestID = _requestHandler.registerRequest();
    packet.initID    = initID;

    send( packet );
    return ( _requestHandler.waitRequest( packet.requestID ) != 0 );
}

bool Config::exit()
{
    ConfigExitPacket packet;
    packet.requestID = _requestHandler.registerRequest();
    send( packet );
    return ( _requestHandler.waitRequest( packet.requestID ) != 0 );
}

uint32_t Config::startFrame( const uint32_t frameID )
{
    EQLOG( LOG_ANY ) << "----- Start Frame -----" << endl;
    ConfigStartFramePacket packet;
    packet.requestID = _requestHandler.registerRequest();
    packet.frameID   = frameID;

    send( packet );
    return (uint32_t)(long long)(_requestHandler.waitRequest(packet.requestID));
}

uint32_t Config::endFrame()
{
    ConfigEndFramePacket packet;
    packet.requestID = _requestHandler.registerRequest();
    send( packet );
    const int frameNumber = 
        (uint32_t)(long long)(_requestHandler.waitRequest(packet.requestID));
    handleEvents();
    EQLOG( LOG_ANY ) << "------ End Frame ------" << endl;
    return frameNumber;
}

uint32_t Config::finishFrames()
{
    ConfigFinishFramesPacket packet;
    packet.requestID = _requestHandler.registerRequest();
    send( packet );
    const int framesNumber = 
        (uint32_t)(long long)(_requestHandler.waitRequest(packet.requestID));
    EQLOG( LOG_ANY ) << "---- Finish Frames ----" << endl;
    return framesNumber;
}

void Config::sendEvent( ConfigEvent& event )
{
    EQASSERT( _appNodeID );

    if( !_appNode )
    {
        RefPtr<eqNet::Node> localNode = eqNet::Node::getLocalNode();
        RefPtr<eqNet::Node> server    = getServer();
        _appNode = localNode->connect( _appNodeID, server );
    }
    EQASSERT( _appNode );

    event.sessionID = getID();
    _appNode->send( event );
}

const ConfigEvent* Config::nextEvent()
{
    eqNet::Command* command = _eventQueue.pop();
    return command->getPacket<ConfigEvent>();
}

void Config::handleEvents()
{
    while( checkEvent( ))
    {
        const ConfigEvent* event = nextEvent();
        if( !handleEvent( event ))
            EQINFO << "Unhandled " << event << endl;
    }
}

void Config::setHeadMatrix( const vmml::Matrix4f& matrix )
{
    EQASSERT( _headMatrix );
    *_headMatrix = matrix;
    _headMatrix->commit();
}

eqNet::Object* Config::instanciateObject( const uint32_t type, const void* data,
                                          const uint64_t dataSize )
{
    switch( type )
    {
        case Object::TYPE_MATRIX4F:
            return new Matrix4f( data, dataSize );
        case Object::TYPE_FRAME:
            return new Frame( data, dataSize );
        case Object::TYPE_FRAMEDATA:
            return new FrameData( data, dataSize );
        default:
            return eqNet::Session::instanciateObject( type, data, dataSize );
    }
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
eqNet::CommandResult Config::_cmdCreateNode( eqNet::Command& command )
{
    const ConfigCreateNodePacket* packet = 
        command.getPacket<ConfigCreateNodePacket>();
    EQINFO << "Handle create node " << packet << endl;
    EQASSERT( packet->nodeID != EQ_ID_INVALID );

    Node* newNode = Global::getNodeFactory()->createNode();
    
    addRegisteredObject( packet->nodeID, newNode, eqNet::Object::SHARE_NODE );
    _addNode( newNode );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdDestroyNode( eqNet::Command& command ) 
{
    const ConfigDestroyNodePacket* packet =
        command.getPacket<ConfigDestroyNodePacket>();
    EQINFO << "Handle destroy node " << packet << endl;

    Node* node = static_cast<Node*>( pollObject( packet->nodeID ));
    if( !node )
        return eqNet::COMMAND_HANDLED;

    node->_thread->join(); // TODO: Move to node?
    _removeNode( node );
    EQASSERT( node->getRefCount() == 1 );
    removeRegisteredObject( node, eqNet::Object::SHARE_NODE );
    
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdInitReply( eqNet::Command& command )
{
    const ConfigInitReplyPacket* packet = 
        command.getPacket<ConfigInitReplyPacket>();
    EQINFO << "handle init reply " << packet << endl;

    if( packet->result )
    {
        _headMatrix = static_cast<Matrix4f*>(pollObject( packet->headMatrixID));
        EQASSERT( _headMatrix );
    }

    _error = packet->error;
    _requestHandler.serveRequest( packet->requestID, (void*)(packet->result) );
    return eqNet::COMMAND_HANDLED;
}
eqNet::CommandResult Config::_cmdExitReply( eqNet::Command& command )
{
    const ConfigExitReplyPacket* packet = 
        command.getPacket<ConfigExitReplyPacket>();
    EQINFO << "handle exit reply " << packet << endl;

    if( _headMatrix )
    {
        EQASSERTINFO( _headMatrix->getRefCount() == 1, 
                      _headMatrix->getRefCount( ));

        removeRegisteredObject( _headMatrix );  // will delete _headMatrix
        _headMatrix = 0;
    }
    _requestHandler.serveRequest( packet->requestID, (void*)(packet->result) );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdStartFrameReply( eqNet::Command& command )
{
    const ConfigStartFrameReplyPacket* packet =
        command.getPacket<ConfigStartFrameReplyPacket>();
    EQVERB << "handle frame start reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, 
                                  (void*)(long long)(packet->frameNumber) );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdEndFrameReply( eqNet::Command& command )
{
    const ConfigEndFrameReplyPacket* packet = 
        command.getPacket<ConfigEndFrameReplyPacket>();
    EQVERB << "handle frame end reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID,
                                  (void*)(long long)(packet->result) );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdFinishFramesReply( eqNet::Command& command )
{
    const ConfigFinishFramesReplyPacket* packet = 
        command.getPacket<ConfigFinishFramesReplyPacket>();
    EQVERB << "handle frames finish reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID,
                                  (void*)(long long)(packet->result) );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdEvent( eqNet::Command& command )
{
    EQVERB << "received config event " << command.getPacket<ConfigEvent>()
           << endl;

    _eventQueue.push( command );
    return eqNet::COMMAND_HANDLED;
}
