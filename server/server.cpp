
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "server.h"

#include "channel.h"
#include "compound.h"
#include "config.h"
#include "global.h"
#include "loader.h"
#include "node.h"
#include "pipe.h"
#include "serverVisitor.h"
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

namespace eq
{
namespace server
{
typedef net::CommandFunc<Server> ServerFunc;

Server::Server()
        : _configID(0)
{
    registerCommand( eq::CMD_SERVER_CHOOSE_CONFIG,
                     ServerFunc( this, &Server::_cmdChooseConfig ),
                     &_serverThreadQueue );
    registerCommand( eq::CMD_SERVER_USE_CONFIG,
                     ServerFunc( this, &Server::_cmdUseConfig ),
                     &_serverThreadQueue );
    registerCommand( eq::CMD_SERVER_RELEASE_CONFIG,
                     ServerFunc( this, &Server::_cmdReleaseConfig ),
                     &_serverThreadQueue );
    registerCommand( eq::CMD_SERVER_SHUTDOWN,
                     ServerFunc( this, &Server::_cmdShutdown ),
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

namespace
{
template< class C, class V >
VisitorResult _accept( C* server, V& visitor )
{ 
    VisitorResult result = visitor.visitPre( server );
    if( result != TRAVERSE_CONTINUE )
        return result;

    const ConfigVector& configs = server->getConfigs();
    for( ConfigVector::const_iterator i = configs.begin();
         i != configs.end(); ++i )
    {
        switch( (*i)->accept( visitor ))
        {
            case TRAVERSE_TERMINATE:
                return TRAVERSE_TERMINATE;

            case TRAVERSE_PRUNE:
                result = TRAVERSE_PRUNE;
                break;
                
            case TRAVERSE_CONTINUE:
            default:
                break;
        }
    }

    switch( visitor.visitPost( server ))
    {
        case TRAVERSE_TERMINATE:
            return TRAVERSE_TERMINATE;

        case TRAVERSE_PRUNE:
            return TRAVERSE_PRUNE;
                
        case TRAVERSE_CONTINUE:
        default:
            break;
    }

    return result;
}
}

VisitorResult Server::accept( ServerVisitor& visitor )
{
    return _accept( this, visitor );
}

VisitorResult Server::accept( ConstServerVisitor& visitor ) const
{
    return _accept( this, visitor );
}

bool Server::run()
{
    EQASSERT( getState() == net::Node::STATE_LISTENING );

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

void Server::registerConfig( Config* config )
{
    if( config->getName().empty( ))
    {
        ostringstream stringStream;
        stringStream << "EQ_CONFIG_" << (++_configID);
        config->setName( stringStream.str( ));
    }

    EQCHECK( registerSession( config ));
}

//===========================================================================
// packet handling methods
//===========================================================================
bool Server::dispatchCommand( net::Command& command )
{
    switch( command->datatype )
    {
        case eq::DATATYPE_EQ_SERVER:
            return net::Dispatcher::dispatchCommand( command );
            
        default:
            return net::Node::dispatchCommand( command );
    }
}

net::CommandResult Server::invokeCommand( net::Command& command )
{
    switch( command->datatype )
    {
        case eq::DATATYPE_EQ_SERVER:
            return net::Dispatcher::invokeCommand( command );
            
        default:
            return net::Node::invokeCommand( command );
    }
}

void Server::_handleCommands()
{
    _running = true;
    while( _running ) // set to false in _cmdShutdown()
    {
        net::Command* command = _serverThreadQueue.pop();

        switch( invokeCommand( *command ))
        {
            case net::COMMAND_HANDLED:
            case net::COMMAND_DISCARD:
                break;

            case net::COMMAND_ERROR:
                EQERROR << "Error handling command " << command << endl;
                abort();
            default:
                EQERROR << "Unknown command result" << endl;
                abort();
        }

        command->release();
    }
    _serverThreadQueue.flush();
}

net::CommandResult Server::_cmdChooseConfig( net::Command& command ) 
{
    const eq::ServerChooseConfigPacket* packet = 
        command.getPacket<eq::ServerChooseConfigPacket>();
    EQINFO << "Handle choose config " << packet << endl;

    // TODO
    Config* config = _configs.empty() ? 0 : _configs[0];
    
    eq::ServerChooseConfigReplyPacket reply( packet );
    net::NodePtr node = command.getNode();

    if( !config )
    {
        reply.configID = EQ_ID_INVALID;
        node->send( reply );
        return net::COMMAND_HANDLED;
    }

    Config* appConfig = new Config( *config );
    appConfig->setApplicationNetNode( node );

    registerConfig( appConfig );

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
    createConfigPacket.objectID  = appConfig->getDistributorID();
    createConfigPacket.appNodeID = node->getNodeID();
    createConfigPacket.appNodeID.convertToNetwork();
    node->send( createConfigPacket, name );

    reply.configID = configID;
    node->send( reply );

    return net::COMMAND_HANDLED;
}

net::CommandResult Server::_cmdUseConfig( net::Command& command ) 
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
    net::NodePtr               node = command.getNode();

    if( !config )
    {
        EQWARN << "Use config parsing failed " << endl;
        reply.configID = EQ_ID_INVALID;
        node->send( reply );
        return net::COMMAND_HANDLED;
    }

    EQINFO << "Using config: " << endl << Global::instance() << config << endl;
    config->setApplicationNetNode( node );
    config->_server = this;
    registerConfig( config );

    const uint32_t configID = config->getID();
    config->setWorkDir( workDir );
    config->setRenderClient( renderClient );
    _appConfigs[configID] = config;

    const string& name = config->getName();

    eq::ServerCreateConfigPacket createConfigPacket;
    createConfigPacket.configID  = configID;
    createConfigPacket.objectID  = config->getDistributorID();
    createConfigPacket.appNodeID = node->getNodeID();
    createConfigPacket.appNodeID.convertToNetwork();
    node->send( createConfigPacket, name );

    reply.configID = configID;
    node->send( reply );
    return net::COMMAND_HANDLED;
}

net::CommandResult Server::_cmdReleaseConfig( net::Command& command )
{
    const eq::ServerReleaseConfigPacket* packet = 
        command.getPacket<eq::ServerReleaseConfigPacket>();
    EQINFO << "Handle release config " << packet << endl;

    eq::ServerReleaseConfigReplyPacket reply( packet );
    Config*                            config = _appConfigs[packet->configID];
    net::NodePtr                node   = command.getNode();

    if( !config )
    {
        EQWARN << "Release request for unknown config" << endl;
        node->send( reply );
        return net::COMMAND_HANDLED;
    }

    if( config->isRunning( ))
    {
        EQWARN << "Release of running configuration" << endl;
        config->exit(); // Make sure config is exited
    }

    config->unmap();

    eq::ServerDestroyConfigPacket destroyConfigPacket;
    destroyConfigPacket.configID  = config->getID();
    node->send( destroyConfigPacket );

    EQCHECK( deregisterSession( config ));

    _appConfigs.erase( packet->configID );
    delete config;

    node->send( reply );
    EQLOG( LOG_ANY ) << "----- Released Config -----" << endl;

    return net::COMMAND_HANDLED;
}

net::CommandResult Server::_cmdShutdown( net::Command& command )
{
    const eq::ServerShutdownPacket* packet = 
        command.getPacket< eq::ServerShutdownPacket >();
    EQINFO << "Handle shutdown " << packet << endl;

    eq::ServerShutdownReplyPacket reply( packet );

    reply.result = _appConfigs.empty();
    if( reply.result )
        _running = false;
    else
        EQWARN << "Ignoring shutdown request, " << _appConfigs.size() 
               << " configs still active" << endl;

    net::NodePtr node = command.getNode();
    node->send( reply );
    return net::COMMAND_HANDLED;
}

std::ostream& operator << ( std::ostream& os, const Server* server )
{
    if( !server )
        return os;
    
    os << disableFlush << disableHeader << "server " << endl;
    os << "{" << endl << indent;
    
    const net::ConnectionDescriptionVector& cds =
        server->getConnectionDescriptions();
    for( net::ConnectionDescriptionVector::const_iterator i = cds.begin();
         i != cds.end(); ++i )
        
        os << static_cast< const ConnectionDescription* >( (*i).get( ));

    const ConfigVector& configs = server->getConfigs();
    for( ConfigVector::const_iterator i = configs.begin();
         i != configs.end(); ++i )

        os << *i;
    
    os << exdent << "}"  << enableHeader << enableFlush << endl;

    return os;
}

}
}
