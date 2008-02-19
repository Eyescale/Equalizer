
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

using namespace eqBase;
using namespace std;
using eqNet::CommandFunc;

namespace eqs
{
Server::Server()
        : _configID(0)
{
    registerCommand( eq::CMD_SERVER_CHOOSE_CONFIG,
                     CommandFunc<Server>( this, &Server::_cmdChooseConfig ),
                     _serverThreadQueue );
    registerCommand( eq::CMD_SERVER_USE_CONFIG,
                     CommandFunc<Server>( this, &Server::_cmdUseConfig ),
                     _serverThreadQueue );
    registerCommand( eq::CMD_SERVER_RELEASE_CONFIG,
                     CommandFunc<Server>( this, &Server::_cmdReleaseConfig ),
                     _serverThreadQueue );
    registerCommand( eq::CMD_SERVER_SHUTDOWN,
                     CommandFunc<Server>( this, &Server::_cmdShutdown ),
                     _serverThreadQueue );
    EQINFO << "New server @" << (void*)this << endl;
}

bool Server::run()
{
    EQASSERT( getState() == eqNet::Node::STATE_LISTENING );

    if( _configs.empty( ))
    {
        EQERROR << "No configurations loaded" << endl;
        return false;
    }

    EQINFO << disableFlush << "Running server: " << endl << indent 
           << Global::instance() << this << exdent << enableFlush;

    _handleCommands();
    return stopListening();
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
bool Server::dispatchCommand( eqNet::Command& command )
{
    switch( command->datatype )
    {
        case eq::DATATYPE_EQ_SERVER:
            return eqNet::Base::dispatchCommand( command );
            
        default:
            return eqNet::Node::dispatchCommand( command );
    }
}

eqNet::CommandResult Server::invokeCommand( eqNet::Command& command )
{
    switch( command->datatype )
    {
        case eq::DATATYPE_EQ_SERVER:
            return eqNet::Base::invokeCommand( command );
            
        default:
            return eqNet::Node::invokeCommand( command );
    }
}

void Server::_handleCommands()
{
    _running = true;
    while( _running ) // set to false in _cmdShutdown()
    {
        eqNet::Command* command = _serverThreadQueue.pop();

        switch( invokeCommand( *command ))
        {
            case eqNet::COMMAND_HANDLED:
            case eqNet::COMMAND_DISCARD:
                break;

            case eqNet::COMMAND_ERROR:
                EQERROR << "Error handling command " << command << endl;
                abort();
        }
    }
}

eqNet::CommandResult Server::_cmdChooseConfig( eqNet::Command& command ) 
{
    const eq::ServerChooseConfigPacket* packet = 
        command.getPacket<eq::ServerChooseConfigPacket>();
    EQINFO << "Handle choose config " << packet << endl;

    // TODO
    Config* config = _configs.empty() ? 0 : _configs[0];
    
    eq::ServerChooseConfigReplyPacket reply( packet );
    RefPtr<eqNet::Node>               node = command.getNode();

    if( !config )
    {
        reply.configID = EQ_ID_INVALID;
        node->send( reply );
        return eqNet::COMMAND_HANDLED;
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
    node->send( createConfigPacket, name );

    reply.configID = configID;
    node->send( reply );

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Server::_cmdUseConfig( eqNet::Command& command ) 
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
    RefPtr<eqNet::Node>               node = command.getNode();

    if( !config )
    {
        reply.configID = EQ_ID_INVALID;
        node->send( reply );
        return eqNet::COMMAND_HANDLED;
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
    node->send( createConfigPacket, name );

    reply.configID = configID;
    node->send( reply );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Server::_cmdReleaseConfig( eqNet::Command& command )
{
    const eq::ServerReleaseConfigPacket* packet = 
        command.getPacket<eq::ServerReleaseConfigPacket>();
    EQINFO << "Handle release config " << packet << endl;

    eq::ServerReleaseConfigReplyPacket reply( packet );
    Config*                            config = _appConfigs[packet->configID];
    RefPtr<eqNet::Node>                node   = command.getNode();

    if( !config )
    {
        EQWARN << "Release request for unknown config" << endl;
        node->send( reply );
        return eqNet::COMMAND_HANDLED;
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

    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Server::_cmdShutdown( eqNet::Command& command )
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

    RefPtr<eqNet::Node> node = command.getNode();
    node->send( reply );
    return eqNet::COMMAND_HANDLED;
}

std::ostream& operator << ( std::ostream& os, const Server* server )
{
    if( !server )
        return os;
    
    os << disableFlush << disableHeader << "server " << endl;
    os << "{" << endl << indent;
    
    const eqNet::ConnectionDescriptionVector& cds =
        server->getConnectionDescriptions();
    for( eqNet::ConnectionDescriptionVector::const_iterator i = cds.begin();
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
