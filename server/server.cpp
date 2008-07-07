
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "server.h"

#include "channel.h"
#include "compound.h"
#include "config.h"
#include "global.h"
#include "loader.h"
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

using namespace eq::base;
using namespace std;
using eq::net::CommandFunc;

namespace eq
{
namespace server
{
Server::Server()
        : _configID(0)
{
    registerCommand( eq::CMD_SERVER_CHOOSE_CONFIG,
                     CommandFunc<Server>( this, &Server::_cmdChooseConfig ),
                     &_serverThreadQueue );
    registerCommand( eq::CMD_SERVER_USE_CONFIG,
                     CommandFunc<Server>( this, &Server::_cmdUseConfig ),
                     &_serverThreadQueue );
    registerCommand( eq::CMD_SERVER_RELEASE_CONFIG,
                     CommandFunc<Server>( this, &Server::_cmdReleaseConfig ),
                     &_serverThreadQueue );
    registerCommand( eq::CMD_SERVER_SHUTDOWN,
                     CommandFunc<Server>( this, &Server::_cmdShutdown ),
                     &_serverThreadQueue );
    EQINFO << "New server @" << (void*)this << endl;
}

Server::~Server()
{
    for( ConfigVector::const_iterator i = _configs.begin(); 
         i != _configs.end(); ++i )
    {
        Config* config = *i;

        config->_server = 0;
        delete config;
    }

    _configs.clear();
}

bool Server::run()
{
    EQASSERT( getState() == eq::net::Node::STATE_LISTENING );

    if( _configs.empty( ))
    {
        EQERROR << "No configurations loaded" << endl;
        return false;
    }

    EQINFO << disableFlush << "Running server: " << endl << indent 
           << Global::instance() << this << exdent << enableFlush;

    _handleCommands();
    return true;
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
bool Server::dispatchCommand( eq::net::Command& command )
{
    switch( command->datatype )
    {
        case eq::DATATYPE_EQ_SERVER:
            return eq::net::Base::dispatchCommand( command );
            
        default:
            return eq::net::Node::dispatchCommand( command );
    }
}

eq::net::CommandResult Server::invokeCommand( eq::net::Command& command )
{
    switch( command->datatype )
    {
        case eq::DATATYPE_EQ_SERVER:
            return eq::net::Base::invokeCommand( command );
            
        default:
            return eq::net::Node::invokeCommand( command );
    }
}

void Server::_handleCommands()
{
    _running = true;
    while( _running ) // set to false in _cmdShutdown()
    {
        eq::net::Command* command = _serverThreadQueue.pop();

        switch( invokeCommand( *command ))
        {
            case eq::net::COMMAND_HANDLED:
            case eq::net::COMMAND_DISCARD:
                break;

            case eq::net::COMMAND_ERROR:
                EQERROR << "Error handling command " << command << endl;
                abort();
            default:
                EQERROR << "Unknown command result" << endl;
                abort();
        }

        _serverThreadQueue.release( command );
    }
}

eq::net::CommandResult Server::_cmdChooseConfig( eq::net::Command& command ) 
{
    const eq::ServerChooseConfigPacket* packet = 
        command.getPacket<eq::ServerChooseConfigPacket>();
    EQINFO << "Handle choose config " << packet << endl;

    // TODO
    Config* config = _configs.empty() ? 0 : _configs[0];
    
    eq::ServerChooseConfigReplyPacket reply( packet );
    eq::net::NodePtr               node = command.getNode();

    if( !config )
    {
        reply.configID = EQ_ID_INVALID;
        node->send( reply );
        return eq::net::COMMAND_HANDLED;
    }

    Config* appConfig = new Config( *config );
    appConfig->setApplicationNetNode( node );

    mapConfig( appConfig );

    // TODO: move to open?: appConfig->setAppName( appName );
    const string rendererInfo = packet->rendererInfo;
    const size_t colonPos     = rendererInfo.find( '#' );
    const string workDir      = rendererInfo.substr( 0, colonPos );
    const string renderClient = rendererInfo.substr( colonPos + 1 );
 
    const uint32_t configID = appConfig->getID();
    appConfig->setWorkDir( workDir );
    appConfig->setRenderClient( renderClient );
    _appConfigs[configID] = appConfig;

    const string& name = appConfig->getName();

    eq::ServerCreateConfigPacket createConfigPacket;
    createConfigPacket.configID  = configID;
    createConfigPacket.appNodeID = node->getNodeID();
    createConfigPacket.appNodeID.convertToNetwork();
    node->send( createConfigPacket, name );

    reply.configID = configID;
    node->send( reply );

    return eq::net::COMMAND_HANDLED;
}

eq::net::CommandResult Server::_cmdUseConfig( eq::net::Command& command ) 
{
    const eq::ServerUseConfigPacket* packet = 
        command.getPacket<eq::ServerUseConfigPacket>();
    EQINFO << "Handle use config " << packet << endl;

    string       configInfo   = packet->configInfo;
    size_t       colonPos     = configInfo.find( '#' );
    const string workDir      = configInfo.substr( 0, colonPos );
    
    configInfo                = configInfo.substr( colonPos + 1 );
    colonPos                  = configInfo.find( '#' );
    const string renderClient = configInfo.substr( 0, colonPos );
    const string configData   = configInfo.substr( colonPos + 1 );
 
    Loader loader;
    Config* config = loader.parseConfig( configData.c_str( ));

    eq::ServerChooseConfigReplyPacket reply( packet );
    eq::net::NodePtr               node = command.getNode();

    if( !config )
    {
        reply.configID = EQ_ID_INVALID;
        node->send( reply );
        return eq::net::COMMAND_HANDLED;
    }

    EQINFO << "Using config: " << endl << Global::instance() << config << endl;
    config->setApplicationNetNode( node );
    mapConfig( config );

    const uint32_t configID = config->getID();
    config->setWorkDir( workDir );
    config->setRenderClient( renderClient );
    _appConfigs[configID] = config;

    const string& name = config->getName();

    eq::ServerCreateConfigPacket createConfigPacket;
    createConfigPacket.configID  = configID;
    createConfigPacket.appNodeID = node->getNodeID();
    createConfigPacket.appNodeID.convertToNetwork();
    node->send( createConfigPacket, name );

    reply.configID = configID;
    node->send( reply );
    return eq::net::COMMAND_HANDLED;
}

eq::net::CommandResult Server::_cmdReleaseConfig( eq::net::Command& command )
{
    const eq::ServerReleaseConfigPacket* packet = 
        command.getPacket<eq::ServerReleaseConfigPacket>();
    EQINFO << "Handle release config " << packet << endl;

    eq::ServerReleaseConfigReplyPacket reply( packet );
    Config*                            config = _appConfigs[packet->configID];
    eq::net::NodePtr                node   = command.getNode();

    if( !config )
    {
        EQWARN << "Release request for unknown config" << endl;
        node->send( reply );
        return eq::net::COMMAND_HANDLED;
    }

    if( config->isRunning( ))
    {
        EQWARN << "Release of running configuration" << endl;
        config->exit(); // Make sure config is exited
    }

    eq::ServerDestroyConfigPacket destroyConfigPacket;
    destroyConfigPacket.configID  = config->getID();
    node->send( destroyConfigPacket );

    const bool unmapped = unmapSession( config );
    EQASSERT( unmapped );

    _appConfigs.erase( packet->configID );
    delete config;

    node->send( reply );
    EQLOG( LOG_ANY ) << "----- Released Config -----" << endl;

    return eq::net::COMMAND_HANDLED;
}

eq::net::CommandResult Server::_cmdShutdown( eq::net::Command& command )
{
    const eq::ServerShutdownPacket* packet = 
        command.getPacket<eq::ServerShutdownPacket>();
    EQINFO << "Handle shutdown " << packet << endl;

    eq::ServerShutdownReplyPacket reply( packet );

    reply.result = _appConfigs.empty();
    if( reply.result )
        _running = false;
    else
        EQWARN << "Ignoring shutdown request, " << _appConfigs.size() 
               << " configs still active" << endl;

    eq::net::NodePtr node = command.getNode();
    node->send( reply );
    return eq::net::COMMAND_HANDLED;
}

std::ostream& operator << ( std::ostream& os, const Server* server )
{
    if( !server )
        return os;
    
    os << disableFlush << disableHeader << "server " << endl;
    os << "{" << endl << indent;
    
    const eq::net::ConnectionDescriptionVector& cds =
        server->getConnectionDescriptions();
    for( eq::net::ConnectionDescriptionVector::const_iterator i = cds.begin();
         i != cds.end(); ++i )
        
        os << (*i).get();

    const ConfigVector& configs = server->getConfigs();
    for( ConfigVector::const_iterator i = configs.begin();
         i != configs.end(); ++i )

        os << *i;
    
    os << exdent << "}"  << enableHeader << enableFlush << endl;

    return os;
}

}
}
