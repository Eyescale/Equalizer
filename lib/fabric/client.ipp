
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

#include <eq/net/command.h>
#include <eq/net/connection.h>
#include <eq/base/dso.h>

namespace eq
{
namespace fabric
{

template< typename S, typename C >
Client< S, C >::Client()
{
}

template< typename S, typename C >
Client< S, C >::~Client()
{
    close();
}

template< typename S, typename C >
bool Client< S, C >::connectServer( ServerPtr server )
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

    if( connect( net::NodePtr( server.get( )) ))
    {
        server->setClient( static_cast< C* >( this ));
        return true;
    }
    
    if( connDesc.isValid( )) // clean up
        server->removeConnectionDescription( connDesc );

    return false;
}

template< typename S, typename C >
bool Client< S, C >::disconnectServer( ServerPtr server )
{
    if( !server->isConnected( ))
    {
        EQWARN << "Trying to disconnect unconnected server" << std::endl;
        return false;
    }

    server->setClient( 0 );

    if( net::Node::close( server.get( )))
        return true;

    EQWARN << "Server disconnect failed" << std::endl;
    return false;
}


template< typename S, typename C >
net::NodePtr Client< S, C >::createNode( const uint32_t type )
{ 
    switch( type )
    {
        case TYPE_EQ_SERVER:
        {
            S* server = new S;
            server->setClient( static_cast< C* >( this ));
            return server;
        }

        default:
            return net::Node::createNode( type );
    }
}

template< typename S, typename C >
bool Client< S, C >::dispatchCommand( net::Command& command )
{
    EQVERB << "dispatchCommand " << command << std::endl;

    switch( command->datatype )
    {
        case DATATYPE_EQ_CLIENT:
            return net::Dispatcher::dispatchCommand( command );

        case DATATYPE_EQ_SERVER:
        {
            net::NodePtr node = command.getNode();

            EQASSERT( dynamic_cast< S* >( node.get( )) );
            ServerPtr server = static_cast< S* >( node.get( ));

            return server->net::Dispatcher::dispatchCommand( command );
        }

        default:
            return net::Node::dispatchCommand( command );
    }
}

template< typename S, typename C >
net::CommandResult Client< S, C >::invokeCommand( net::Command& command )
{
    EQVERB << "invokeCommand " << command << std::endl;

    switch( command->datatype )
    {
        case DATATYPE_EQ_CLIENT:
            return net::Dispatcher::invokeCommand( command );

        case DATATYPE_EQ_SERVER:
        {
            net::NodePtr node = command.getNode();

            EQASSERT( dynamic_cast<S*>( node.get( )) );
            ServerPtr server = static_cast<S*>( node.get( ));

            return server->net::Dispatcher::invokeCommand( command );
        }
        default:
            return net::Node::invokeCommand( command );
    }
}

}
}
