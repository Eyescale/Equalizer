
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "config.h"

#include "client.h"
#include "configEvent.h"
#include "frame.h"
#include "frameData.h"
#include "global.h"
#include "node.h"
#include "nodeFactory.h"
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
                    eqNet::CommandFunc<Config>( this, &Config::_cmdPush ));
    registerCommand( REQ_CONFIG_INIT_REPLY, 
                    eqNet::CommandFunc<Config>( this, &Config::_reqInitReply ));
    registerCommand( CMD_CONFIG_EXIT_REPLY, 
                    eqNet::CommandFunc<Config>( this, &Config::_cmdPush ));
    registerCommand( REQ_CONFIG_EXIT_REPLY, 
                    eqNet::CommandFunc<Config>( this, &Config::_reqExitReply ));
    registerCommand( CMD_CONFIG_START_FRAME_REPLY, 
              eqNet::CommandFunc<Config>( this, &Config::_cmdStartFrameReply ));
    registerCommand( CMD_CONFIG_FINISH_FRAME_REPLY, 
             eqNet::CommandFunc<Config>( this, &Config::_cmdFinishFrameReply ));
    registerCommand( CMD_CONFIG_FINISH_ALL_FRAMES_REPLY, 
         eqNet::CommandFunc<Config>( this, &Config::_cmdFinishAllFramesReply ));
    registerCommand( CMD_CONFIG_EVENT, 
                     eqNet::CommandFunc<Config>( this, &Config::_cmdEvent ));
}

Config::~Config()
{
    _appNodeID = eqNet::NodeID::ZERO;
    _appNode   = 0;
}

eqBase::RefPtr<Client> Config::getClient()
{ 
    RefPtr<eqNet::Node> node = getServer();
    EQASSERT( dynamic_cast< Server* >( node.get( )));

    RefPtr< Server > server = RefPtr_static_cast< eqNet::Node, Server >( node );
    return server->getClient(); 
}

void Config::_addNode( Node* node )
{
    node->_config = this;
    _nodes.push_back( node );
}

void Config::_removeNode( Node* node )
{
    vector<Node*>::iterator i = find( _nodes.begin(), _nodes.end(), node );
    EQASSERT( i != _nodes.end( ));
    _nodes.erase( i );

    node->_config = 0;
}

Node* Config::_findNode( const uint32_t id )
{
    for( vector<Node*>::const_iterator i = _nodes.begin(); i != _nodes.end(); 
         ++i )
    {
        Node* node = *i;
        if( node->getID() == id )
            return node;
    }
    return 0;
}

bool Config::init( const uint32_t initID )
{
    registerObject( &_headMatrix );

    ConfigInitPacket packet;
    packet.requestID    = _requestHandler.registerRequest();
    packet.initID       = initID;
    packet.headMatrixID = _headMatrix.getID();

    send( packet );
    
    RefPtr< Client > client = getClient();
    while( !_requestHandler.isServed( packet.requestID ))
        client->processCommand();
    const bool ret = ( _requestHandler.waitRequest( packet.requestID ) != 0 );

    if( !ret )
        deregisterObject( &_headMatrix );
    return ret;
}

bool Config::exit()
{
    ConfigExitPacket packet;
    packet.requestID = _requestHandler.registerRequest();
    send( packet );

    RefPtr< Client > client = getClient();
    while( !_requestHandler.isServed( packet.requestID ))
        client->processCommand();
    const bool ret = ( _requestHandler.waitRequest( packet.requestID ) != 0 );

    deregisterObject( &_headMatrix );
    while( _eventQueue.tryPop( )); // flush all pending events

    return ret;
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

uint32_t Config::finishFrame()
{
    ConfigFinishFramePacket packet;
    packet.requestID = _requestHandler.registerRequest();

    send( packet );
    const int frameNumber = 
        (uint32_t)(long long)(_requestHandler.waitRequest(packet.requestID));

    handleEvents();
    EQLOG( LOG_ANY ) << "----- Finish Frame ----" << endl;
    return frameNumber;
}

uint32_t Config::finishAllFrames()
{
    ConfigFinishAllFramesPacket packet;
    packet.requestID = _requestHandler.registerRequest();
    send( packet );

    const int framesNumber = 
        (uint32_t)(long long)(_requestHandler.waitRequest(packet.requestID));
    handleEvents();
    EQLOG( LOG_ANY ) << "-- Finish All Frames --" << endl;
    return framesNumber;
}

void Config::sendEvent( ConfigEvent& event )
{
    EQASSERT( _appNodeID );

    if( !_appNode )
    {
        RefPtr<eqNet::Node> localNode = getLocalNode();
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
    _headMatrix = matrix;
    _headMatrix.commit();
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

    Node* node = Global::getNodeFactory()->createNode();
    
    attachObject( node, packet->nodeID );
    _addNode( node );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdDestroyNode( eqNet::Command& command ) 
{
    const ConfigDestroyNodePacket* packet =
        command.getPacket<ConfigDestroyNodePacket>();
    EQINFO << "Handle destroy node " << packet << endl;

    Node* node = _findNode( packet->nodeID );
    if( !node )
        return eqNet::COMMAND_HANDLED;

    _removeNode( node );
    detachObject( node );
    delete node;

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_reqInitReply( eqNet::Command& command )
{
    const ConfigInitReplyPacket* packet = 
        command.getPacket<ConfigInitReplyPacket>();
    EQINFO << "handle init reply " << packet << endl;

    _error = packet->error;
    _requestHandler.serveRequest( packet->requestID, (void*)(packet->result) );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_reqExitReply( eqNet::Command& command )
{
    const ConfigExitReplyPacket* packet = 
        command.getPacket<ConfigExitReplyPacket>();
    EQINFO << "handle exit reply " << packet << endl;

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

eqNet::CommandResult Config::_cmdFinishFrameReply( eqNet::Command& command )
{
    const ConfigFinishFrameReplyPacket* packet = 
        command.getPacket<ConfigFinishFrameReplyPacket>();
    EQVERB << "handle frame finish reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID,
                                  (void*)(long long)(packet->result) );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdFinishAllFramesReply( eqNet::Command& command )
{
    const ConfigFinishAllFramesReplyPacket* packet = 
        command.getPacket<ConfigFinishAllFramesReplyPacket>();
    EQVERB << "handle all frames finish reply " << packet << endl;

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
