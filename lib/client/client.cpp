
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "client.h"

#include "commandQueue.h"
#include "global.h"
#include "init.h"
#include "nodeFactory.h"
#include "packets.h"
#include "server.h"

#include <eq/fabric/nodeType.h>
#include <eq/net/command.h>
#include <eq/net/connection.h>
#include <eq/net/connectionDescription.h>
#include <eq/net/global.h>
#include <eq/base/dso.h>

namespace eq
{
/** @cond IGNORE */
typedef net::CommandFunc<Client> ClientFunc;

static net::ConnectionPtr _startLocalServer();
static void _joinLocalServer();

typedef net::ConnectionPtr (*eqsStartLocalServer_t)( const std::string& file );
typedef void (*eqsJoinLocalServer_t)();

typedef fabric::Client Super;
/** @endcond */

Client::Client()
        : Super()
        , _running( false )
{
    registerCommand( fabric::CMD_CLIENT_EXIT, 
                     ClientFunc( this, &Client::_cmdExit ), &_mainThreadQueue );

    EQINFO << "New client at " << (void*)this << std::endl;
}

Client::~Client()
{
    EQINFO << "Delete client at " << (void*)this << std::endl;
    close();
}

bool Client::connectServer( ServerPtr server )
{
    if( Super::connectServer( server.get( )))
    {
        server->setClient( this );
        return true;
    }

    if( !server->getConnectionDescriptions().empty() ||
        !Global::getServer().empty() || getenv( "EQ_SERVER" ))
    {
        return false;
    }

    // Use app-local server if no explicit server was set
    net::ConnectionPtr connection = _startLocalServer();
    if( !connection )
        return false;

    if( !_connect( server.get(), connection ))
        // giving up
        return false;

    server->setClient( this );
    server->_localServer = true;
    return true;
}

/** @cond IGNORE */
namespace
{
base::DSO _libeqserver;
}

net::ConnectionPtr _startLocalServer()
{
    if( !_libeqserver.open(
#ifdef _MSC_VER
         "EqualizerServer.dll"
#elif defined (WIN32)
        "libeqserver.dll"
#elif defined (Darwin)
        "libeqserver.dylib"
#else
        "libeqserver.so"
#endif
                           ))
    {
        EQWARN << "Can't open Equalizer server library" << std::endl;
        return 0;
    }

    eqsStartLocalServer_t eqsStartLocalServer = (eqsStartLocalServer_t)
        _libeqserver.getFunctionPointer( "eqsStartLocalServer" );

    if( !eqsStartLocalServer )
    {
        EQWARN << "Can't find server entry function eqsStartLocalServer"
               << std::endl;
        return 0;
    }

    return eqsStartLocalServer( Global::getConfigFile( ));
}

static void _joinLocalServer()
{
    eqsJoinLocalServer_t eqsJoinLocalServer = (eqsJoinLocalServer_t)
        _libeqserver.getFunctionPointer( "eqsJoinLocalServer" );

    if( !eqsJoinLocalServer )
    {
        EQWARN << "Can't find server entry function eqsJoinLocalServer"
               << std::endl;
        return;
    }

    eqsJoinLocalServer();
    _libeqserver.close();
}
/** @endcond */

bool Client::disconnectServer( ServerPtr server )
{
    bool success = true;

    // shut down process-local server (see _startLocalServer)
    if( server->_localServer )
    {
        EQASSERT( server->isConnected( ))
        server->shutdown();
        _joinLocalServer();
        server->_localServer = false;
        server->setClient( 0 );
        EQASSERT( !server->isConnected( ))
    }
    else
    {
        server->setClient( 0 );
        success = Super::disconnectServer( server.get( ));
    }

    _mainThreadQueue.flush();
    return success;
}

bool Client::listen()
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

bool Client::clientLoop()
{
    EQINFO << "Entered client loop" << std::endl;

    _running = true;
    while( _running )
        processCommand();

    // cleanup
    _mainThreadQueue.flush();
    EQASSERT( !hasSessions( ));

    return true;
}

bool Client::exitClient()
{
    return (net::Node::exitClient() & eq::exit( ));
}

bool Client::hasCommands()
{
    return !_mainThreadQueue.isEmpty();
}

net::NodePtr Client::createNode( const uint32_t type )
{ 
    switch( type )
    {
        case fabric::NODETYPE_EQ_SERVER:
        {
            Server* server = new Server;
            server->setClient( this );
            return server;
        }

        default:
            return net::Node::createNode( type );
    }
}

net::CommandResult Client::_cmdExit( net::Command& command )
{
    _running = false;
    // Close connection here, this is the last packet we'll get on it
    command.getLocalNode()->close( command.getNode( ));
    return net::COMMAND_HANDLED;
}
}

