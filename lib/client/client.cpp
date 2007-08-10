
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
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

using namespace eq;
using namespace eqBase;
using namespace std;

static RefPtr< eqNet::Connection > _startLocalServer();

Client::Client()
        : _commandQueue( 0 ),
          _running( false )
{
    registerCommand( CMD_CLIENT_EXIT,
                     eqNet::CommandFunc<Client>( this, &Client::_cmdPush ));
    registerCommand( REQ_CLIENT_EXIT,
                     eqNet::CommandFunc<Client>( this, &Client::_reqExit ));
    EQINFO << "New client at " << (void*)this << endl;
}

Client::~Client()
{
    EQINFO << "Delete client at " << (void*)this << endl;
    stopListening();
}

bool Client::listen()
{
    EQASSERT( !_commandQueue );
    if( useMessagePump( )) _commandQueue = new eq::CommandQueue;
    else                   _commandQueue = new eqNet::CommandQueue;
    return eqNet::Node::listen();
}
bool Client::stopListening()
{
    if( !eqNet::Node::stopListening( ))
        return false;

    delete _commandQueue;
    _commandQueue = 0;
}

bool Client::connectServer( RefPtr<Server> server )
{
    if( server->isConnected( ))
        return true;
    bool explicitAddress = true;

    if( server->nConnectionDescriptions() == 0 )
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
        server->_client = this;
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
        server->_client = this;
        return true;
    }

    // giving up
    return false;
}

typedef eqBase::RefPtr< eqNet::Connection > (*eqsStartLocalServer_t)();

RefPtr< eqNet::Connection > _startLocalServer()
{
#ifdef WIN32_VC
    HMODULE libeqserver = LoadLibrary( "EqualizerServer.dll" );
#elif defined (WIN32)
    HMODULE libeqserver = LoadLibrary( "libeqserver.dll" );
#elif defined (Darwin)
    void* libeqserver = dlopen( "libeqserver.dylib", RTLD_LAZY );
#else
    void* libeqserver = dlopen( "libeqserver.so", RTLD_LAZY );
#endif

    if( !libeqserver )
    {
        EQWARN << "Can't open Equalizer server library: " << EQ_DL_ERROR <<endl;
        return 0;
    }

#ifdef WIN32
    eqsStartLocalServer_t eqsStartLocalServer = (eqsStartLocalServer_t)
        GetProcAddress( libeqserver, "eqsStartLocalServer" );
#else
    eqsStartLocalServer_t eqsStartLocalServer = (eqsStartLocalServer_t)
        dlsym( libeqserver, "eqsStartLocalServer" );
#endif

    if( !eqsStartLocalServer )
    {
        EQWARN << "Can't find server entry function eqsStartLocalServer: "
               << EQ_DL_ERROR << endl;
        return 0;
    }

    return eqsStartLocalServer();
}

bool Client::disconnectServer( RefPtr<Server> server )
{
    if( !server->isConnected( ))
    {
        EQWARN << "Trying to disconnect unconnected server" << endl;
        return false;
    }

    server->_client = 0;
    const int success = disconnect( 
        RefPtr_static_cast< Server, eqNet::Node >( server ));
    if( !success )
        EQWARN << "Server disconnect failed" << endl;

    // cleanup
    _commandQueue->flush();
    return success;
}


eqBase::RefPtr<eqNet::Node> Client::createNode( const uint32_t type )
{ 
    switch( type )
    {
        case TYPE_EQ_SERVER:
            return new Server;

        default:
            return eqNet::Node::createNode( type );
    }
}

eqNet::Session* Client::createSession()
{
    return Global::getNodeFactory()->createConfig(); 
}

void Client::clientLoop()
{
    EQINFO << "Entered client loop" << endl;

    _running = true;
    while( _running )
        processCommand();

    // cleanup
    _commandQueue->flush();
    EQASSERT( !hasSessions( ));

    return;
}

void Client::processCommand()
{
    eqNet::Command* command = _commandQueue->pop();
    switch( dispatchCommand( *command ))
    {
        case eqNet::COMMAND_HANDLED:
        case eqNet::COMMAND_DISCARD:
            break;
            
        case eqNet::COMMAND_ERROR:
            EQERROR << "Error handling command packet" << endl;
            abort();
            
        case eqNet::COMMAND_PUSH:
            EQUNIMPLEMENTED;
        case eqNet::COMMAND_REDISPATCH:
            EQUNIMPLEMENTED;
    }
}

bool Client::runClient( const std::string& clientArgs )
{
    const bool ret = eqNet::Node::runClient( clientArgs );

    exitLocal();
    eq::exit();

    EQINFO << "Leaving auto-launched client process " << getRefCount() << endl;
    ::exit( ret ? EXIT_SUCCESS : EXIT_FAILURE ); // never return from eq::init
    return ret;
}

eqNet::CommandResult Client::handleCommand( eqNet::Command& command )
{
    EQVERB << "handleCommand " << command << endl;

    switch( command->datatype )
    {
        case DATATYPE_EQ_CLIENT:
            return invokeCommand( command );

        case DATATYPE_EQ_SERVER:
        {
            RefPtr<eqNet::Node> node = command.getNode();

            EQASSERT( dynamic_cast<Server*>( node.get( )) );
            RefPtr<Server> server = 
                RefPtr_static_cast<eqNet::Node, Server>( node );

            return server->invokeCommand( command );
        }
        default:
            return eqNet::COMMAND_ERROR;
    }
}

eqNet::CommandResult Client::_reqExit( eqNet::Command& command )
{
    _running = false;
    // Close connection here, this is the last packet we'll get on it
    command.getLocalNode()->disconnect( command.getNode( ));
    return eqNet::COMMAND_HANDLED;
}
