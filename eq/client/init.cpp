
/* Copyright (c) 2005-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
 *                    2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include "init.h"

#include "client.h"
#include "config.h"
#include "global.h"
#include "nodeFactory.h"
#include "os.h"
#include "server.h"

#include <eq/client/version.h>
#include <eq/fabric/configParams.h>
#include <eq/fabric/init.h>
#include <co/global.h>
#include <co/pluginRegistry.h>
#include <lunchbox/file.h>

#include <fstream>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
namespace arg = boost::program_options;

#ifdef EQ_USE_PARACOMP
#  include <pcapi.h>
#endif

#ifdef _WIN32
#  define atoll _atoi64
#endif

namespace eq
{
namespace
{
static std::ofstream* _logFile = 0;
static lunchbox::a_int32_t _initialized;
}

static bool _parseArguments( const int argc, char** argv );
static void _initPlugins();
static void _exitPlugins();
//extern void _initErrors();
//extern void _exitErrors();

bool _init( const int argc, char** argv, NodeFactory* nodeFactory )
{
    const char *env = getenv( "EQ_LOG_LEVEL" );
    if( env )
        lunchbox::Log::level = lunchbox::Log::getLogLevel( env );

    env = getenv( "EQ_LOG_TOPICS" );
    if( env )
        lunchbox::Log::topics |= atoll( env );

    lunchbox::Log::instance().setThreadName( "Main" );
    if( !_parseArguments( argc, argv ))
        return false;
    LBINFO << "Equalizer v" << Version::getString() << " initializing"
           << std::endl;

    if( ++_initialized > 1 ) // not first
    {
        LBINFO << "Equalizer client library initialized more than once"
               << std::endl;
        return true;
    }
//    _initErrors();

#ifdef AGL
    GetCurrentEventQueue();
#endif

#ifdef EQ_USE_PARACOMP
    LBINFO << "Initializing Paracomp library" << std::endl;
    PCerr err = pcSystemInitialize( 0 );
    if( err != PC_NO_ERROR )
    {
        LBERROR << "Paracomp initialization failed: " << err << std::endl;
        return false;
    }
#endif

    LBASSERT( nodeFactory );
    Global::_nodeFactory = nodeFactory;

    const std::string& programName = Global::getProgramName();
    if( programName.empty() && argc > 0 )
        Global::setProgramName( argv[0] );

    const std::string& workDir = Global::getWorkDir();
    if( workDir.empty( ))
    {
        char cwd[MAXPATHLEN];
        Global::setWorkDir( getcwd( cwd, MAXPATHLEN ));
    }

    _initPlugins();
    return fabric::init( argc, argv );
}

bool exit()
{
    if( _initialized <= 0 )
    {
        LBERROR << "Equalizer client library not initialized" << std::endl;
        return false;
    }
    if( --_initialized > 0 ) // not last
        return true;

#ifdef EQ_USE_PARACOMP
    pcSystemFinalize();
#endif

    Global::_nodeFactory = 0;
//    _exitErrors();
    _exitPlugins();
    const bool ret = fabric::exit();

    if( _logFile )
    {
#ifdef NDEBUG
        lunchbox::Log::setOutput( std::cout );
#else
        lunchbox::Log::setOutput( std::cerr );
#endif
        _logFile->close();
        delete _logFile;
        _logFile = 0;
    }
    return ret;
}

bool _parseArguments( const int argc, char** argv )
{
    typedef stde::hash_map< std::string, uint32_t > Flags;
    Flags configFlags;
    configFlags["multiprocess"] = fabric::ConfigParams::FLAG_MULTIPROCESS;
    configFlags["multiprocess_db"] = fabric::ConfigParams::FLAG_MULTIPROCESS_DB;
    configFlags["ethernet"] = fabric::ConfigParams::FLAG_NETWORK_ETHERNET;
    configFlags["infiniband"] = fabric::ConfigParams::FLAG_NETWORK_INFINIBAND;
    configFlags["2D_horizontal"] =
        fabric::ConfigParams::FLAG_LOAD_EQ_HORIZONTAL;
    configFlags["2D_vertical"] =
        fabric::ConfigParams::FLAG_LOAD_EQ_VERTICAL;
    configFlags["2D_tiles"] =
        fabric::ConfigParams::FLAG_LOAD_EQ_2D;

    arg::options_description options( "Equalizer library options" );
    options.add_options()
        ( "eq-help", "Displays usage information and exits" )
        ( "eq-logfile", arg::value< std::string >(),
          "Redirect log output to given file" )
        ( "eq-server", arg::value< std::string >(), "The server address" )
        ( "eq-config", arg::value< std::string >(),
          "The config filename or autoconfig session name" )
        ( "eq-config-flags", arg::value< Strings >()->multitoken(),
          "The autoconfig flags" )
        ( "eq-config-prefixes", arg::value< Strings >()->multitoken(),
          "The network prefix filter(s) in CIDR notation for autoconfig "
          "(white-space separated)" )
        ( "eq-render-client", arg::value< std::string >(),
          "The render client executable filename" )
    ;

    arg::variables_map vm;
    try
    {
        arg::store( arg::command_line_parser( argc, argv )
                        .options( options ).allow_unregistered().run(), vm );
        arg::notify( vm );
    }
    catch( const std::exception& e )
    {
        LBERROR << "Error in argument parsing: " << e.what() << std::endl;
        return false;
    }

    if( vm.count( "eq-help" ))
    {
        std::cout << options << std::endl;
        return false;
    }

    if( vm.count( "eq-logfile" ))
    {
        const std::string& newFile = vm["eq-logfile"].as< std::string >();
        std::ofstream* oldLog = _logFile;
        std::ofstream* newLog = new std::ofstream( newFile.c_str( ));

        if( newLog->is_open( ))
        {
            _logFile = newLog;
            lunchbox::Log::setOutput( *newLog );

            if( oldLog )
            {
                *oldLog << "Redirected log to " << newFile << std::endl;
                oldLog->close();
                delete oldLog;
            }
            else
                std::cout << "Redirected log to " << newFile << std::endl;
        }
        else
        {
            LBWARN << "Can't open log file " << newFile << ": "
                   << lunchbox::sysError << std::endl;
            delete newLog;
            newLog = 0;
        }
    }

    if( vm.count( "eq-server" ))
        Global::setServer( vm["eq-server"].as< std::string >( ));

    if( vm.count( "eq-config" ))
        Global::setConfigFile( vm["eq-config"].as< std::string >( ));

    if( vm.count( "eq-config-flags" ))
    {
        const Strings& flagStrings = vm["eq-config-flags"].as< Strings >( );
        uint32_t flags = Global::getFlags();
        for( StringsCIter i = flagStrings.begin(); i != flagStrings.end(); ++i )
        {
            Flags::const_iterator j = configFlags.find( *i );
            if( j != configFlags.end( ))
                flags |= j->second;
            else
                LBWARN << "Unknown argument for --eq-config-flags: " << *i
                       << std::endl;
        }
        Global::setFlags( flags );
    }

    if( vm.count( "eq-config-prefixes" ))
    {
        const Strings& prefixes = vm["eq-config-prefixes"].as< Strings >( );
        Global::setPrefixes( prefixes );
    }

    if( vm.count( "eq-client" ))
    {
        const std::string& renderClient = vm["eq-client"].as< std::string >();
        Global::setProgramName( renderClient );
        Global::setWorkDir( lunchbox::getDirname( renderClient ));
    }

    return true;
}

void _initPlugins()
{
    co::PluginRegistry& plugins = co::Global::getPluginRegistry();

    plugins.addDirectory( "/usr/share/Equalizer/plugins" );
    plugins.addDirectory( "/usr/local/share/Equalizer/plugins" );
#ifdef _WIN32 // final INSTALL_DIR is not known at compile time
    plugins.addDirectory( "../share/Equalizer/plugins" );
#else
    plugins.addDirectory( std::string( EQ_INSTALL_DIR ) +
                          std::string( "share/Equalizer/plugins" ));
#endif

    plugins.addDirectory( ".eqPlugins" );
    plugins.addDirectory( "/opt/local/lib" ); // MacPorts

    const char* home = getenv( "HOME" );
    if( home )
        plugins.addDirectory( std::string( home ) + "/.eqPlugins" );

#ifdef EQUALIZER_DSO_NAME
    if( plugins.addPlugin( EQUALIZER_DSO_NAME )) // Found by LDD
        return;

    // Hard-coded compile locations as backup:
    std::string absDSO = std::string( EQ_BUILD_DIR ) + "lib/" +
                         EQUALIZER_DSO_NAME;
    if( plugins.addPlugin( absDSO ))
        return;

#  ifdef NDEBUG
    absDSO = std::string( EQ_BUILD_DIR ) + "lib/Release/" + EQUALIZER_DSO_NAME;
#  else
    absDSO = std::string( EQ_BUILD_DIR ) + "lib/Debug/" + EQUALIZER_DSO_NAME;
#  endif

    if( plugins.addPlugin( absDSO ))
        return;

    LBWARN << "Built-in Equalizer plugins not loaded: " << EQUALIZER_DSO_NAME
           << " not in library search path and " << absDSO << " not found"
           << std::endl;
#else
#  ifndef NDEBUG
#    error "EQUALIZER_DSO_NAME not defined"
#  endif
    LBWARN << "Built-in Equalizer plugins not loaded: EQUALIZER_DSO_NAME not "
           << "defined" << std::endl;
#endif
}

void _exitPlugins()
{
    co::PluginRegistry& plugins = co::Global::getPluginRegistry();

#ifdef _WIN32 // final INSTALL_DIR is not known at compile time
    plugins.removeDirectory( "../share/Equalizer/plugins" );
#else
    plugins.removeDirectory( std::string( EQ_INSTALL_DIR ) +
                             std::string( "share/Equalizer/plugins" ));
#endif
    plugins.removeDirectory( "/usr/local/share/Equalizer/plugins" );
    plugins.removeDirectory( ".eqPlugins" );

    const char* home = getenv( "HOME" );
    if( home )
        plugins.removeDirectory( std::string( home ) + "/.eqPlugins" );
}

Config* getConfig( const int argc, char** argv )
{
    // 1. initialization of a local client node
    ClientPtr client = new Client;
    if( client->initLocal( argc, argv ))
    {
        // 2. connect to server
        ServerPtr server = new Server;
        if( client->connectServer( server ))
        {
            // 3. choose configuration
            fabric::ConfigParams configParams;
            Config* config = server->chooseConfig( configParams );
            if( config )
                return config;

            LBERROR << "No matching config on server" << std::endl;

            // -2. disconnect server
            client->disconnectServer( server );
        }
        else
            LBERROR << "Can't open server" << std::endl;

        // -1. exit local client node
        client->exitLocal();
    }
    else
        LBERROR << "Can't init local client node" << std::endl;

    return 0;
}

void releaseConfig( Config* config )
{
    if( !config )
        return;

    ServerPtr server = config->getServer();
    LBASSERT( server.isValid( ));
    server->releaseConfig( config );

    ClientPtr client = server->getClient();
    LBASSERT( client.isValid( ));

    client->disconnectServer( server );
    client->exitLocal();

    LBASSERTINFO( client->getRefCount() == 1, client->getRefCount( ));
    LBASSERTINFO( server->getRefCount() == 1, server->getRefCount( ));
}

}
