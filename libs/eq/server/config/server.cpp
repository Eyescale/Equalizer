
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

#include <eq/client/configParams.h>

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

	std::istringstream iss(session);
	std::string token;

	uint32_t rtNeuronFlag = 0;
	while( getline(iss, token, '-') )
	{
		if( token == "rtneuron" )
		{
			rtNeuronFlag = ConfigParams::FLAG_MULTIPROCESS;
			break;
		}
	}

    if( !Resources::discover( config, session, flags | rtNeuronFlag ))
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

    Display::discoverLocal( config );
    const Compounds compounds = Loader::addOutputCompounds( server );
    if( compounds.empty( ))
    {
        delete config;
        return 0;
    }

    const Channels channels = Resources::configureSourceChannels( config );
    Resources::configure( compounds, channels );

    configureForBenchmark( config, session, flags | rtNeuronFlag );

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

static void _setAffinity( const Config* config, const int32_t cpuMap[3], const uint32_t flags )
{
    //TODO: Change the code so that the affinitiy is set not considering
    // the nodes are listed in order

    const Nodes& nodes = config->getNodes();
    size_t nodeCount = 0;
    for( NodesCIter i = nodes.begin(); i != nodes.end(); ++i )
    {
        const Pipes& pipes = (*i)->getPipes();

        if( !(*i)->isApplicationNode( ) && ( flags & ConfigParams::FLAG_MULTIPROCESS ) )
        {
            pipes[0]->setIAttribute( Pipe::IATTR_HINT_AFFINITY, cpuMap[ (nodeCount++) % 3 ] );
        }
        else
        {
            if( pipes.size() != 3 )
                continue;

            pipes[0]->setIAttribute( Pipe::IATTR_HINT_AFFINITY, cpuMap[0] );
            pipes[1]->setIAttribute( Pipe::IATTR_HINT_AFFINITY, cpuMap[1] );
            pipes[2]->setIAttribute( Pipe::IATTR_HINT_AFFINITY, cpuMap[2] );
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

void Server::configureForBenchmark( Config* config, const std::string& session_,
                                    const uint32_t flags )
{
    std::string session = session_;
    std::string token;
    std::istringstream iss(session);

    while( getline( iss, token, '-' ))
    {
    	if( token == "GoodAffinity" )
		{
			// int32_t affinityCPUs[3] = { fabric::CPU + 0, fabric::CPU + 0,
			//                             fabric::CPU + 1 };
			int32_t affinityCPUs[3] = { fabric::CORE + 1, fabric::CORE + 2,
													fabric::CORE + 7 };

			_setAffinity( config, affinityCPUs, flags );
		}
		else if( token == "BadAffinity" )
		{
			// int32_t affinityCPUs[3] = { fabric::CPU + 1, fabric::CPU + 1,
			//                            fabric::CPU + 0 };
			int32_t affinityCPUs[3] = { fabric::CORE + 7, fabric::CORE + 8,
														  fabric::CORE + 2 };
			_setAffinity( config, affinityCPUs, flags );
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
