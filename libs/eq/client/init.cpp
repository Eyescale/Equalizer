
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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
#include "configParams.h"
#include "global.h"
#include "nodeFactory.h"
#include "os.h"
#include "server.h"

#include <eq/client/version.h>
#include <eq/fabric/init.h>
#include <co/global.h>
#include <co/pluginRegistry.h>
#include <lunchbox/file.h>

#include <fstream>

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

static void _parseArguments( const int argc, char** argv );
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
    _parseArguments( argc, argv );
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

void _parseArguments( const int argc, char** argv )
{
    // We do not use getopt_long baecause of:
    // - reordering of arguments
    // - different behaviour of GNU and BSD implementations
    // - incomplete man pages
    for( int i=1; i<argc; ++i )
    {
        if( strcmp( "--eq-logfile", argv[i] ) == 0 )
        {
            ++i;
            if( i<argc )
            {
                std::ofstream* oldLog = _logFile;
                std::ofstream* newLog = new std::ofstream( argv[i] );

                if( newLog->is_open( ))
                {
                    _logFile = newLog;
                    lunchbox::Log::setOutput( *newLog );

                    if( oldLog )
                    {
                        *oldLog << "Redirected log to " << argv[i] << std::endl;
                        oldLog->close();
                        delete oldLog;
                    }
                }
                else
                {
                    LBWARN << "Can't open log file " << argv[i] << ": "
                           << lunchbox::sysError << std::endl;
                    delete newLog;
                    newLog = 0;
                }
            }
        }
        if( strcmp( "--eq-server", argv[i] ) == 0 )
        {
            ++i;
            if( i<argc )
                Global::setServer( argv[i] );
        }
        else if( strcmp( "--eq-config", argv[i] ) == 0 )
        {
            ++i;
            if( i<argc )
                Global::setConfigFile( argv[i] );
        }
        else if( strcmp( "--eq-config-flags", argv[i] ) == 0 )
        {
            ++i;
            if( i >= argc )
                break;

            uint32_t flags = Global::getFlags();
            if( strcmp( "multiprocess", argv[i] ))
                flags |= ConfigParams::FLAG_MULTIPROCESS;
            else if( strcmp( "multiprocess_db", argv[i] ))
                flags |= ConfigParams::FLAG_MULTIPROCESS_DB;
            Global::setFlags( flags );
        }
        else if( strcmp( "--eq-render-client", argv[i] ) == 0 )
        {
            ++i;
            if( i<argc )
            {
                co::Global::setProgramName( argv[i] );
                co::Global::setWorkDir( lunchbox::getDirname( argv[i] ));
            }
        }
    }
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
            ConfigParams configParams;
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
