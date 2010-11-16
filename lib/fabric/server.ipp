
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

#include "configPackets.h"
#include "log.h"
#include "serverPackets.h"

#include <eq/net/command.h>
#include <eq/net/connectionDescription.h>

namespace eq
{
namespace fabric
{

#define CmdFunc net::CommandFunc< Server< CL, S, CFG, NF, N > >

template< class CL, class S, class CFG, class NF, class N >
Server< CL, S, CFG, NF, N >::Server( NF* nodeFactory )
        : _nodeFactory( nodeFactory )
{
    EQASSERT( nodeFactory );
    EQLOG( LOG_INIT ) << "New " << base::className( this ) << std::endl;
}

template< class CL, class S, class CFG, class NF, class N >
Server< CL, S, CFG, NF, N >::~Server()
{
    EQLOG( LOG_INIT ) << "Delete " << base::className( this ) << std::endl;
    _client = 0;
    EQASSERT( _configs.empty( ));
}

template< class CL, class S, class CFG, class NF, class N >
void Server< CL, S, CFG, NF, N >::setClient( ClientPtr client )
{
    _client = client;
    if( !client )
        return;

    net::CommandQueue* queue = static_cast< S* >( this )->getMainThreadQueue();
    registerCommand( CMD_SERVER_CREATE_CONFIG, 
                     CmdFunc( this, &Server::_cmdCreateConfig ), queue );
    registerCommand( CMD_SERVER_DESTROY_CONFIG, 
                     CmdFunc( this, &Server::_cmdDestroyConfig ), queue );
}

template< class CL, class S, class CFG, class NF, class N >
void Server< CL, S, CFG, NF, N >::_addConfig( CFG* config )
{ 
    EQASSERT( config->getServer() == static_cast< S* >( this ));
    EQASSERT( stde::find( _configs, config ) == _configs.end( ));
    _configs.push_back( config );
}

template< class CL, class S, class CFG, class NF, class N >
bool Server< CL, S, CFG, NF, N >::_removeConfig( CFG* config )
{
    typename Configs::iterator i = stde::find( _configs, config );
    if( i == _configs.end( ))
        return false;

    _configs.erase( i );
    return true;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
template< class CL, class S, class CFG, class NF, class N > bool
Server< CL, S, CFG, NF, N >::_cmdCreateConfig( net::Command& command )
{
    const ServerCreateConfigPacket* packet = 
        command.getPacket<ServerCreateConfigPacket>();
    EQVERB << "Handle create config " << packet << std::endl;
    EQASSERT( packet->proxy.identifier <= EQ_ID_MAX );
    
    CFG* config = _nodeFactory->createConfig( static_cast< S* >( this ));
    net::LocalNodePtr localNode = command.getLocalNode();
    localNode->mapSession( command.getNode(), config, packet->configID );
    config->map( packet->proxy );

    if( packet->requestID <= EQ_ID_MAX )
    {
        ConfigCreateReplyPacket reply( packet );
        command.getNode()->send( reply );
    }

    return true;
}

template< class CL, class S, class CFG, class NF, class N > bool
Server< CL, S, CFG, NF, N >::_cmdDestroyConfig( net::Command& command )
{
    const ServerDestroyConfigPacket* packet = 
        command.getPacket<ServerDestroyConfigPacket>();
    EQVERB << "Handle destroy config " << packet << std::endl;
    
    net::LocalNodePtr  localNode  = command.getLocalNode();
    net::Session* session    = localNode->getSession( packet->configID );

    CFG* config = EQSAFECAST( CFG*, session );
    static_cast< typename CFG::Super* >( config )->unmap();
    EQCHECK( localNode->unmapSession( config ));
    _nodeFactory->releaseConfig( config );

    if( packet->requestID <= EQ_ID_MAX )
    {
        ServerDestroyConfigReplyPacket reply( packet );
        command.getNode()->send( reply );
    }
    return true;
}

template< class CL, class S, class CFG, class NF, class N >
std::ostream& operator << ( std::ostream& os, 
                            const Server< CL, S, CFG, NF, N >& server )
{
    os << base::disableFlush << base::disableHeader << "server " << std::endl;
    os << "{" << std::endl << base::indent;
    
    const net::ConnectionDescriptions& cds =
        server.getConnectionDescriptions();
    for( net::ConnectionDescriptions::const_iterator i = cds.begin();
         i != cds.end(); ++i )
    {
        net::ConnectionDescriptionPtr desc = *i;
        os << *desc;
    }

    const std::vector< CFG* >& configs = server.getConfigs();
    for( typename std::vector< CFG* >::const_iterator i = configs.begin();
         i != configs.end(); ++i )
    {
        const CFG* config = *i;
        os << *config;
    }

    os << base::exdent << "}"  << base::enableHeader << base::enableFlush
       << std::endl;

    return os;
}

}
}
