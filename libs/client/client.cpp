
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "client.h"

#include "commandQueue.h"
#include "global.h"
#include "init.h"
#include "nodeFactory.h"
#include "server.h"

#include <eq/fabric/commands.h>
#include <eq/fabric/nodeType.h>
#include <co/command.h>
#include <co/connection.h>
#include <co/connectionDescription.h>
#include <co/global.h>
#include <co/base/dso.h>

#ifdef WIN32_API
#  include <direct.h>  // for chdir
#  define chdir _chdir
#endif

namespace eq
{
/** @cond IGNORE */
typedef co::CommandFunc<Client> ClientFunc;

static co::ConnectionPtr _startLocalServer();
static void _joinLocalServer();

typedef co::ConnectionPtr (*eqsStartLocalServer_t)( const std::string& file );
typedef void (*eqsJoinLocalServer_t)();

typedef fabric::Client Super;
/** @endcond */

Client::Client()
        : Super()
        , _running( false )
{
    registerCommand( fabric::CMD_CLIENT_EXIT, 
                     ClientFunc( this, &Client::_cmdExit ), &_mainThreadQueue );

    EQINFO << "New client at " << (void*)this << std::endl;
}

Client::~Client()
{
    EQINFO << "Delete client at " << (void*)this << std::endl;
    close();
}

bool Client::connectServer( ServerPtr server )
{
    if( Super::connectServer( server.get( )))
    {
        server->setClient( this );
        return true;
    }

    if( !server->getConnectionDescriptions().empty() ||
        !Global::getServer().empty() || getenv( "EQ_SERVER" ))
    {
        return false;
    }

    // Use app-local server if no explicit server was set
    co::ConnectionPtr connection = _startLocalServer();
    if( !connection )
        return false;

    if( !_connect( server.get(), connection ))
        // giving up
        return false;

    server->setClient( this );
    server->_localServer = true;
    return true;
}

/** @cond IGNORE */
namespace
{
    co::base::DSO _libeqserver;
}

#define QUOTE( string ) STRINGIFY( string )
#define STRINGIFY( foo ) #foo

co::ConnectionPtr _startLocalServer()
{
    Strings dirNames;
    dirNames.push_back( "" );

#ifdef EQ_BUILD_DIR
#ifdef NDEBUG
	dirNames.push_back( std::string( QUOTE( EQ_BUILD_DIR )) + "libs/server/Release/" );
#else
	dirNames.push_back( std::string( QUOTE( EQ_BUILD_DIR )) + "libs/server/Debug/" );
#endif
    dirNames.push_back( std::string( QUOTE( EQ_BUILD_DIR )) + "libs/server/" );
#endif

#ifdef _MSC_VER
    const std::string libName = "EqualizerServer.dll";
#elif defined (_WIN32)
    const std::string libName = "libEqualizerServer.dll";
#elif defined (Darwin)
    const std::string libName = "libEqualizerServer.dylib";
#else
    const std::string libName = "libEqualizerServer.so";
#endif

    while( !_libeqserver.isOpen() && !dirNames.empty( ))
    {
        _libeqserver.open( dirNames.back() + libName );
        dirNames.pop_back();
    }

    if( !_libeqserver.isOpen( ))
    {
        EQWARN << "Can't open Equalizer server library" << std::endl;
        return 0;
    }

    eqsStartLocalServer_t eqsStartLocalServer = (eqsStartLocalServer_t)
        _libeqserver.getFunctionPointer( "eqsStartLocalServer" );

    if( !eqsStartLocalServer )
    {
        EQWARN << "Can't find server entry function eqsStartLocalServer"
               << std::endl;
        return 0;
    }

    return eqsStartLocalServer( Global::getConfigFile( ));
}

static void _joinLocalServer()
{
    eqsJoinLocalServer_t eqsJoinLocalServer = (eqsJoinLocalServer_t)
        _libeqserver.getFunctionPointer( "eqsJoinLocalServer" );

    if( !eqsJoinLocalServer )
    {
        EQWARN << "Can't find server entry function eqsJoinLocalServer"
               << std::endl;
        return;
    }

    eqsJoinLocalServer();
    _libeqserver.close();
}
/** @endcond */

bool Client::disconnectServer( ServerPtr server )
{
    bool success = true;

    // shut down process-local server (see _startLocalServer)
    if( server->_localServer )
    {
        EQASSERT( server->isConnected( ));
        EQCHECK( server->shutdown( ));
        _joinLocalServer();
        server->_localServer = false;
        server->setClient( 0 );
        EQASSERT( !server->isConnected( ))
    }
    else
    {
        server->setClient( 0 );
        success = Super::disconnectServer( server.get( ));
    }

    _mainThreadQueue.flush();
    return success;
}

bool Client::initLocal( const int argc, char** argv )
{
    bool isClient = false;
    std::string clientOpts;

    for( int i=1; i<argc; ++i )
    {
        if( std::string( "--eq-client" ) == argv[i] )
        {
            isClient = true;
            if( i < argc-1 && argv[i+1][0] != '-' ) // server-started client
            {
                clientOpts = argv[++i];

                if( !deserialize( clientOpts ))
                    EQWARN << "Failed to parse client listen port parameters"
                           << std::endl;
                EQASSERT( !clientOpts.empty( ));
            }
        }
    }
    EQINFO << "Launching " << getNodeID() << std::endl;

    if( !Super::initLocal( argc, argv ))
        return false;

    if( isClient )
    {
        EQVERB << "Client node started from command line with option " 
               << clientOpts << std::endl;

        EQCHECK( _setupClient( clientOpts ));
        clientLoop();
        exitClient();
    }

    return true;
}

bool Client::_setupClient( const std::string& clientArgs )
{
    EQASSERT( isListening( ));
    if( clientArgs.empty( ))
        return true;

    size_t nextPos = clientArgs.find( CO_SEPARATOR );
    if( nextPos == std::string::npos )
    {
        EQERROR << "Could not parse working directory: " << clientArgs
                << std::endl;
        return false;
    }

    const std::string workDir = clientArgs.substr( 0, nextPos );
    std::string description = clientArgs.substr( nextPos + 1 );

    co::Global::setWorkDir( workDir );
    if( !workDir.empty() && chdir( workDir.c_str( )) == -1 )
        EQWARN << "Can't change working directory to " << workDir << ": "
               << strerror( errno ) << std::endl;
    
    nextPos = description.find( CO_SEPARATOR );
    if( nextPos == std::string::npos )
    {
        EQERROR << "Could not parse server node type: " << description
                << " is left from " << clientArgs << std::endl;
        return false;
    }

    co::NodePtr server = createNode( fabric::NODETYPE_EQ_SERVER );
    if( !server->deserialize( description ))
        EQWARN << "Can't parse server data" << std::endl;

    EQASSERTINFO( description.empty(), description );
    if( !connect( server ))
    {
        EQERROR << "Can't connect server node" << std::endl;
        return false;
    }

    return true;
}

void Client::clientLoop()
{
    EQINFO << "Entered client loop" << std::endl;

    _running = true;
    while( _running )
        processCommand();

    // cleanup
    _mainThreadQueue.flush();
}

void Client::exitClient()
{
    bool ret = exitLocal();
    if( !eq::exit( ))
        ret = false;

    EQINFO << "Exit " << co::base::className( this ) << " process used "
           << getRefCount() << std::endl;
    ::exit( ret ? EXIT_SUCCESS : EXIT_FAILURE );
}

bool Client::hasCommands()
{
    return !_mainThreadQueue.isEmpty();
}

co::NodePtr Client::createNode( const uint32_t type )
{ 
    switch( type )
    {
        case fabric::NODETYPE_EQ_SERVER:
        {
            Server* server = new Server;
            server->setClient( this );
            return server;
        }

        default:
            return co::Node::createNode( type );
    }
}

bool Client::_cmdExit( co::Command& command )
{
    _running = false;
    // Close connection here, this is the last packet we'll get on it
    command.getLocalNode()->disconnect( command.getNode( ));
    return true;
}
}

