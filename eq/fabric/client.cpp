
/* Copyright (c) 2010-2015, Stefan Eilemann <eile@eyescale.ch>
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

#include "client.h"

#include "global.h"
#include "nodeType.h"

#include <eq/fabric/commands.h>

#include <co/iCommand.h>
#include <co/commandQueue.h>
#include <co/connection.h>
#include <co/connectionDescription.h>
#include <lunchbox/dso.h>

namespace eq
{
namespace fabric
{
namespace detail
{
class Client {};
}

Client::Client()
    : _impl( 0 )
{
}

Client::~Client()
{
    delete _impl;
    LBASSERT( isClosed( ));
}

bool Client::connectServer( co::NodePtr server )
{
    if( server->isConnected( ))
        return true;

    if( !server->getConnectionDescriptions().empty( ))
        return connect( server );

    co::ConnectionDescriptionPtr connDesc( new co::ConnectionDescription );
    connDesc->port = EQ_DEFAULT_PORT;
    const std::string& globalServer = Global::getServer();
    const char* envServer = getenv( "EQ_SERVER" );
    std::string address = !globalServer.empty() ? globalServer :
                           envServer            ? envServer : "localhost";
    if( !connDesc->fromString( address ))
    {
        LBWARN << "Can't parse server address " << address << std::endl;
        return false;
    }
    LBDEBUG << "Connecting server " << connDesc->toString() << std::endl;
    server->addConnectionDescription( connDesc );

    if( connect( server ))
        return true;

    server->removeConnectionDescription( connDesc );
    return false;
}

bool Client::disconnectServer( co::NodePtr server )
{
    if( !server->isConnected( ))
    {
        LBWARN << "Trying to disconnect unconnected server" << std::endl;
        return false;
    }

    if( disconnect( server ))
        return true;

    LBWARN << "Server disconnect failed" << std::endl;
    return false;
}

void Client::processCommand( const uint32_t timeout )
{
    co::CommandQueue* queue = getMainThreadQueue();
    LBASSERT( queue );
    co::ICommand command = queue->pop( timeout );
    if( !command.isValid( )) // wakeup() or timeout delivers invalid command
        return;

    LBCHECK( command( ));
}

bool Client::dispatchCommand( co::ICommand& command )
{
    LBVERB << "dispatch " << command << std::endl;

    if( command.getCommand() >= co::CMD_NODE_CUSTOM &&
        command.getCommand() < CMD_SERVER_CUSTOM )
    {
        co::NodePtr node = command.getRemoteNode();
        return node->co::Dispatcher::dispatchCommand( command );
    }

    return co::LocalNode::dispatchCommand( command );
}

}
}
