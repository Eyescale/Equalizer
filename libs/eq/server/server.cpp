
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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
#include "nodeFactory.h"
#include "pipe.h"
#include "window.h"
#ifdef EQ_USE_GPUSD
#  include "config/server.h"
#  include <eq/client/global.h>
#endif

#include <co/command.h>
#include <co/connectionDescription.h>
#include <co/global.h>
#include <co/init.h>
#include <co/localNode.h>
#include <lunchbox/refPtr.h>
#include <lunchbox/sleep.h>

#include <sstream>

#include "configBackupVisitor.h"
#include "configRestoreVisitor.h"

namespace eq
{
namespace server
{
namespace
{
static NodeFactory _nf;
}

typedef co::CommandFunc<Server> ServerFunc;
typedef fabric::Server< co::Node, Server, Config, NodeFactory, co::LocalNode,
                        ServerVisitor > Super;

Server::Server()
        : Super( &_nf )
        , _running( false )
{
    lunchbox::Log::setClock( &_clock );
    disableInstanceCache();

    registerCommand( fabric::CMD_SERVER_CHOOSE_CONFIG,
                     ServerFunc( this, &Server::_cmdChooseConfig ),
                     &_mainThreadQueue );
    registerCommand( fabric::CMD_SERVER_RELEASE_CONFIG,
                     ServerFunc( this, &Server::_cmdReleaseConfig ),
                     &_mainThreadQueue );
    registerCommand( fabric::CMD_SERVER_DESTROY_CONFIG_REPLY,
                     ServerFunc( this, &Server::_cmdDestroyConfigReply ), 0 );
    registerCommand( fabric::CMD_SERVER_SHUTDOWN,
                     ServerFunc( this, &Server::_cmdShutdown ),
                     &_mainThreadQueue );
    registerCommand( fabric::CMD_SERVER_MAP,
                     ServerFunc( this, &Server::_cmdMap ), &_mainThreadQueue );
    registerCommand( fabric::CMD_SERVER_UNMAP,
                     ServerFunc( this, &Server::_cmdUnmap ),
                     &_mainThreadQueue );
}

Server::~Server()
{
    LBASSERT( getConfigs().empty( )); // not possible - config RefPtr's myself
    deleteConfigs();
    lunchbox::Log::setClock( 0 );
}

void Server::init()
{
    lunchbox::Thread::setName( lunchbox::className( this ));
    LBASSERT( isListening( ));

    const Configs& configs = getConfigs();
#ifndef EQ_USE_GPUSD
    if( configs.empty( ))
        LBWARN << "No configurations loaded" << std::endl;
#endif

    LBINFO << lunchbox::disableFlush << "Running server: " << std::endl
           << lunchbox::indent << Global::instance() << *this
           << lunchbox::exdent << lunchbox::enableFlush << std::endl;

    for( Configs::const_iterator i = configs.begin(); i != configs.end(); ++i )
        (*i)->register_();
}

void Server::exit()
{
    const Configs& configs = getConfigs();
    for( Configs::const_iterator i = configs.begin(); i != configs.end(); ++i )
        (*i)->deregister();
}

void Server::run()
{
    init();
    handleCommands();
    exit();
}

void Server::deleteConfigs()
{
    const Configs& configs = getConfigs();
    while( !configs.empty( ))
    {
        Config* config = configs.back();
        _removeConfig( config );
        delete config;
    }
}

//===========================================================================
// packet handling methods
//===========================================================================
bool Server::dispatchCommand( co::CommandPtr command )
{
    switch( (*command)->type )
    {
        case fabric::PACKETTYPE_EQ_SERVER:
            return co::Dispatcher::dispatchCommand( command );

        default:
            return co::LocalNode::dispatchCommand( command );
    }
}


void Server::handleCommands()
{
    _running = true;
    while( _running ) // set to false in _cmdShutdown()
    {
        co::CommandPtr command = _mainThreadQueue.pop();
        if( !(*command)( ))
        {
            LBABORT( "Error handling command " << command );
        }
    }
    _mainThreadQueue.flush();
}

bool Server::_cmdChooseConfig( co::Command& command )
{
    LBVERB << "Handle choose config " << command << std::endl;

    Config* config = 0;
    const Configs& configs = getConfigs();
    for( Configs::const_iterator i = configs.begin();
         i != configs.end() && !config; ++i )
    {
        Config* candidate = *i;
        const float version = candidate->getFAttribute( Config::FATTR_VERSION );
        LBASSERT( version == 1.2f );
        if( !candidate->isUsed() && version == 1.2f )
            config = candidate;
    }

    const uint32_t requestID = command.get< uint32_t >();
    const uint32_t flags = command.get< uint32_t >();

#ifdef EQ_USE_GPUSD
    if( !config )
    {
        // TODO move session name to ConfigParams
        config = config::Server::configure( this, eq::Global::getConfigFile(),
                                            flags );
        if( config )
        {
            config->register_();
            LBINFO << "Configured " << *this << std::endl;
        }
    }
#endif

    co::NodePtr node = command.getNode();

    if( !config )
    {
        node->send( fabric::CMD_SERVER_CHOOSE_CONFIG_REPLY,
                    PACKETTYPE_EQ_SERVER )
                << UUID::ZERO << requestID;
        return true;
    }

    ConfigBackupVisitor backup;
    config->accept( backup );

    const std::string  rendererInfo = command.get< std::string >();
    const size_t       colonPos     = rendererInfo.find( '#' );
    const std::string  workDir      = rendererInfo.substr( 0, colonPos );
    const std::string  renderClient = rendererInfo.substr( colonPos + 1 );

    config->setApplicationNetNode( node );
    config->setWorkDir( workDir );
    config->setRenderClient( renderClient );
    config->commit();

    node->send( CMD_SERVER_CREATE_CONFIG, PACKETTYPE_EQ_SERVER )
            << co::ObjectVersion( config ) << LB_UNDEFINED_UINT32;

    server::Node* appNode = config->findApplicationNode();
    const co::ConnectionDescriptions& descs =
        appNode->getConnectionDescriptions();

    if( config->getNodes().size() > 1 )
    {
        if( descs.empty() && node->getConnectionDescriptions().empty( ))
        {
            LBWARN << "Likely misconfiguration: Neither the application nor the"
                   << " config file has a connection for this multi-node "
                   << "config. Render clients will be unable to communicate "
                   << "with the application process." << std::endl;
        }
        if( getConnectionDescriptions().empty( ))
        {
            LBWARN << "Likely misconfiguration: The server has no listening "
                   << "connection for this multi-node config. Render clients "
                   << "will be unable to communicate with the server."
                   << std::endl;
        }
    }

    node->send( fabric::CMD_SERVER_CHOOSE_CONFIG_REPLY, PACKETTYPE_EQ_SERVER )
            << config->getID() << requestID << co::serialize( descs );
    return true;
}

bool Server::_cmdReleaseConfig( co::Command& command )
{
    LBVERB << "Handle release config " << command << std::endl;

    UUID configID = command.get< UUID >();
    uint32_t requestID = command.get< uint32_t >();

    co::NodePtr node = command.getNode();

    Config* config = 0;
    const Configs& configs = getConfigs();
    for( Configs::const_iterator i = configs.begin();
         i != configs.end() && !config; ++i )
    {
        Config* candidate = *i;
        if( candidate->getID() == configID )
            config = candidate;
    }

    if( !config )
    {
        LBWARN << "Release request for unknown config" << std::endl;
        node->send( fabric::CMD_SERVER_RELEASE_CONFIG_REPLY,
                    PACKETTYPE_EQ_SERVER )
                << requestID;
        return true;
    }

    if( config->isRunning( ))
    {
        LBWARN << "Release of running configuration" << std::endl;
        config->exit(); // Make sure config is exited
    }

    const uint32_t destroyRequestID = registerRequest();
    node->send( fabric::CMD_SERVER_DESTROY_CONFIG, PACKETTYPE_EQ_SERVER )
            << config->getID() << destroyRequestID;
    waitRequest( destroyRequestID );

#ifdef EQ_USE_GPUSD
    if( config->isAutoConfig( ))
    {
        LBASSERT( _admins.empty( ));
        config->deregister();
        delete config;
    }
    else
#endif
    {
        ConfigRestoreVisitor restore;
        config->accept( restore );
        config->commit();
    }

    node->send( fabric::CMD_SERVER_RELEASE_CONFIG_REPLY, PACKETTYPE_EQ_SERVER )
            << requestID;
    LBLOG( lunchbox::LOG_ANY ) << "----- Released Config -----" << std::endl;
    return true;
}

bool Server::_cmdDestroyConfigReply( co::Command& command )
{
    serveRequest( command.get< uint32_t >( ));
    return true;
}

bool Server::_cmdShutdown( co::Command& command )
{
    const uint32_t requestID = command.get< uint32_t >();

    co::NodePtr node = command.getNode();

    if( !_admins.empty( ))
    {
        LBWARN << "Ignoring shutdown request, " << _admins.size()
               << " admin clients connected" << std::endl;

        node->send( fabric::CMD_SERVER_SHUTDOWN_REPLY, PACKETTYPE_EQ_SERVER )
                << requestID << false;
        return true;
    }

    const Configs& configs = getConfigs();
    for( Configs::const_iterator i = configs.begin(); i != configs.end(); ++i )
    {
        Config* candidate = *i;
        if( candidate->isUsed( ))
        {
            LBWARN << "Ignoring shutdown request due to used config"
                   << std::endl;

            node->send( fabric::CMD_SERVER_SHUTDOWN_REPLY, PACKETTYPE_EQ_SERVER)
                    << requestID << false;
            return true;
        }
    }

    LBINFO << "Shutting down server" << std::endl;

    _running = false;
    node->send( fabric::CMD_SERVER_SHUTDOWN_REPLY, PACKETTYPE_EQ_SERVER )
            << requestID << true;

#ifndef WIN32
    // WAR for 2874188: Lockup at shutdown
    lunchbox::sleep( 100 );
#endif

    return true;
}

bool Server::_cmdMap( co::Command& command )
{
    co::NodePtr node = command.getNode();
    _admins.push_back( node );

    const Configs& configs = getConfigs();
    for( Configs::const_iterator i = configs.begin(); i != configs.end(); ++i )
    {
        Config* config = *i;
        node->send( CMD_SERVER_CREATE_CONFIG, PACKETTYPE_EQ_SERVER )
                << co::ObjectVersion( config ) << LB_UNDEFINED_UINT32;
    }

    node->send( fabric::CMD_SERVER_MAP_REPLY, fabric::PACKETTYPE_EQ_SERVER )
            << command.get< uint32_t >();
    return true;
}

bool Server::_cmdUnmap( co::Command& command )
{
    co::NodePtr node = command.getNode();
    co::Nodes::iterator i = stde::find( _admins, node );

    LBASSERT( i != _admins.end( ));
    if( i != _admins.end( ))
    {
        _admins.erase( i );
        const Configs& configs = getConfigs();
        for( Configs::const_iterator j = configs.begin();
             j != configs.end(); ++j )
        {
            Config* config = *j;
            node->send( CMD_SERVER_DESTROY_CONFIG )
                    << config->getID() << LB_UNDEFINED_UINT32;
        }
    }

    node->send( fabric::CMD_SERVER_UNMAP_REPLY, fabric::PACKETTYPE_EQ_SERVER )
            << command.get< uint32_t >();
    return true;
}

}
}
#include "../fabric/server.ipp"
template class eq::fabric::Server< co::Node, eq::server::Server,
                                   eq::server::Config,
                                   eq::server::NodeFactory,
                                   co::LocalNode, eq::server::ServerVisitor >;

/** @cond IGNORE */
template std::ostream&
eq::fabric::operator << ( std::ostream&, const eq::server::Super& );
/** @endcond */
