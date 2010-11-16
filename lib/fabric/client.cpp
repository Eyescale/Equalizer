
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch>
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
#include "packetType.h"

#include <eq/net/command.h>
#include <eq/net/commandQueue.h>
#include <eq/net/connection.h>
#include <eq/net/connectionDescription.h>
#include <eq/base/dso.h>

namespace eq
{
namespace fabric
{

Client::Client()
{
}

Client::~Client()
{
    EQASSERT( isClosed( ));
}

bool Client::connectServer( net::NodePtr server )
{
    if( server->isConnected( ))
        return true;

    net::ConnectionDescriptionPtr connDesc;
    if( server->getConnectionDescriptions().empty( ))
    {
        connDesc = new net::ConnectionDescription;
        connDesc->port = EQ_DEFAULT_PORT;
    
        const std::string globalServer = Global::getServer();
        const char* envServer = getenv( "EQ_SERVER" );
        std::string address = !globalServer.empty() ? globalServer :
                               envServer            ? envServer : "localhost";

        if( !connDesc->fromString( address ))
            EQWARN << "Can't parse server address " << address << std::endl;
        EQASSERT( address.empty( ));
        EQINFO << "Connecting to " << connDesc->toString() << std::endl;

        server->addConnectionDescription( connDesc );
    }

    if( connect( server ))
        return true;
    
    if( connDesc.isValid( )) // clean up
        server->removeConnectionDescription( connDesc );

    return false;
}

bool Client::disconnectServer( net::NodePtr server )
{
    if( !server->isConnected( ))
    {
        EQWARN << "Trying to disconnect unconnected server" << std::endl;
        return false;
    }

    if( net::LocalNode::disconnect( server ))
        return true;

    EQWARN << "Server disconnect failed" << std::endl;
    return false;
}

void Client::processCommand()
{
    net::CommandQueue* queue = getMainThreadQueue();
    EQASSERT( queue );

    net::Command* command = queue->pop();
    if( !command ) // just a wakeup()
        return;

    if( !invokeCommand( *command ))
    {
        EQABORT( "Error handling command packet" );
    }
    command->release();
}

bool Client::dispatchCommand( net::Command& command )
{
    EQVERB << "dispatchCommand " << command << std::endl;

    switch( command->type )
    {
        case PACKETTYPE_EQ_CLIENT:
            return net::Dispatcher::dispatchCommand( command );

        case PACKETTYPE_EQ_SERVER:
        {
            net::NodePtr node = command.getNode();
            return node->net::Dispatcher::dispatchCommand( command );
        }

        default:
            return net::LocalNode::dispatchCommand( command );
    }
}

bool Client::invokeCommand( net::Command& command )
{
    EQVERB << "invokeCommand " << command << std::endl;

    switch( command->type )
    {
        case PACKETTYPE_EQ_CLIENT:
            return net::Dispatcher::invokeCommand( command );

        case PACKETTYPE_EQ_SERVER:
        {
            net::NodePtr node = command.getNode();
            return node->net::Dispatcher::invokeCommand( command );
        }

        default:
            return net::LocalNode::invokeCommand( command );
    }
}

}
}
