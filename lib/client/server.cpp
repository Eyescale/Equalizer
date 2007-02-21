
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "server.h"

#include "client.h"
#include "config.h"
#include "configParams.h"
#include "global.h"
#include "node.h"
#include "nodeFactory.h"
#include "packets.h"

#include <eq/net/command.h>
#include <eq/net/connection.h>

using namespace eq;
using namespace eqBase;
using namespace std;

Server::Server()
{
    registerCommand( CMD_SERVER_CREATE_CONFIG, 
                 eqNet::CommandFunc<Server>( this, &Server::_cmdCreateConfig ));
    registerCommand( CMD_SERVER_DESTROY_CONFIG, 
                eqNet::CommandFunc<Server>( this, &Server::_cmdDestroyConfig ));
    registerCommand( CMD_SERVER_CHOOSE_CONFIG_REPLY, 
            eqNet::CommandFunc<Server>( this, &Server::_cmdChooseConfigReply ));

    EQINFO << "New server at " << (void*)this << endl;
}

Server::~Server()
{
    EQINFO << "Delete server at " << (void*)this << endl;
}


Config* Server::chooseConfig( const ConfigParams& parameters )
{
    if( !isConnected( ))
        return 0;

    ServerChooseConfigPacket packet;

    packet.requestID      = _requestHandler.registerRequest();
    string rendererInfo   = parameters.workDir + '#' + parameters.renderClient;
#ifdef WIN32 // replace dir delimeters since '\' is often used as escape char
    for( size_t i=0; i<rendererInfo.length(); ++i )
        if( rendererInfo[i] == '\\' )
            rendererInfo[i] = '/';
#endif
    send( packet, rendererInfo );

    Config* config = static_cast<Config*>(
        _requestHandler.waitRequest( packet.requestID ));
    if( !config )
        return 0;

    return config;
}

void Server::releaseConfig( Config* config )
{
    if( !isConnected( ))
        return;

    ServerReleaseConfigPacket packet;
    packet.configID = config->getID();
    send( packet );
    delete config;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
eqNet::CommandResult Server::_cmdCreateConfig( eqNet::Command& command )
{
    const ServerCreateConfigPacket* packet = 
        command.getPacket<ServerCreateConfigPacket>();
    EQINFO << "Handle create config " << packet << ", name " << packet->name 
           << endl;
    
    RefPtr<Node> localNode = command.getLocalNode();
    Config*      config    = Global::getNodeFactory()->createConfig();

    EQASSERT( localNode->getSession( packet->configID ) == 0 );
    config->_appNodeID = packet->appNodeID;
    localNode->addSession( config, command.getNode(), packet->configID,
                           packet->name );

    EQASSERT( dynamic_cast<Client*>( localNode.get( )));
    Client* client = static_cast<Client*>( localNode.get( ));
    client->refUsed();

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Server::_cmdDestroyConfig( eqNet::Command& command )
{
    const ServerDestroyConfigPacket* packet = 
        command.getPacket<ServerDestroyConfigPacket>();
    EQINFO << "Handle destroy config " << packet << endl;
    
    RefPtr<Node>    localNode  = command.getLocalNode();
    eqNet::Session* session    = localNode->getSession( packet->configID );

    EQASSERTINFO( dynamic_cast<Config*>( session ), typeid(*session).name( ));
    Config* config = static_cast<Config*>( session );

    localNode->removeSession( config );
    delete config;

    EQASSERT( dynamic_cast<Client*>( localNode.get( )));
    Client* client = static_cast<Client*>( localNode.get( ));
    client->unrefUsed();

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Server::_cmdChooseConfigReply( eqNet::Command& command )
{
    const ServerChooseConfigReplyPacket* packet = 
        command.getPacket<ServerChooseConfigReplyPacket>();
    EQINFO << "Handle choose config reply " << packet << endl;

    if( packet->configID == EQ_ID_INVALID )
    {
        _requestHandler.serveRequest( packet->requestID, 0 );
        return eqNet::COMMAND_HANDLED;
    }

    Config*      config    = Global::getNodeFactory()->createConfig();
    RefPtr<Node> localNode = command.getLocalNode();
 
    EQASSERT( localNode->getSession( packet->configID ) == 0 );
    config->_appNodeID = localNode->getNodeID();
    localNode->addSession( config, command.getNode(), packet->configID, 
                           packet->sessionName);

    _requestHandler.serveRequest( packet->requestID, config );
    return eqNet::COMMAND_HANDLED;
}
