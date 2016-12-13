
/* Copyright (c) 2016, Stefan.Eilemann@epfl.ch
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "Client.h"
#include "Server.h"
#include "hwsd.h"

#include <pression/data/Registry.h>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

using co::Strings;
using co::uint128_t;
using plydist::Options;

namespace fs = boost::filesystem;
namespace acc = boost::accumulators;
namespace po = boost::program_options;

namespace
{
const uint32_t N_COMMITS = 5;
const size_t TIMEOUT = 300000; // 5 minutes

std::string _addEnv( const std::string& env )
{
    const char* value = getenv( env.c_str( ));
    if( value )
        return env + "=" + value + " ";
    return std::string();
}

void _wait( plydist::Server& server, const size_t nNodes )
{
    server.doneNodes.timedWaitGE( nNodes, TIMEOUT );
    if( server.doneNodes.get() != nNodes )
        std::cout << "Error: Only " << server.doneNodes.get() << " of "
                  << nNodes << " nodes did run" << std::endl;
    server.doneNodes = 0;
}

void _benchmark( plydist::Server& server, const co::Nodes& nodes,
                 const std::string& launchCommand )
{
    co::LocalNodePtr localNode = server.getLocalNode();
    lunchbox::Clock clock;
    for( co::NodePtr node : nodes )
        localNode->launch( node, launchCommand );
    _wait( server, nodes.size( ));
    const float launchTime = clock.resetTimef();

    for( auto node : localNode->getNodes( false ))
        node->send( plydist::CMD_CLIENT_MAP );
    _wait( server, nodes.size( ));
    const float mapTime = clock.resetTimef();
    std::cout << "map " << server.getRSP() << std::endl;

    for( auto node : localNode->getNodes( false ))
        node->send( plydist::CMD_CLIENT_SYNC ) << N_COMMITS;
    for( size_t i = 0; i < N_COMMITS; ++i )
        server.commit();
    _wait( server, nodes.size( ));
    const float syncTime = clock.resetTimef() / float( N_COMMITS );

    std::cout << nodes.size() << ", "
              << launchTime  << ", " << mapTime  << ", " << syncTime
              << std::endl;
    std::cout << "sync " << server.getRSP() << std::endl;
}

void _benchmark( const int argc, char** argv, const std::string& model,
                 const Options opts, const co::Nodes& nodes,
                 const co::CompressorInfo& compressor )
{
    co::DataOStream::clearStatistics();

    // Run benchmark
    const fs::path program = fs::system_complete( fs::path( argv[ 0 ]));
    std::string launchCommand = std::string( "ssh -n %h " );
#ifndef WIN32
#  ifdef Darwin
    const char libPath[] = "DYLD_LIBRARY_PATH";
#  else
    const char libPath[] = "LD_LIBRARY_PATH";
#  endif
    launchCommand +=
        std::string( "env " ) + _addEnv( libPath ) + _addEnv( "PATH" ) +
        " LB_LOG_LEVEL=" + lunchbox::Log::getLogLevelString() + " ";
#endif
    launchCommand +=
        program.string() + " --client --lb-logfile " + lunchbox::getWorkDir() +
        "/" + lunchbox::getFilename( argv[0] ) + "_%n.log";

    for( int i = 1; i < argc; ++i )
        launchCommand += std::string( " '" ) + argv[i] + "'";

    for( size_t i = 1; i <= nodes.size(); ++i )
    {
        plydist::Server server( argc, argv, model, opts, compressor );
        _benchmark( server, co::Nodes( nodes.begin(), nodes.begin() + i ),
                    launchCommand + " --model-id " +
                    std::to_string( server.getModelID( )));
    }
    if( opts & Options::compression )
        std::cout << co::DataOStream::printStatistics << std::endl;
}
}

int main( const int argc, char** argv )
{
    for( int i = 1; i < argc; ++i )
    {
        if( argv[ i ] == std::string( "--client" ))
        {
            plydist::Client( argc, argv );
            return EXIT_SUCCESS;
        }
    }
    co::init( argc, argv );

    po::options_description options(
        "Benchmark tool for triply object distribution" );
    options.add_options()
        ( "help,h", "Display usage information and exit" )
        ( "model,m", po::value< std::string >(),
          "PLY model file to distribute" )
        ( "nodes,n", po::value< co::Strings >()->multitoken(),
          "Receiving node host names" )
        ( "processes,p", po::value< size_t >(),
          "Processes per node" )
        ( "options,o", po::value< unsigned >(),
          "Enabled distribution options (advanced, see protocol.h)" );

    po::variables_map vm;
    try
    {
        po::store( po::command_line_parser( argc, argv ).options(
                        options ).allow_unregistered().run(), vm );
        po::notify( vm );
    }
    catch( const std::exception& e )
    {
        std::cerr << "Error in argument parsing: " << e.what() << std::endl
                  << options << std::endl;
        LBTHROW( std::runtime_error( "Command line parsing failed" ));
    }

    if( vm.count( "help" ))
    {
        std::cout << options << std::endl;
        return EXIT_SUCCESS;
    }

    // Collect Collage Nodes to use
    co::Nodes nodes;
    if( vm.count( "nodes" ))
    {
        const co::Strings& names = vm["nodes"].as< co::Strings >();
        for( auto name : names )
        {
            co::NodePtr node = new co::Node();
            co::ConnectionDescriptionPtr desc =
                new co::ConnectionDescription( name );
            LBASSERTINFO( name.empty(),
                          "Failed to parse host description, left " << name );

            node->setHostname( desc->getHostname( ));
            node->addConnectionDescription( desc );
#if 1
            desc = new co::ConnectionDescription( *desc );
            desc->type = co::CONNECTIONTYPE_RSP;
            desc->interfacename = desc->hostname;
            desc->hostname.clear();
            node->addConnectionDescription( desc );
#endif
            nodes.push_back( node );
        }
    }

    if( nodes.empty( ))
        nodes = discoverHWSDHosts();

    if( nodes.empty( ))
    {
        std::cerr << "No hosts given or found on the network" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << nodes.size() << " nodes:";
    for( co::NodePtr node : nodes )
        std::cout << " " << node->getHostname();
    std::cout << std::endl
              << "nNodes, launch time, map time, commit time" << std::endl;

    if( vm.count( "processes" ))
    {
        size_t num = vm["processes"].as< size_t >();
        co::Nodes newNodes = nodes; // assume at least one process (ignore 0)
        while( num > 1 ) // duplicate the rest
        {
            --num;
            for( co::NodePtr node : nodes )
            {
                co::NodePtr clone = new co::Node;
                clone->setHostname( node->getHostname( ));
                for( co::ConnectionDescriptionPtr desc :
                         node->getConnectionDescriptions( ))
                {
                    clone->addConnectionDescription(
                        new co::ConnectionDescription( *desc ));
                }
                newNodes.push_back( clone );
            }
        }
        nodes.swap( newNodes );
    }

    unsigned disabledOptions = unsigned( Options::multicast );
    if( vm.count( "options" ))
        disabledOptions = ~vm["options"].as< unsigned >();

    const std::string& model = vm.count( "model" ) ?
        vm["model"].as< std::string >() :
        lunchbox::getRootPath() + "/share/Equalizer/data/screwdriver.ply";

    const auto& defaultCompressor =
        pression::data::Registry::getInstance().choose();

    // Benchmark all server options
    for( Options opts = Options::none;
         opts < Options::all;
         opts = ( opts == Options::none ) ? Options::instanceCache : opts << 1 )
    {
        if( (opts & disabledOptions) != 0 )
            continue;

        std::cout << "Options " << opts;
        if( opts & Options::compression )
            std::cout << defaultCompressor.name;
        std::cout << std::endl;

        _benchmark( argc, argv, model, opts, nodes, defaultCompressor );
    }

    // Benchmark all compression engines
    if( !(Options::compression & disabledOptions) )
    {
        const auto opts = Options::compression | Options::buffered;
        for( const auto& compressor :
                 pression::data::Registry::getInstance().getInfos( ))
        {
            std::cout << "Options " << opts << compressor.name << std::endl;
            _benchmark( argc, argv, model, opts, nodes, compressor );
        }
    }
    co::exit();
}
