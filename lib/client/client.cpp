
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include "commands.h"
#include "global.h"
#include "init.h"
#include "nodeFactory.h"
#include "packets.h"
#include "server.h"

#include <eq/net/command.h>
#include <eq/net/connection.h>

#ifdef WIN32
#  define EQ_DL_ERROR getErrorString( GetLastError( ))
#else
#  include <dlfcn.h>
#  define EQ_DL_ERROR dlerror()
#endif

using namespace eq::base;
using namespace std;

namespace eq
{
typedef net::CommandFunc<Client> ClientFunc;

static net::ConnectionPtr _startLocalServer();
static void _joinLocalServer();

Client::Client()
        : _nodeThreadQueue( 0 )
        , _running( false )
{
    EQINFO << "New client at " << (void*)this << endl;
}

Client::~Client()
{
    EQINFO << "Delete client at " << (void*)this << endl;
    stopListening();
}

bool Client::listen()
{
    EQASSERT( !_nodeThreadQueue );
    _nodeThreadQueue = new CommandQueue;

    registerCommand( CMD_CLIENT_EXIT, ClientFunc( this, &Client::_cmdExit ),
                     _nodeThreadQueue );

    return net::Node::listen();
}

void Client::setWindowSystem( const WindowSystem windowSystem )
{
    // access already locked by Config::setWindowSystem (caller)
    if( _nodeThreadQueue->getWindowSystem() == WINDOW_SYSTEM_NONE )
    {
        _nodeThreadQueue->setWindowSystem( windowSystem );
        EQINFO << "Client command message pump set up for " << windowSystem
               << endl;
    }
    else if( _nodeThreadQueue->getWindowSystem() != windowSystem )
        EQWARN << "Can't switch to window system " << windowSystem 
               << ", already using " <<  _nodeThreadQueue->getWindowSystem()
               << endl;
}

bool Client::stopListening()
{
    if( !net::Node::stopListening( ))
        return false;

    delete _nodeThreadQueue;
    _nodeThreadQueue = 0;
    return true;
}

bool Client::connectServer( ServerPtr server )
{
    if( server->isConnected( ))
        return true;
    bool explicitAddress = true;

    if( server->getConnectionDescriptions().empty( ))
    {
        net::ConnectionDescriptionPtr connDesc = 
            new net::ConnectionDescription;
        connDesc->TCPIP.port = EQ_DEFAULT_PORT;
    
        const string globalServer = Global::getServer();
        const char*  envServer    = getenv( "EQ_SERVER" );
        string       address      = !globalServer.empty() ? globalServer :
                                    envServer             ? envServer : 
                                    "localhost";

        if( !connDesc->fromString( address ))
            EQWARN << "Can't parse server address " << address << endl;
        EQASSERT( address.empty( ));
        EQINFO << "Connecting to " << connDesc->toString() << endl;

        server->addConnectionDescription( connDesc );

        if( globalServer.empty() && !envServer )
            explicitAddress = false;
    }

    if( connect( net::NodePtr( server.get( )) ))
    {
        server->setClient( this );
        return true;
    }

    if( explicitAddress )
        return false;

    // Use app-local server if no explicit server was set
    net::ConnectionPtr connection = _startLocalServer();
    if( !connection )
        return false;

    if( connect( server.get(), connection ))
    {
        server->setClient( this );
        server->_localServer = true;
        return true;
    }

    // giving up
    return false;
}

namespace
{
#ifdef WIN32_VC
static HMODULE _libeqserver = 0;
#elif defined (WIN32)
static HMODULE _libeqserver = 0;
#elif defined (Darwin)
static void* _libeqserver = 0;
#else
static void* _libeqserver = 0;
#endif
}

typedef net::ConnectionPtr (*eqsStartLocalServer_t)( const std::string& file );

net::ConnectionPtr _startLocalServer()
{
    if( !_libeqserver )
    {
#ifdef WIN32_VC
        _libeqserver = LoadLibrary( "EqualizerServer.dll" );
#elif defined (WIN32)
        _libeqserver = LoadLibrary( "libeqserver.dll" );
#elif defined (Darwin)
        _libeqserver = dlopen( "libeqserver.dylib", RTLD_LAZY );
#else
        _libeqserver = dlopen( "libeqserver.so", RTLD_LAZY );
#endif
    }

    if( !_libeqserver )
    {
        EQWARN << "Can't open Equalizer server library: " << EQ_DL_ERROR <<endl;
        return 0;
    }

#ifdef WIN32
    eqsStartLocalServer_t eqsStartLocalServer = (eqsStartLocalServer_t)
        GetProcAddress( _libeqserver, "eqsStartLocalServer" );
#else
    eqsStartLocalServer_t eqsStartLocalServer = (eqsStartLocalServer_t)
        dlsym( _libeqserver, "eqsStartLocalServer" );
#endif

    if( !eqsStartLocalServer )
    {
        EQWARN << "Can't find server entry function eqsStartLocalServer: "
               << EQ_DL_ERROR << endl;
        return 0;
    }

    return eqsStartLocalServer( Global::getConfigFile( ));
}

typedef void (*eqsJoinLocalServer_t)();

static void _joinLocalServer()
{
    if( !_libeqserver )
    {
        EQWARN << "Equalizer server library not opened" << endl;
        return;
    }

#ifdef WIN32
    eqsJoinLocalServer_t eqsJoinLocalServer = (eqsJoinLocalServer_t)
        GetProcAddress( _libeqserver, "eqsJoinLocalServer" );
#else
    eqsJoinLocalServer_t eqsJoinLocalServer = (eqsJoinLocalServer_t)
        dlsym( _libeqserver, "eqsJoinLocalServer" );
#endif

    if( !eqsJoinLocalServer )
    {
        EQWARN << "Can't find server entry function eqsJoinLocalServer: "
               << EQ_DL_ERROR << endl;
        return;
    }

    eqsJoinLocalServer();
}

bool Client::disconnectServer( ServerPtr server )
{
    if( !server->isConnected( ))
    {
        EQWARN << "Trying to disconnect unconnected server" << endl;
        return false;
    }

    // shut down process-local server (see _startLocalServer)
    if( server->_localServer )
    {
        server->shutdown();
        _joinLocalServer();
    }

    server->setClient( 0 );
    server->_localServer = false;

    const int success = disconnect( server.get( ));
    if( !success )
        EQWARN << "Server disconnect failed" << endl;

    // cleanup
    _nodeThreadQueue->flush();
    return success;
}


net::NodePtr Client::createNode( const uint32_t type )
{ 
    switch( type )
    {
        case TYPE_EQ_SERVER:
        {
            Server* server = new Server;
            server->setClient( this );
            return server;
        }

        default:
            return net::Node::createNode( type );
    }
}

bool Client::clientLoop()
{
    EQINFO << "Entered client loop" << endl;

    _running = true;
    while( _running )
        processCommand();

    // cleanup
    _nodeThreadQueue->flush();
    EQASSERT( !hasSessions( ));

    return true;
}

bool Client::exitClient()
{
    return (net::Node::exitClient() & eq::exit( ));
}

void Client::processCommand()
{
    net::Command* command = _nodeThreadQueue->pop();
    if( !command ) // just a wakeup()
        return;

    switch( invokeCommand( *command ))
    {
        case net::COMMAND_HANDLED:
        case net::COMMAND_DISCARD:
            break;
            
        case net::COMMAND_ERROR:
            EQERROR << "Error handling command packet" << endl;
            abort();
    }
    command->release();
}

bool Client::dispatchCommand( net::Command& command )
{
    EQVERB << "dispatchCommand " << command << endl;

    switch( command->datatype )
    {
        case DATATYPE_EQ_CLIENT:
            return net::Dispatcher::dispatchCommand( command );

        case DATATYPE_EQ_SERVER:
        {
            net::NodePtr node = command.getNode();

            EQASSERT( dynamic_cast< Server* >( node.get( )) );
            ServerPtr server = static_cast< Server* >( node.get( ));

            return server->net::Dispatcher::dispatchCommand( command );
        }

        default:
            return net::Node::dispatchCommand( command );
    }
}

net::CommandResult Client::invokeCommand( net::Command& command )
{
    EQVERB << "invokeCommand " << command << endl;

    switch( command->datatype )
    {
        case DATATYPE_EQ_CLIENT:
            return net::Dispatcher::invokeCommand( command );

        case DATATYPE_EQ_SERVER:
        {
            net::NodePtr node = command.getNode();

            EQASSERT( dynamic_cast<Server*>( node.get( )) );
            ServerPtr server = static_cast<Server*>( node.get( ));

            return server->net::Dispatcher::invokeCommand( command );
        }
        default:
            return net::Node::invokeCommand( command );
    }
}

net::CommandResult Client::_cmdExit( net::Command& command )
{
    _running = false;
    // Close connection here, this is the last packet we'll get on it
    command.getLocalNode()->disconnect( command.getNode( ));
    return net::COMMAND_HANDLED;
}
}
