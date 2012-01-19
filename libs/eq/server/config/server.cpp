
/* Copyright (c) 2011-2012, Stefan Eilemann <eile@eyescale.h>
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

#include "display.h"
#include "resources.h"

#include "../config.h"
#include "../global.h"
#include "../loader.h"
#include "../node.h"
#include "../server.h"

#include <fstream>

namespace eq
{
namespace server
{
namespace config
{

ServerPtr Server::configure( const std::string& session )
{
    Global::instance()->setConfigFAttribute( Config::FATTR_VERSION, 1.2f );
    ServerPtr server = new server::Server;

    Config* config = new Config( server );
    config->setName( session + " autoconfig" );

    if( !Resources::discover( config, session ))
        return 0;

    if( config->getNodes().size() > 1 )
        // add server connection for cluster configs
        server->addConnectionDescription( new ConnectionDescription );

    Display::discoverLocal( config );
    const Compounds compounds = Loader::addOutputCompounds( server );
    if( compounds.empty( ))
        return 0;

    const Channels channels = Resources::configureSourceChannels( config );
    Resources::configure( compounds, channels );

    configureForBenchmark( config, session );

    std::ofstream configFile;
    const std::string filename = session + ".auto.eqc";
    configFile.open( filename.c_str( ));
    configFile << co::base::indent << Global::instance() << *server
               << co::base::exdent << std::endl;
    configFile.close();

    return server;
}

static void _setAffinity( const Config* config, const int32_t cpuMap[3] )
{
    const Nodes& nodes = config->getNodes();
    for( NodesCIter i = nodes.begin(); i != nodes.end(); ++i )
    {
        const Pipes& pipes = (*i)->getPipes();

        if( pipes.size() != 3 )
            continue;

        pipes[0]->setIAttribute( Pipe::IATTR_HINT_AFFINITY, cpuMap[0] );
        pipes[1]->setIAttribute( Pipe::IATTR_HINT_AFFINITY, cpuMap[1] );
        pipes[2]->setIAttribute( Pipe::IATTR_HINT_AFFINITY, cpuMap[2] );
    }
}

static void _setNetwork( const Config* config, const co::ConnectionType type,
                         const std::string& hostPostfix )
{
    const Nodes& nodes = config->getNodes();
    for( NodesCIter i = nodes.begin(); i != nodes.end(); ++i )
    {
        const Node* node = *i;
        const co::ConnectionDescriptions& descriptions =
            node->getConnectionDescriptions();

        co::ConnectionDescriptionPtr desc;

        if( descriptions.empty() )
        {
            desc = new co::ConnectionDescription();
            (*i)->addConnectionDescription( desc );
        }
        else
            desc = descriptions.front();

        desc->type = type;

        const std::string& hostname = node->getHost();        
        if( hostname.empty( )) // appNode!?
        {
            EQASSERT( node->isApplicationNode( ));
            desc->setHostname( std::string( "node01" ) + hostPostfix );
        }
        else
        {
            const std::string host = hostname.substr( 0, hostname.find( '.' ));
            desc->setHostname( host + hostPostfix );
        }
    }
}

void Server::configureForBenchmark( Config* config, const std::string& session_ )
{
    std::string session = session_;

    std::string token;

    std::istringstream iss(session);

    while ( getline(iss, token, '-') )
    {
        if( token == "GoodAffinity" )
        {
            int32_t affinityCPUs[3] = { fabric::CPU + 0, fabric::CPU + 0, 
                                        fabric::CPU + 1 };
            _setAffinity( config, affinityCPUs );
        }
        else if( token == "BadAffinity" )
        {
            int32_t affinityCPUs[3] = { fabric::CPU + 1, fabric::CPU + 1, 
                                        fabric::CPU + 0 };
            _setAffinity( config, affinityCPUs );
        }
        else if( token == "TenGig" )
            _setNetwork( config, co::CONNECTIONTYPE_TCPIP, "t.cluster" );
        else if( token == "IPoIB" )
            _setNetwork( config, co::CONNECTIONTYPE_TCPIP, "i.cluster" );
        else if( token == "SDP" )
            _setNetwork( config, co::CONNECTIONTYPE_SDP, "i.cluster" );
        else if( token == "RDMA" )
            _setNetwork( config, co::CONNECTIONTYPE_RDMA, "i.cluster" );
    }
}

}
}
}
