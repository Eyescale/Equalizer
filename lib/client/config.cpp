
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "config.h"

#include "configEvent.h"
#include "frame.h"
#include "frameBuffer.h"
#include "global.h"
#include "node.h"
#include "nodeFactory.h"
#include "object.h"
#include "packets.h"
#include "server.h"

#include <eq/net/global.h>

using namespace eq;
using namespace eqBase;
using namespace std;

Config::Config( const uint32_t nCommands )
        : Session( nCommands, true )
{
    EQASSERT( nCommands >= CMD_CONFIG_CUSTOM );
    registerCommand( CMD_CONFIG_CREATE_NODE, this, reinterpret_cast<CommandFcn>(
                         &eq::Config::_cmdCreateNode ));
    registerCommand( CMD_CONFIG_DESTROY_NODE, this,reinterpret_cast<CommandFcn>(
                         &eq::Config::_cmdDestroyNode ));
    registerCommand( CMD_CONFIG_INIT_REPLY, this, reinterpret_cast<CommandFcn>( 
                         &eq::Config::_cmdInitReply ));
    registerCommand( CMD_CONFIG_EXIT_REPLY, this, reinterpret_cast<CommandFcn>(
                         &eq::Config::_cmdExitReply ));
    registerCommand( CMD_CONFIG_FRAME_BEGIN_REPLY, this, 
                     reinterpret_cast<CommandFcn>(
                         &eq::Config::_cmdBeginFrameReply ));
    registerCommand( CMD_CONFIG_FRAME_END_REPLY, this, 
                     reinterpret_cast<CommandFcn>(
                         &eq::Config::_cmdEndFrameReply ));
    registerCommand( CMD_CONFIG_EVENT, this, reinterpret_cast<CommandFcn>(
                         &eq::Config::_cmdEvent ));
    _headMatrix = NULL;
}

Config::~Config()
{
    _appNodeID = eqNet::NodeID::ZERO;
    _appNode   = NULL;
}
void Config::_addNode( Node* node )
{
    node->_config = this;
}

void Config::_removeNode( Node* node )
{
    node->_config = NULL;
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

uint32_t Config::beginFrame( const uint32_t frameID )
{
    ConfigBeginFramePacket packet;
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
    return frameNumber;
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

ConfigEvent* Config::nextEvent()
{
    ConfigEvent* event;
    _eventQueue.pop( NULL, (eqNet::Packet**)&event );
    return event;
}

void Config::handleEvents()
{
    while( checkEvent( ))
    {
        ConfigEvent* event = nextEvent();
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
        case Object::TYPE_FRAMEBUFFER:
            return new FrameBuffer( data, dataSize );
        default:
            return eqNet::Session::instanciateObject( type, data, dataSize );
    }
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
eqNet::CommandResult Config::_cmdCreateNode( eqNet::Node* node, 
                                             const eqNet::Packet* pkg )
{
    ConfigCreateNodePacket* packet = (ConfigCreateNodePacket*)pkg;
    EQINFO << "Handle create node " << packet << endl;
    EQASSERT( packet->nodeID != EQ_ID_INVALID );

    Node* newNode = Global::getNodeFactory()->createNode();
    
    addRegisteredObject( packet->nodeID, newNode, eqNet::Object::SHARE_NODE );
    _addNode( newNode );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdDestroyNode( eqNet::Node* Node, 
                                              const eqNet::Packet* pkg )
{
    ConfigDestroyNodePacket* packet = (ConfigDestroyNodePacket*)pkg;
    EQINFO << "Handle destroy node " << packet << endl;

    eq::Node* delNode = (eq::Node*)pollObject( packet->nodeID );
    if( !delNode )
        return eqNet::COMMAND_HANDLED;

    delNode->_thread->join(); // wait for node thread termination. move to node
    _removeNode( delNode );
    EQASSERT( delNode->getRefCount() == 1 );
    removeRegisteredObject( delNode, eqNet::Object::SHARE_NODE );

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdInitReply( eqNet::Node* node,
                                            const eqNet::Packet* pkg )
{
    ConfigInitReplyPacket* packet = (ConfigInitReplyPacket*)pkg;
    EQINFO << "handle init reply " << packet << endl;

    if( packet->result )
    {
        _headMatrix = (Matrix4f*)pollObject( packet->headMatrixID );
        EQASSERT( _headMatrix );
    }

    _requestHandler.serveRequest( packet->requestID, (void*)(packet->result) );
    return eqNet::COMMAND_HANDLED;
}
eqNet::CommandResult Config::_cmdExitReply( eqNet::Node* node,
                                            const eqNet::Packet* pkg )
{
    ConfigExitReplyPacket* packet = (ConfigExitReplyPacket*)pkg;
    EQINFO << "handle exit reply " << packet << endl;

    _headMatrix = NULL;

    _requestHandler.serveRequest( packet->requestID, (void*)(packet->result) );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdBeginFrameReply(eqNet::Node* node,
                                                 const eqNet::Packet* pkg )
{
    ConfigBeginFrameReplyPacket* packet = (ConfigBeginFrameReplyPacket*)pkg;
    EQVERB << "handle frame begin reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, 
                                  (void*)(long long)(packet->frameNumber) );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdEndFrameReply( eqNet::Node* node,
                                                const eqNet::Packet* pkg )
{
    ConfigEndFrameReplyPacket* packet = (ConfigEndFrameReplyPacket*)pkg;
    EQVERB << "handle frame end reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID,
                                  (void*)(long long)(packet->result) );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Config::_cmdEvent( eqNet::Node* node,
                                        const eqNet::Packet* packet )
{
    EQVERB << "received config event " << (ConfigEvent* )packet << endl;

    _eventQueue.push( node, packet );
    return eqNet::COMMAND_HANDLED;
}
