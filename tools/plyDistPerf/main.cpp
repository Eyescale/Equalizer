
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

#include "client.h"
#include "hwsd.h"
#include "server.h"

#include "vertexBufferDist.h"
#include <triply/vertexBufferRoot.h>
#include <pression/compressorInfo.h>

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

namespace fs = boost::filesystem;
namespace acc = boost::accumulators;
namespace arg = boost::program_options;

namespace
{
std::string _addEnv( const std::string& env )
{
    const char* value = getenv( env.c_str( ));
    if( value )
        return env + "=" + value + " ";
    return std::string();
}

void _benchmark( plydist::Server& server, const co::Nodes& nodes,
                 const std::string& launchCommand )
{
    for( co::NodePtr node : nodes )
        server.getLocalNode()->launch( node, launchCommand );

    server.result.doneNodes.timedWaitGE( nodes.size(), 120000 );
    acc::accumulator_set< float, acc::features< acc::tag::mean,
                                                acc::tag::variance >> acc;
    acc = std::for_each( server.result.times.begin(), server.result.times.end(),
                         acc );

    if( server.result.doneNodes.get() != nodes.size( ))
        std::cout << "Error: Only " << server.result.doneNodes.get()
                  << " of " << nodes.size() << " nodes did run" << std::endl;
    else
        std::cout << nodes.size() << ", " << acc::mean( acc ) << ", "
                  << std::sqrt( acc::variance( acc )) << std::endl;
}

void _benchmark( const int argc, char** argv, const std::string& model,
                 const plydist::Options opts, const co::Nodes& nodes,
                 const uint32_t compressor = EQ_COMPRESSOR_INVALID )
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

    for( size_t i = 1; i <= nodes.size(); ++i )
    {
        plydist::Server server( argc, argv, model, opts, compressor );
        _benchmark( server, co::Nodes( nodes.begin(), nodes.begin() + i ),
                    launchCommand );
    }
    if( opts & plydist::Options::compression )
        std::cout << co::DataOStream::printStatistics << std::endl;
}

std::vector< uint32_t > _findByteCompressors()
{
    const auto& plugins = co::Global::getPluginRegistry().getPlugins();
    std::vector< uint32_t > names;

    for( pression::Plugin* plugin : plugins )
        for( const auto& info : plugin->getInfos( ))
            if( info.tokenType == EQ_COMPRESSOR_DATATYPE_BYTE )
                names.push_back( info.name );

    std::sort( names.begin(), names.end( ));
    return names;
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

    arg::options_description options(
        "Benchmark tool for triply object distribution" );
    options.add_options()
        ( "help,h", "Display usage information and exit" )
        ( "model,m", arg::value< std::string >(),
          "PLY model file to distribute" )
        ( "nodes,n", arg::value< co::Strings >()->multitoken(),
          "Receiving node host names" )
        ( "processes,p", arg::value< size_t >(),
          "Processes per node" )
        ( "options,o", arg::value< size_t >(),
          "Enabled distribution options (advanced, see protocol.h)" );

    arg::variables_map vm;
    try
    {
        arg::store( arg::command_line_parser( argc, argv ).options(
                        options ).allow_unregistered().run(), vm );
        arg::notify( vm );
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

            node->addConnectionDescription( desc );
#if 0
            desc = new co::ConnectionDescription( *desc );
            desc->type = co::CONNECTIONTYPE_RSP;
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
        std::cout << " "
                  << node->getConnectionDescriptions().front()->getHostname();
    std::cout << std::endl;

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

    size_t disabledOptions = 0;
    if( vm.count( "options" ))
        disabledOptions = ~vm["options"].as< size_t >();

    const std::string& model = vm.count( "model" ) ?
        vm["model"].as< std::string >() :
        lunchbox::getRootPath() + "/share/Equalizer/data/screwdriver.ply";

#if 1
    // Benchmark all server options
    for( plydist::Options opts = plydist::Options::none;
         opts < plydist::Options::all; ++opts )
    {
        if( (opts & disabledOptions) != 0 )
            continue;

        std::cout << "Options " << opts << std::endl;
        _benchmark( argc, argv, model, opts, nodes );
    }
#endif

    // Benchmark all compression engines
    if( !(plydist::Options::compression & disabledOptions) )
    {
        for( uint32_t type : _findByteCompressors( ))
        {
            std::cout << "Compressor 0x" << std::hex << type << std::dec
                      << std::endl;
            _benchmark( argc, argv, model,
                     plydist::Options::compression | plydist::Options::buffered,
                        nodes, type );
        }
    }
    co::exit();
}
