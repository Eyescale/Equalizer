
/* Copyright (c) 2005-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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
#ifdef EQUALIZER_USE_HWSD
#  include "config/server.h"
#endif

#include <eq/fabric/commands.h>
#include <eq/fabric/configParams.h>

#include <co/iCommand.h>
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
        , _mainThreadQueue( co::Global::getCommandQueueLimit( ))
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
    lunchbox::Thread::setName( "Server" );
    LBASSERT( isListening( ));

    const Configs& configs = getConfigs();
#ifndef EQUALIZER_USE_HWSD
    if( configs.empty( ))
        LBWARN << "No configurations loaded" << std::endl;
#endif

    LBDEBUG << lunchbox::disableFlush << lunchbox::disableHeader
           << "Running server: " << std::endl
           << lunchbox::indent << Global::instance() << *this
           << lunchbox::exdent << lunchbox::enableHeader
           << lunchbox::enableFlush << std::endl;

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
// ICommand handling methods
//===========================================================================
void Server::handleCommands()
{
    _running = true;
    while( _running ) // set to false in _cmdShutdown()
    {
        const co::ICommands& commands = _mainThreadQueue.popAll();
        LBASSERT( !commands.empty( ));

        for( co::ICommandsCIter i = commands.begin(); i != commands.end(); ++i )
        {
            // We want to avoid a non-const copy of commands, hence the cast...
            co::ICommand& command = const_cast< co::ICommand& >( *i );

            if( !command( ))
            {
                LBABORT( "Error handling " << command );
            }
            if( !_running )
                break;
        }
    }
    _mainThreadQueue.flush();
}

bool Server::_cmdChooseConfig( co::ICommand& command )
{
    const uint32_t requestID = command.read< uint32_t >();
    const fabric::ConfigParams& params = command.read< fabric::ConfigParams >();

    LBVERB << "Handle choose config " << command << " req " << requestID
           << " renderer " << params.getWorkDir() << '/'
           << params.getRenderClient() << std::endl;

    Config* config = 0;
    const Configs& configs = getConfigs();
    for( ConfigsCIter i = configs.begin(); i != configs.end() && !config; ++i )
    {
        Config* candidate = *i;
        const float version = candidate->getFAttribute( Config::FATTR_VERSION );
        LBASSERT( version == 1.2f );
        if( !candidate->isUsed() && version == 1.2f )
            config = candidate;
    }

#ifdef EQUALIZER_USE_HWSD
    if( !config )
    {
        const std::string& configFile = command.read< std::string >();
        config = config::Server::configure( this, configFile, params );
        if( config )
        {
            config->register_();
            LBDEBUG << lunchbox::disableFlush << lunchbox::disableHeader
                   << "Configured:" << std::endl
                   << lunchbox::indent << Global::instance() << *this
                   << lunchbox::exdent << lunchbox::enableHeader
                   << lunchbox::enableFlush << std::endl;
        }
    }
#endif

    co::NodePtr node = command.getRemoteNode();

    if( !config )
    {
        node->send( fabric::CMD_SERVER_CHOOSE_CONFIG_REPLY )
            << uint128_t() << requestID;
        return true;
    }

    ConfigBackupVisitor backup;
    config->accept( backup );
    config->setApplicationNetNode( node );
    config->setWorkDir( params.getWorkDir( ));
    config->setRenderClient( params.getRenderClient( ));
    config->setRenderClientArgs( params.getRenderClientArgs( ));
    config->setRenderClientEnvPrefixes( params.getRenderClientEnvPrefixes( ));
    config->commit();

    node->send( fabric::CMD_SERVER_CREATE_CONFIG )
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

    node->send( fabric::CMD_SERVER_CHOOSE_CONFIG_REPLY )
            << config->getID() << requestID << co::serialize( descs );
    return true;
}

bool Server::_cmdReleaseConfig( co::ICommand& command )
{
    const uint128_t& configID = command.read< uint128_t >();
    const uint32_t requestID = command.read< uint32_t >();

    LBVERB << "Handle release config " << command << " config " << configID
           << std::endl;

    co::NodePtr node = command.getRemoteNode();

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
        node->send( fabric::CMD_SERVER_RELEASE_CONFIG_REPLY ) << requestID;
        return true;
    }

    if( config->isRunning( ))
    {
        LBWARN << "Release of running configuration" << std::endl;
        config->exit(); // Make sure config is exited
    }

    lunchbox::Request< void > request = registerRequest< void >();
    node->send( fabric::CMD_SERVER_DESTROY_CONFIG )
        << config->getID() << request;
    request.wait();

#ifdef EQUALIZER_USE_HWSD
    if( config->isAutoConfig( ))
    {
        LBASSERT( _admins.empty( ));
        config->deregister();
        config::Server::release( config );
    }
    else
#endif
    {
        ConfigRestoreVisitor restore;
        config->accept( restore );
        config->commit();
    }

    node->send( fabric::CMD_SERVER_RELEASE_CONFIG_REPLY ) << requestID;
    LBVERB << "Released config " << configID << std::endl;
    return true;
}

bool Server::_cmdDestroyConfigReply( co::ICommand& command )
{
    serveRequest( command.read< uint32_t >( ));
    return true;
}

bool Server::_cmdShutdown( co::ICommand& command )
{
    const uint32_t requestID = command.read< uint32_t >();

    co::NodePtr node = command.getRemoteNode();

    if( !_admins.empty( ))
    {
        LBWARN << "Ignoring shutdown request, " << _admins.size()
               << " admin clients connected" << std::endl;

        node->send( fabric::CMD_SERVER_SHUTDOWN_REPLY ) << requestID << false;
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

            node->send( fabric::CMD_SERVER_SHUTDOWN_REPLY )
                    << requestID << false;
            return true;
        }
    }

    LBDEBUG << "Shutting down server" << std::endl;

    _running = false;
    node->send( fabric::CMD_SERVER_SHUTDOWN_REPLY ) << requestID << true;

#ifndef WIN32
    // WAR for 2874188: Lockup at shutdown
    lunchbox::sleep( 100 );
#endif

    return true;
}

bool Server::_cmdMap( co::ICommand& command )
{
    co::NodePtr node = command.getRemoteNode();
    _admins.push_back( node );

    const Configs& configs = getConfigs();
    for( Configs::const_iterator i = configs.begin(); i != configs.end(); ++i )
    {
        Config* config = *i;
        node->send( fabric::CMD_SERVER_CREATE_CONFIG )
                << co::ObjectVersion( config ) << LB_UNDEFINED_UINT32;
    }

    node->send( fabric::CMD_SERVER_MAP_REPLY ) << command.read< uint32_t >();
    return true;
}

bool Server::_cmdUnmap( co::ICommand& command )
{
    co::NodePtr node = command.getRemoteNode();
    co::Nodes::iterator i = lunchbox::find( _admins, node );

    LBASSERT( i != _admins.end( ));
    if( i != _admins.end( ))
    {
        _admins.erase( i );
        const Configs& configs = getConfigs();
        for( Configs::const_iterator j = configs.begin();
             j != configs.end(); ++j )
        {
            Config* config = *j;
            node->send( fabric::CMD_SERVER_DESTROY_CONFIG )
                    << config->getID() << LB_UNDEFINED_UINT32;
        }
    }

    node->send( fabric::CMD_SERVER_UNMAP_REPLY ) << command.read< uint32_t >();
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
