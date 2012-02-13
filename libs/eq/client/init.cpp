
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
#include <co/base/file.h>
#include <co/base/global.h>
#include <co/base/pluginRegistry.h>

#include <fstream>

#ifdef EQ_USE_PARACOMP
#  include <pcapi.h>
#endif

namespace eq
{
namespace
{
static std::ofstream* _logFile = 0;
static co::base::a_int32_t _initialized;
}

static void _parseArguments( const int argc, char** argv );
static void _initPlugins();
static void _exitPlugins();
//extern void _initErrors();
//extern void _exitErrors();

bool _init( const int argc, char** argv, NodeFactory* nodeFactory )
{
    co::base::Log::instance().setThreadName( "Main" );
    _parseArguments( argc, argv );
    EQINFO << "Equalizer v" << Version::getString() << " initializing"
           << std::endl;

    if( ++_initialized > 1 ) // not first
    {
        EQINFO << "Equalizer client library initialized more than once"
               << std::endl;
        return true;
    }
//    _initErrors();

#ifdef AGL
    GetCurrentEventQueue();
#endif

#ifdef EQ_USE_PARACOMP
    EQINFO << "Initializing Paracomp library" << std::endl;
    PCerr err = pcSystemInitialize( 0 );
    if( err != PC_NO_ERROR )
    {
        EQERROR << "Paracomp initialization failed: " << err << std::endl;
        return false;
    }
#endif

    EQASSERT( nodeFactory );
    Global::_nodeFactory = nodeFactory;

    _initPlugins();
    return fabric::init( argc, argv );
}

bool exit()
{
    if( _initialized <= 0 )
    {
        EQERROR << "Equalizer client library not initialized" << std::endl;
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
        co::base::Log::setOutput( std::cout );
#else
        co::base::Log::setOutput( std::cerr );
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
                if( _logFile )
                {
                    EQWARN << "Redirecting log to " << argv[i] << std::endl;
                    _logFile->close();
                    delete _logFile;
                }

                _logFile = new std::ofstream( argv[i] );
                if( _logFile->is_open( ))
                    co::base::Log::setOutput( *_logFile );
                else
                {
                    EQWARN << "Can't open log file " << argv[i] << ": "
                           << co::base::sysError << std::endl;
                    delete _logFile;
                    _logFile = 0;
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
                co::Global::setWorkDir( co::base::getDirname( argv[i] ));
            }
        }
    }
}
    
void _initPlugins()
{
    co::base::PluginRegistry& plugins = co::base::Global::getPluginRegistry();

    plugins.addDirectory( "/usr/share/Equalizer/plugins" );
    plugins.addDirectory( "/usr/local/share/Equalizer/plugins" );
#ifdef _WIN32 // final INSTALL_DIR is not known at compile time
    plugins.addDirectory( "../share/Equalizer/plugins" );
#else
    plugins.addDirectory( std::string( EQ_INSTALL_DIR ) + 
                          std::string( "share/Equalizer/plugins" ));
#endif

    plugins.addDirectory( ".eqPlugins" );

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

    EQWARN << "Built-in Equalizer plugins not loaded: " << EQUALIZER_DSO_NAME
           << " not in library search path and " << absDSO << " not found"
           << std::endl;
#else
#  ifndef NDEBUG
#    error "EQUALIZER_DSO_NAME not defined"
#  endif
    EQWARN << "Built-in Equalizer plugins not loaded: EQUALIZER_DSO_NAME not "
           << "defined" << std::endl;
#endif
}

void _exitPlugins()
{
    co::base::PluginRegistry& plugins = co::base::Global::getPluginRegistry();

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

            EQERROR << "No matching config on server" << std::endl;

            // -2. disconnect server
            client->disconnectServer( server );
        }
        else
            EQERROR << "Can't open server" << std::endl;
        
        // -1. exit local client node
        client->exitLocal();
    }
    else
        EQERROR << "Can't init local client node" << std::endl;

    return 0;
}

void releaseConfig( Config* config )
{
    if( !config )
        return;

    ServerPtr server = config->getServer();
    EQASSERT( server.isValid( ));
    server->releaseConfig( config );

    ClientPtr client = server->getClient();
    EQASSERT( client.isValid( ));

    client->disconnectServer( server );
    client->exitLocal();

    EQASSERTINFO( client->getRefCount() == 1, client->getRefCount( ));
    EQASSERTINFO( server->getRefCount() == 1, server->getRefCount( ));
}

}
