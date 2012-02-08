
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

Config* Server::configure( ServerPtr server, const std::string& session,
                           const uint32_t flags )
{
    if( !server->getConfigs().empty( )) // don't do more than one auto config
        return 0;

    Global::instance()->setConfigFAttribute( Config::FATTR_VERSION, 1.2f );

    Config* config = new Config( server );
    config->setName( session + " autoconfig" );

    if( !Resources::discover( config, session, flags ))
    {
        delete config;
        return 0;
    }

    if( config->getNodes().size() > 1 ) // add server connection for clusters
    {
        co::ConnectionDescriptionPtr desc = new co::ConnectionDescription;
        desc->port = EQ_DEFAULT_PORT;
        server->addListener( desc );
    }

    Display::discoverLocal( config, flags );
    const Compounds compounds = Loader::addOutputCompounds( server );
    if( compounds.empty( ))
    {
        delete config;
        return 0;
    }

    const Channels channels = Resources::configureSourceChannels( config );
    Resources::configure( compounds, channels, flags );

    configureForBenchmark( config, session );

    std::ofstream configFile;
    const std::string filename = session + ".auto.eqc";
    configFile.open( filename.c_str( ));
    if( configFile.is_open( ))
    {
        std::ostream& previous = co::base::Log::getOutput();

        co::base::Log::setOutput( configFile );
        co::base::Log::instance( __FILE__, __LINE__ )
            << co::base::disableHeader << Global::instance() << *server
            << std::endl << co::base::enableHeader;
        co::base::Log::setOutput( previous );

        configFile.close();
    }
    return config;
}

static void _setAffinity( const Config* config, const int32_t cpuMap[3] )
{
    const Nodes& nodes = config->getNodes();
    for( NodesCIter i = nodes.begin(); i != nodes.end(); ++i )
    {
        const Pipes& pipes = (*i)->getPipes();
        for( PipesCIter j = pipes.begin(); j != pipes.end(); ++j )
        {
            Pipe* pipe = *j;
            const uint32_t device = pipe->getDevice();
            if( device < 3 )
                pipe->setIAttribute( Pipe::IATTR_HINT_AFFINITY, cpuMap[device] );
        }
    }
}

static void _setNetwork( const Config* config, const co::ConnectionType type,
                         const std::string& hostPostfix )
{
    const Nodes& nodes = config->getNodes();
    for( NodesCIter i = nodes.begin(); i != nodes.end(); ++i )
    {
        Node* node = *i;
        const co::ConnectionDescriptions& descriptions =
            node->getConnectionDescriptions();

        if( descriptions.empty() )
            node->addConnectionDescription( new co::ConnectionDescription );

        co::ConnectionDescriptionPtr desc = descriptions.front();
        desc->type = type;
        desc->bandwidth = 300000; // To disable compressor

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

    while( getline( iss, token, '-' ))
    {
        if( token == "GoodAffinity" )
        {
            int32_t affinityCPUs[3] = { fabric::SOCKET + 0, fabric::SOCKET + 0,
                                                    fabric::SOCKET + 1 };

            _setAffinity( config, affinityCPUs );
        }
        else if( token == "BadAffinity" )
        {
            int32_t affinityCPUs[3] = { fabric::SOCKET + 1, fabric::SOCKET + 1,
                                        fabric::SOCKET + 0 };
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
