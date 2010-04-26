
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

#include "server.h"

namespace eq
{
namespace fabric
{

#define CmdFunc net::CommandFunc< Server< CL, S, CFG > >

template< class CL, class S, class CFG >
Server< CL, S, CFG >::Server()
{
}

template< class CL, class S, class CFG >
Server< CL, S, CFG >::~Server()
{
    _client = 0;
    EQASSERT( _configs.empty( ));
}

template< class CL, class S, class CFG >
void Server< CL, S, CFG >::setClient( ClientPtr client, net::CommandQueue* queue )
{
    _client = client;
    if( !client )
        return;

    registerCommand( fabric::CMD_SERVER_CREATE_CONFIG, 
                     CmdFunc( this, &Server::_cmdCreateConfig ), queue );
    registerCommand( fabric::CMD_SERVER_DESTROY_CONFIG, 
                     CmdFunc( this, &Server::_cmdDestroyConfig ), queue );
}

template< class CL, class S, class CFG >
void Server< CL, S, CFG >::_addConfig( CFG* config )
{ 
    EQASSERT( config->getServer() == static_cast< S* >( this ));
    _configs.push_back( config );
}

template< class CL, class S, class CFG >
bool Server< CL, S, CFG >::_removeConfig( CFG* config )
{
    typename ConfigVector::iterator i = stde::find( _configs, config );
    if( i == _configs.end( ))
        return false;

    _configs.erase( i );
    return true;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
template< class CL, class S, class CFG > net::CommandResult
Server< CL, S, CFG >::_cmdCreateConfig( net::Command& command )
{
    const ServerCreateConfigPacket* packet = 
        command.getPacket<ServerCreateConfigPacket>();
    EQVERB << "Handle create config " << packet << std::endl;
    EQASSERT( packet->proxy.identifier != EQ_ID_INVALID );
    
    CFG* config = _createConfig();
    net::NodePtr localNode = command.getLocalNode();
    localNode->mapSession( command.getNode(), config, packet->configID );
    config->map( packet->proxy );

    if( packet->requestID != EQ_ID_INVALID )
    {
        ConfigCreateReplyPacket reply( packet );
        command.getNode()->send( reply );
    }

    return net::COMMAND_HANDLED;
}

template< class CL, class S, class CFG > net::CommandResult
Server< CL, S, CFG >::_cmdDestroyConfig( net::Command& command )
{
    const ServerDestroyConfigPacket* packet = 
        command.getPacket<ServerDestroyConfigPacket>();
    EQVERB << "Handle destroy config " << packet << std::endl;
    
    net::NodePtr  localNode  = command.getLocalNode();
    net::Session* session    = localNode->getSession( packet->configID );

    CFG* config = EQSAFECAST( CFG*, session );
    config->unmap();
    EQCHECK( localNode->unmapSession( config ));
    _releaseConfig( config );

    if( packet->requestID != EQ_ID_INVALID )
    {
        ServerDestroyConfigReplyPacket reply( packet );
        command.getNode()->send( reply );
    }
    return net::COMMAND_HANDLED;
}

}
}
