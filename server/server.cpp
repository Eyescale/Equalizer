
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "server.h"

#include "channel.h"
#include "compound.h"
#include "config.h"
#include "global.h"
#include "node.h"
#include "pipe.h"
#include "window.h"

#include <eq/base/refPtr.h>
#include <eq/net/command.h>
#include <eq/net/connectionDescription.h>
#include <eq/net/init.h>
#include <eq/net/node.h>
#include <eq/client/packets.h>

#include <sstream>

using namespace eqs;
using namespace eqBase;
using namespace std;

Server::Server()
        : _configID(0)
{
    registerCommand( eq::CMD_SERVER_CHOOSE_CONFIG,
                     eqNet::CommandFunc<Server>( this, &Server::_cmdPush ));
    registerCommand( eq::REQ_SERVER_CHOOSE_CONFIG, 
                  eqNet::CommandFunc<Server>( this, &Server::_reqChooseConfig ));
    registerCommand( eq::CMD_SERVER_RELEASE_CONFIG,
                     eqNet::CommandFunc<Server>( this, &Server::_cmdPush ));
    registerCommand( eq::REQ_SERVER_RELEASE_CONFIG,
                 eqNet::CommandFunc<Server>( this, &Server::_reqReleaseConfig ));
    EQINFO << "New server @" << (void*)this << endl;
}

bool Server::run()
{
    RefPtr<eqNet::Connection> connection =
        eqNet::Connection::create( eqNet::Connection::TYPE_TCPIP );
    RefPtr<eqNet::ConnectionDescription> connDesc = 
        connection->getDescription();

    connDesc->TCPIP.port = EQ_DEFAULT_PORT;
    if( !connection->listen( ))
    {
        EQERROR << "Could not create listening socket" << endl;
        return false;
    }
    if( !listen( connection ))
        return false;

    if( nConfigs() == 0 )
    {
        EQERROR << "No configurations loaded" << endl;
        return false;
    }

    EQINFO << disableFlush << "Running server: " << endl << indent 
           << Global::instance() << this << exdent << enableFlush;

    _handleCommands();
    return stopListening();
}

void Server::handleDisconnect( Node* node )
{
    Node::handleDisconnect( node );
    // TODO: free up resources requested by disconnected node
}

void Server::addConfig( Config* config )
{ 
    config->_server = this;
    _configs.push_back( config );
}

void Server::mapConfig( Config* config )
{
    ostringstream stringStream;
    stringStream << "EQ_CONFIG_" << (++_configID);

    const bool mapped = mapSession( this, config, stringStream.str( ));
    EQASSERT( mapped );
}

//===========================================================================
// packet handling methods
//===========================================================================
eqNet::CommandResult Server::handleCommand( eqNet::Command& command )
{
    switch( command->datatype )
    {
        case eq::DATATYPE_EQ_SERVER:
            return invokeCommand( command );
            
        default:
            EQUNIMPLEMENTED;
            return eqNet::COMMAND_ERROR;
    }
}

void Server::_handleCommands()
{
    while( true )
    {
        eqNet::Command* command = _commandQueue.pop();

        switch( dispatchCommand( *command ))
        {
            case eqNet::COMMAND_HANDLED:
            case eqNet::COMMAND_DISCARD:
                break;

            case eqNet::COMMAND_ERROR:
                EQERROR << "Error handling command " << command << endl;
                abort();

            case eqNet::COMMAND_PUSH:
                EQUNIMPLEMENTED;
            case eqNet::COMMAND_PUSH_FRONT:
                EQUNIMPLEMENTED;
            case eqNet::COMMAND_REDISPATCH:
                EQUNIMPLEMENTED;
        }
    }
}

eqNet::CommandResult Server::_reqChooseConfig( eqNet::Command& command ) 
{
    const eq::ServerChooseConfigPacket* packet = 
        command.getPacket<eq::ServerChooseConfigPacket>();
    EQINFO << "Handle choose config " << packet << endl;

    // TODO
    Config* config = nConfigs()>0 ? getConfig(0) : NULL;
    
    eq::ServerChooseConfigReplyPacket reply( packet );
    RefPtr<eqNet::Node>               node = command.getNode();

    if( config==NULL )
    {
        reply.configID = EQ_ID_INVALID;
        node->send( reply );
        return eqNet::COMMAND_HANDLED;
    }

    Config* appConfig = new Config( *config );
    appConfig->setApplicationNode( node );

    mapConfig( appConfig );

    // TODO: move to open: appConfig->setAppName( appName );
    const string rendererInfo = packet->rendererInfo;
    const size_t colonPos     = rendererInfo.find( ':' );
    const string workDir      = rendererInfo.substr( 0, colonPos );
    const string renderClient = rendererInfo.substr( colonPos + 1 );
 
    appConfig->setWorkDir( workDir );
    appConfig->setRenderClient( renderClient );

    reply.configID = appConfig->getID();
    _appConfigs[reply.configID] = appConfig;

    const string& name = appConfig->getName();

    node->send( reply, name );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Server::_reqReleaseConfig( eqNet::Command& command )
{
    const eq::ServerReleaseConfigPacket* packet = 
        command.getPacket<eq::ServerReleaseConfigPacket>();
    EQINFO << "Handle release config " << packet << endl;

    Config* config = _appConfigs[packet->configID];
    if( !config )
    {
        EQWARN << "Release request for unknown config" << endl;
        return eqNet::COMMAND_HANDLED;
    }

    config->exit(); // Make sure config is exited
    const bool unmapped = unmapSession( config );
    EQASSERT( unmapped );

    _appConfigs.erase( packet->configID );
    delete config;
    return eqNet::COMMAND_HANDLED;
}

std::ostream& eqs::operator << ( std::ostream& os, const Server* server )
{
    if( !server )
        return os;
    
    const uint32_t nConfigs = server->nConfigs();
    
    os << disableFlush << disableHeader << "server " << endl;
    os << "{" << endl << indent;
    
    for( uint32_t i=0; i<nConfigs; i++ )
        os << server->getConfig(i);
    
    os << exdent << "}"  << enableHeader << enableFlush << endl;

    return os;
}
