
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com>
 * Copyright (c) 2010,      Cedric Stalder <cedric.stalder@gmail.com>
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
#include "serverVisitor.h"
#include "window.h"

#include <eq/admin/packets.h>
#include <eq/client/packets.h>
#include <eq/net/command.h>
#include <eq/net/connectionDescription.h>
#include <eq/net/global.h>
#include <eq/net/init.h>
#include <eq/net/node.h>
#include <eq/base/refPtr.h>
#include <eq/base/sleep.h>

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

typedef net::CommandFunc<Server> ServerFunc;
typedef fabric::Server< net::Node, Server, Config, NodeFactory > Super;

Server::Server()
        : Super( &_nf )
        , _running( false )
{
    base::Log::setClock( &_clock );

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
    EQASSERT( getConfigs().empty( )); // not possible - config RefPtr's myself
    deleteConfigs();
    base::Log::setClock( 0 );
}

namespace
{
template< class C, class V >
VisitorResult _accept( C* server, V& visitor )
{ 
    VisitorResult result = visitor.visitPre( server );
    if( result != TRAVERSE_CONTINUE )
        return result;

    const Configs& configs = server->getConfigs();
    for( Configs::const_iterator i = configs.begin(); i != configs.end(); ++i )
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

VisitorResult Server::accept( ServerVisitor& visitor ) const
{
    return _accept( this, visitor );
}

bool Server::listen()
{
    if( getConnectionDescriptions().empty( )) // add default listener
    {
        net::ConnectionDescriptionPtr connDesc = new net::ConnectionDescription;
        connDesc->type = net::CONNECTIONTYPE_TCPIP;
        connDesc->port = net::Global::getDefaultPort();
        addConnectionDescription( connDesc );
    }
    return Super::listen();
}

void Server::init()
{
    EQASSERT( isListening( ));
    base::Thread::setDebugName( typeid( *this ).name( ));

    const Configs& configs = getConfigs();
    if( configs.empty( ))
        EQWARN << "No configurations loaded" << std::endl;

    EQINFO << base::disableFlush << "Running server: " << std::endl
        << base::indent << Global::instance() << *this << base::exdent
        << base::enableFlush;

    for( Configs::const_iterator i = configs.begin(); i != configs.end(); ++i )
        registerConfig( *i );
}

void Server::exit()
{
    const Configs& configs = getConfigs();
    if( configs.empty( ))
        EQWARN << "No configurations loaded" << std::endl;

    for( Configs::const_iterator i = configs.begin(); i != configs.end(); ++i )
        deregisterConfig( *i );
}

void Server::run()
{
    init();
    handleCommands();
    exit();
}

void Server::_addConfig( Config* config )
{
    Super::_addConfig( config );

    if( config->getName().empty( ))
    {
        std::ostringstream stringStream;
        stringStream << "EQ_CONFIG_" << config->getID();
        config->setName( stringStream.str( ));
    }

    if( _running )
        registerConfig( config );
}

bool Server::_removeConfig( Config* config )
{
    const Configs& configs = getConfigs();
    if( stde::find( configs, config ) == configs.end( ))
        return false;

    if( _running )
        deregisterConfig( config );

    return Super::_removeConfig( config );
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

void Server::registerConfig( Config* config )
{
    registerSession( config );
    config->register_();
}

bool Server::deregisterConfig( Config* config )
{
    config->deregister();
    return deregisterSession( config );
}

//===========================================================================
// packet handling methods
//===========================================================================
bool Server::dispatchCommand( net::Command& command )
{
    switch( command->type )
    {
        case fabric::PACKETTYPE_EQ_SERVER:
            return net::Dispatcher::dispatchCommand( command );
            
        default:
            return net::Node::dispatchCommand( command );
    }
}

bool Server::invokeCommand( net::Command& command )
{
    switch( command->type )
    {
        case fabric::PACKETTYPE_EQ_SERVER:
            return net::Dispatcher::invokeCommand( command );
            
        default:
            return net::Node::invokeCommand( command );
    }
}

void Server::handleCommands()
{
    _running = true;
    while( _running ) // set to false in _cmdShutdown()
    {
        net::Command* command = _mainThreadQueue.pop();

        if( !invokeCommand( *command ))
        {
            EQABORT( "Error handling command " << command );
        }

        command->release();
    }
    _mainThreadQueue.flush();
}

bool Server::_cmdChooseConfig( net::Command& command ) 
{
    const ServerChooseConfigPacket* packet = 
        command.getPacket<ServerChooseConfigPacket>();
    EQINFO << "Handle choose config " << packet << std::endl;

    Config* config = 0;
    const Configs& configs = getConfigs();
    for( Configs::const_iterator i = configs.begin();
         i != configs.end() && !config; ++i )
    {
        Config* candidate = *i;
        const float version = candidate->getFAttribute(Config::FATTR_VERSION);
        EQASSERT( version == 1.1f );
        if( !candidate->isUsed() && version == 1.1f)
            config = candidate;
    }
    
    ServerChooseConfigReplyPacket reply( packet );
    net::NodePtr node = command.getNode();

    if( !config )
    {
        reply.configID = net::SessionID::ZERO;
        node->send( reply );
        return true;
    }

    ConfigBackupVisitor backup;
    config->accept( backup );
    config->setApplicationNetNode( node );

    const std::string  rendererInfo = packet->rendererInfo;
    const size_t       colonPos     = rendererInfo.find( '#' );
    const std::string  workDir      = rendererInfo.substr( 0, colonPos );
    const std::string  renderClient = rendererInfo.substr( colonPos + 1 );

    config->setWorkDir( workDir );
    config->setRenderClient( renderClient );

    fabric::ServerCreateConfigPacket createConfigPacket;
    createConfigPacket.configID = config->getID();
    createConfigPacket.proxy = config->getProxyVersion();
    createConfigPacket.proxy.version = config->commit();

    reply.configID = config->getID();

    node->send( createConfigPacket );
    node->send( reply );

    return true;
}

bool Server::_cmdReleaseConfig( net::Command& command )
{
    const ServerReleaseConfigPacket* packet = 
        command.getPacket<ServerReleaseConfigPacket>();
    EQINFO << "Handle release config " << packet << std::endl;

    ServerReleaseConfigReplyPacket reply( packet );
    net::NodePtr node = command.getNode();

    Config* config = 0;
    const Configs& configs = getConfigs();
    for( Configs::const_iterator i = configs.begin();
         i != configs.end() && !config; ++i )
    {
        Config* candidate = *i;
        if( candidate->getID() == packet->configID )
            config = candidate;
    }

    if( !config )
    {
        EQWARN << "Release request for unknown config" << std::endl;
        node->send( reply );
        return true;
    }

    if( config->isRunning( ))
    {
        EQWARN << "Release of running configuration" << std::endl;
        config->exit(); // Make sure config is exited
    }

    fabric::ServerDestroyConfigPacket destroyConfigPacket;
    destroyConfigPacket.requestID = registerRequest();
    destroyConfigPacket.configID  = config->getID();
    node->send( destroyConfigPacket );
    waitRequest( destroyConfigPacket.requestID );

    ConfigRestoreVisitor restore;
    config->accept( restore );
    EQINFO << "Released " << *config << std::endl;

    node->send( reply );
    EQLOG( base::LOG_ANY ) << "----- Released Config -----" << std::endl;
    return true;
}

bool Server::_cmdDestroyConfigReply( net::Command& command ) 
{
    const fabric::ServerDestroyConfigReplyPacket* packet = 
        command.getPacket< fabric::ServerDestroyConfigReplyPacket >();

    serveRequest( packet->requestID );
    return true;
}

bool Server::_cmdShutdown( net::Command& command )
{
    const ServerShutdownPacket* packet = 
        command.getPacket< ServerShutdownPacket >();

    ServerShutdownReplyPacket reply( packet );
    net::NodePtr node = command.getNode();

    if( !_admins.empty( ))
    {
        EQWARN << "Ignoring shutdown request, " << _admins.size()
               << " admin clients connected" << std::endl;

        node->send( reply );
        return true;
    }

    const Configs& configs = getConfigs();
    for( Configs::const_iterator i = configs.begin(); i != configs.end(); ++i )
    {
        Config* candidate = *i;
        if( candidate->isUsed( ))
        {
            EQWARN << "Ignoring shutdown request due to used config" 
                   << std::endl;

            node->send( reply );
            return true;
        }
    }

    EQINFO << "Shutting down server" << std::endl;

    _running = false;
    reply.result = true;
    node->send( reply );

#ifndef WIN32
    // WAR for 2874188: Lockup at shutdown
    base::sleep( 100 );
#endif

    return true;
}

bool Server::_cmdMap( net::Command& command )
{
    net::NodePtr node = command.getNode();
    _admins.push_back( node );

    const Configs& configs = getConfigs();
    for( Configs::const_iterator i = configs.begin(); i != configs.end(); ++i )
    {
        Config* config = *i;
        fabric::ServerCreateConfigPacket createConfigPacket;
        createConfigPacket.configID = config->getID();
        createConfigPacket.proxy = config->getProxyVersion();
        node->send( createConfigPacket );
    }

    const admin::ServerMapPacket* packet =
        command.getPacket< admin::ServerMapPacket >();
    admin::ServerMapReplyPacket reply( packet );
    node->send( reply );
    return true;
}

bool Server::_cmdUnmap( net::Command& command )
{
    net::NodePtr node = command.getNode();
    net::Nodes::iterator i = stde::find( _admins, node );

    EQASSERT( i != _admins.end( ));
    if( i != _admins.end( ))
    {
        _admins.erase( i );
        const Configs& configs = getConfigs();
        for( Configs::const_iterator j = configs.begin();
             j != configs.end(); ++j )
        {
            Config* config = *j;
            fabric::ServerDestroyConfigPacket destroyConfigPacket;
            destroyConfigPacket.configID  = config->getID();
            node->send( destroyConfigPacket );
        }
    }

    const admin::ServerUnmapPacket* packet = 
        command.getPacket< admin::ServerUnmapPacket >();
    admin::ServerUnmapReplyPacket reply( packet );
    node->send( reply );
    return true;
}

}
}
#include "../lib/fabric/server.ipp"
template class eq::fabric::Server< eq::net::Node, eq::server::Server,
                                   eq::server::Config,
                                   eq::server::NodeFactory >;

/** @cond IGNORE */
template std::ostream& eq::fabric::operator <<
( std::ostream&, const fabric::Server< eq::net::Node, eq::server::Server,
                                       eq::server::Config,
                                       eq::server::NodeFactory >& );
/** @endcond */
