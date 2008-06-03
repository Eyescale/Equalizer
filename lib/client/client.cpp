
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "client.h"

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

using namespace eqBase;
using namespace std;
using eqNet::CommandFunc;

namespace eq
{
static RefPtr< eqNet::Connection > _startLocalServer();

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
    registerCommand( CMD_CLIENT_EXIT,
                     CommandFunc<Client>( this, &Client::_cmdExit ),
                     _nodeThreadQueue );

    return eqNet::Node::listen();
}

void Client::setWindowSystem( const WindowSystem windowSystem )
{
    // called from pipe threads - but only during init
    static SpinLock _lock;
    ScopedMutex< SpinLock > mutex( _lock );

    if( _nodeThreadQueue->getWindowSystem() == WINDOW_SYSTEM_NONE )
    {
        if( useMessagePump( ))
        {
            _nodeThreadQueue->setWindowSystem( windowSystem );
            EQINFO << "Client message pump set up for " << windowSystem << endl;
        }
    }
    else if( _nodeThreadQueue->getWindowSystem() != windowSystem )
        EQWARN << "Can't switch to window system " << windowSystem 
               << ", already using " <<  _nodeThreadQueue->getWindowSystem()
               << endl;
}

bool Client::stopListening()
{
    if( !eqNet::Node::stopListening( ))
        return false;

    delete _nodeThreadQueue;
    _nodeThreadQueue = 0;
    return true;
}

bool Client::connectServer( RefPtr<Server> server )
{
    if( server->isConnected( ))
        return true;
    bool explicitAddress = true;

    if( server->getConnectionDescriptions().empty( ))
    {
        RefPtr<eqNet::ConnectionDescription> connDesc = 
            new eqNet::ConnectionDescription;
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

    if( connect( RefPtr_static_cast< Server, eqNet::Node >( server )))
    {
        server->setClient( this );
        return true;
    }

    if( explicitAddress )
        return false;

    // Use app-local server if no explicit server was set
    RefPtr< eqNet::Connection > connection = _startLocalServer();
    if( !connection )
        return false;

    if( connect( RefPtr_static_cast< Server, eqNet::Node >( server ),
                 connection ))
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

typedef eqBase::RefPtr< eqNet::Connection > (*eqsStartLocalServer_t)();

RefPtr< eqNet::Connection > _startLocalServer()
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

    return eqsStartLocalServer();
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

bool Client::disconnectServer( RefPtr<Server> server )
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

    const int success = disconnect( 
        RefPtr_static_cast< Server, eqNet::Node >( server ));
    if( !success )
        EQWARN << "Server disconnect failed" << endl;

    // cleanup
    _nodeThreadQueue->flush();
    return success;
}


eqBase::RefPtr<eqNet::Node> Client::createNode( const uint32_t type )
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
            return eqNet::Node::createNode( type );
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
    return (eqNet::Node::exitClient() & eq::exit( ));
}

void Client::processCommand()
{
    eqNet::Command* command = _nodeThreadQueue->pop();
    switch( invokeCommand( *command ))
    {
        case eqNet::COMMAND_HANDLED:
        case eqNet::COMMAND_DISCARD:
            break;
            
        case eqNet::COMMAND_ERROR:
            EQERROR << "Error handling command packet" << endl;
            abort();
    }
    _nodeThreadQueue->release( command );
}

bool Client::dispatchCommand( eqNet::Command& command )
{
    EQVERB << "dispatchCommand " << command << endl;

    switch( command->datatype )
    {
        case DATATYPE_EQ_CLIENT:
            return eqNet::Base::dispatchCommand( command );

        case DATATYPE_EQ_SERVER:
        {
            RefPtr<eqNet::Node> node = command.getNode();

            EQASSERT( dynamic_cast<Server*>( node.get( )) );
            RefPtr<Server> server = 
                RefPtr_static_cast<eqNet::Node, Server>( node );

            return server->eqNet::Base::dispatchCommand( command );
        }

        default:
            return eqNet::Node::dispatchCommand( command );
    }
}

eqNet::CommandResult Client::invokeCommand( eqNet::Command& command )
{
    EQVERB << "invokeCommand " << command << endl;

    switch( command->datatype )
    {
        case DATATYPE_EQ_CLIENT:
            return eqNet::Base::invokeCommand( command );

        case DATATYPE_EQ_SERVER:
        {
            RefPtr<eqNet::Node> node = command.getNode();

            EQASSERT( dynamic_cast<Server*>( node.get( )) );
            RefPtr<Server> server = 
                RefPtr_static_cast<eqNet::Node, Server>( node );

            return server->eqNet::Base::invokeCommand( command );
        }
        default:
            return eqNet::Node::invokeCommand( command );
    }
}

eqNet::CommandResult Client::_cmdExit( eqNet::Command& command )
{
    _running = false;
    // Close connection here, this is the last packet we'll get on it
    command.getLocalNode()->disconnect( command.getNode( ));
    return eqNet::COMMAND_HANDLED;
}
}
