
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
#include "commands.h"
#include "global.h"
#include "init.h"
#include "nodeFactory.h"
#include "packets.h"
#include "server.h"

#include <eq/net/command.h>
#include <eq/net/connection.h>
#include <eq/base/dso.h>

namespace eq
{
/** @cond IGNORE */
typedef net::CommandFunc<Client> ClientFunc;

static net::ConnectionPtr _startLocalServer();
static void _joinLocalServer();

typedef net::ConnectionPtr (*eqsStartLocalServer_t)( const std::string& file );
typedef void (*eqsJoinLocalServer_t)();

typedef fabric::Client< Server, Client > Super;
/** @endcond */

Client::Client()
        : Super()
        , _nodeThreadQueue( 0 )
        , _running( false )
{
    EQINFO << "New client at " << (void*)this << std::endl;
}

Client::~Client()
{
    EQINFO << "Delete client at " << (void*)this << std::endl;
    close();
}

bool Client::listen()
{
    EQASSERT( !_nodeThreadQueue );
    _nodeThreadQueue = new CommandQueue;

    registerCommand( CMD_CLIENT_EXIT, ClientFunc( this, &Client::_cmdExit ),
                     _nodeThreadQueue );

    return net::Node::listen();
}

bool Client::close()
{
    if( !net::Node::close( ))
        return false;

    delete _nodeThreadQueue;
    _nodeThreadQueue = 0;
    return true;
}

bool Client::connectServer( ServerPtr server )
{
    if( Super::connectServer( server ))
        return true;

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
    // shut down process-local server (see _startLocalServer)
    if( server->_localServer )
    {
        EQASSERT( server->isConnected( ))
        server->shutdown();
        _joinLocalServer();
        server->_localServer = false;
    }

    const int success = Super::disconnectServer( server );
    _nodeThreadQueue->flush();
    return success;
}

bool Client::clientLoop()
{
    EQINFO << "Entered client loop" << std::endl;

    _running = true;
    while( _running )
        processCommand();

    // cleanup
    _nodeThreadQueue->flush();
    EQASSERT( !hasSessions( ));

    return true;
}

bool Client::exitClient()
{
    return (net::Node::exitClient() & eq::exit( ));
}

bool Client::hasCommands()
{
    return !_nodeThreadQueue->isEmpty();
}

void Client::processCommand()
{
    net::Command* command = _nodeThreadQueue->pop();
    if( !command ) // just a wakeup()
        return;

    switch( invokeCommand( *command ))
    {
        case net::COMMAND_HANDLED:
        case net::COMMAND_DISCARD:
            break;
            
        case net::COMMAND_ERROR:
            EQABORT( "Error handling command packet" );
            break;
    }
    command->release();
}

net::CommandResult Client::_cmdExit( net::Command& command )
{
    _running = false;
    // Close connection here, this is the last packet we'll get on it
    command.getLocalNode()->close( command.getNode( ));
    return net::COMMAND_HANDLED;
}
}

#include "../fabric/client.ipp"
template class eq::fabric::Client< eq::Server, eq::Client >;

