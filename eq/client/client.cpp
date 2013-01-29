
/* Copyright (c) 2005-2013, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "client.h"

#include "commandQueue.h"
#include "config.h"
#include "node.h"
#include "global.h"
#include "init.h"
#include "nodeFactory.h"
#include "server.h"

#include <eq/fabric/commands.h>
#include <eq/fabric/configVisitor.h>
#include <eq/fabric/leafVisitor.h>
#include <eq/fabric/elementVisitor.h>
#include <eq/fabric/nodeType.h>
#include <eq/fabric/view.h>

#include <co/iCommand.h>
#include <co/connection.h>
#include <co/connectionDescription.h>
#include <co/global.h>
#include <lunchbox/dso.h>

#ifdef WIN32_API
#  include <direct.h>  // for chdir
#  define chdir _chdir
#endif

namespace eq
{
/** @cond IGNORE */
namespace detail
{
class Client
{
public:
    Client()
        : running( false )
        , modelUnit( EQ_UNDEFINED_UNIT )
    {}

    CommandQueue queue; //!< The command->node command queue.
    bool running;
    Strings activeLayouts;
    float modelUnit;
};
}

typedef co::CommandFunc<Client> ClientFunc;

static co::ConnectionPtr _startLocalServer();
static void _joinLocalServer();

typedef co::Connection* (*eqsStartLocalServer_t)( const std::string& file );
typedef void (*eqsJoinLocalServer_t)();

typedef fabric::Client Super;
/** @endcond */

Client::Client()
        : Super()
        , impl_( new detail::Client )
{
    registerCommand( fabric::CMD_CLIENT_EXIT,
                     ClientFunc( this, &Client::_cmdExit ), &impl_->queue );

    LBVERB << "New client at " << (void*)this << std::endl;
}

Client::~Client()
{
    LBVERB << "Delete client at " << (void*)this << std::endl;
    LBASSERT( isClosed( ));
    close();
    delete impl_;
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

    if( !connect( server.get(), connection ))
        // giving up
        return false;

    server->setClient( this );
    server->_localServer = true;
    return true;
}

/** @cond IGNORE */
namespace
{
    lunchbox::DSO _libeqserver;
}

co::ConnectionPtr _startLocalServer()
{
    Strings dirNames;
    dirNames.push_back( "" );

#ifdef EQ_BUILD_DIR
#  ifdef NDEBUG
    dirNames.push_back( std::string( EQ_BUILD_DIR ) + "lib/Release/" );
#  else
    dirNames.push_back( std::string( EQ_BUILD_DIR ) + "lib/Debug/" );
#  endif
    dirNames.push_back( std::string( EQ_BUILD_DIR ) + "lib/" );
#endif

#ifdef _MSC_VER
    const std::string libName = "EqualizerServer.dll";
#elif defined (_WIN32)
    const std::string libName = "libEqualizerServer.dll";
#elif defined (Darwin)
    dirNames.push_back( "/opt/local/lib/" ); // MacPorts
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
        LBWARN << "Can't open Equalizer server library" << std::endl;
        return 0;
    }

    eqsStartLocalServer_t eqsStartLocalServer = (eqsStartLocalServer_t)
        _libeqserver.getFunctionPointer( "eqsStartLocalServer" );

    if( !eqsStartLocalServer )
    {
        LBWARN << "Can't find server entry function eqsStartLocalServer"
               << std::endl;
        return 0;
    }

    co::ConnectionPtr conn = eqsStartLocalServer( Global::getConfigFile( ));
    if( conn )
        conn->unref(); // WAR "C" linkage
    return conn;
}

static void _joinLocalServer()
{
    eqsJoinLocalServer_t eqsJoinLocalServer = (eqsJoinLocalServer_t)
        _libeqserver.getFunctionPointer( "eqsJoinLocalServer" );

    if( !eqsJoinLocalServer )
    {
        LBWARN << "Can't find server entry function eqsJoinLocalServer"
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
        LBASSERT( server->isConnected( ));
        LBCHECK( server->shutdown( ));
        _joinLocalServer();
        server->_localServer = false;
        server->setClient( 0 );
        LBASSERT( !server->isConnected( ));
    }
    else
    {
        server->setClient( 0 );
        success = Super::disconnectServer( server.get( ));
    }

    impl_->queue.flush();
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
                    LBWARN << "Failed to parse client listen port parameters"
                           << std::endl;
                LBASSERT( !clientOpts.empty( ));
            }
        }
        else if( std::string( "--eq-layout" ) == argv[i] &&
                 i < argc-1 && // more args
                 argv[i+1][0] != '-' ) // next arg not an option
        {
            impl_->activeLayouts.push_back( argv[++i] );
        }
        else if( std::string( "--eq-modelunit" ) == argv[i] &&
                 i < argc-1 && // more args
                 argv[i+1][0] != '-' ) // next arg not an option
        {
            std::istringstream unitString( argv[++i] );
            unitString >> impl_->modelUnit;
        }
    }
    LBVERB << "Launching " << getNodeID() << std::endl;

    if( !Super::initLocal( argc, argv ))
        return false;

    if( isClient )
    {
        LBVERB << "Client node started from command line with option "
               << clientOpts << std::endl;

        if( !_setupClient( clientOpts ))
        {
            exitLocal();
            return false;
        }

        clientLoop();
        exitClient();
    }

    return true;
}

bool Client::_setupClient( const std::string& clientArgs )
{
    LBASSERT( isListening( ));
    if( clientArgs.empty( ))
        return true;

    size_t nextPos = clientArgs.find( CO_SEPARATOR );
    if( nextPos == std::string::npos )
    {
        LBERROR << "Could not parse working directory: " << clientArgs
                << std::endl;
        return false;
    }

    const std::string workDir = clientArgs.substr( 0, nextPos );
    std::string description = clientArgs.substr( nextPos + 1 );

    Global::setWorkDir( workDir );
    if( !workDir.empty() && chdir( workDir.c_str( )) == -1 )
        LBWARN << "Can't change working directory to " << workDir << ": "
               << strerror( errno ) << std::endl;

    nextPos = description.find( CO_SEPARATOR );
    if( nextPos == std::string::npos )
    {
        LBERROR << "Could not parse server node type: " << description
                << " is left from " << clientArgs << std::endl;
        return false;
    }

    co::NodePtr server = createNode( fabric::NODETYPE_SERVER );
    if( !server->deserialize( description ))
        LBWARN << "Can't parse server data" << std::endl;

    LBASSERTINFO( description.empty(), description );
    if( !connect( server ))
    {
        LBERROR << "Can't connect server node using " << *server << std::endl;
        return false;
    }

    return true;
}

void Client::clientLoop()
{
    LBINFO << "Entered client loop" << std::endl;

    impl_->running = true;
    while( impl_->running )
        processCommand();

    // cleanup
    impl_->queue.flush();
}

bool Client::exitLocal()
{
    impl_->activeLayouts.clear();
    impl_->modelUnit = EQ_UNDEFINED_UNIT;
    return fabric::Client::exitLocal();
}

void Client::exitClient()
{
    bool ret = exitLocal();
    LBINFO << "Exit " << lunchbox::className( this ) << " process used "
           << getRefCount() << std::endl;

    if( !eq::exit( ))
        ret = false;
    ::exit( ret ? EXIT_SUCCESS : EXIT_FAILURE );
}

bool Client::hasCommands()
{
    return !impl_->queue.isEmpty();
}

co::CommandQueue* Client::getMainThreadQueue()
{
    return &impl_->queue;
}

const Strings& Client::getActiveLayouts()
{
    return impl_->activeLayouts;
}

float Client::getModelUnit() const
{
    return impl_->modelUnit;
}

co::NodePtr Client::createNode( const uint32_t type )
{
    switch( type )
    {
        case fabric::NODETYPE_SERVER:
        {
            Server* server = new Server;
            server->setClient( this );
            return server;
        }

        default:
            return Super::createNode( type );
    }
}

bool Client::_cmdExit( co::ICommand& command )
{
    impl_->running = false;
    // Close connection here, this is the last command we'll get on it
    command.getLocalNode()->disconnect( command.getNode( ));
    return true;
}

namespace
{
class StopNodesVisitor : public ServerVisitor
{
public:
    virtual ~StopNodesVisitor() {}
    virtual VisitorResult visitPre( Node* node )
        {
            node->dirtyClientExit();
            return TRAVERSE_CONTINUE;
        }
};
}

void Client::notifyDisconnect( co::NodePtr node )
{
    if( node->getType() == fabric::NODETYPE_SERVER )
    {
        // local command dispatching
        co::OCommand( this, this, fabric::CMD_CLIENT_EXIT );

        ServerPtr server = static_cast< Server* >( node.get( ));
        StopNodesVisitor stopNodes;
        server->accept( stopNodes );
    }
    fabric::Client::notifyDisconnect( node );
}

}
