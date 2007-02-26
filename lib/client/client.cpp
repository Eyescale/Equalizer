
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

using namespace eq;
using namespace eqBase;
using namespace std;

Client::Client()
        : _running( false )
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
}

bool Client::connectServer( RefPtr<Server> server )
{
    if( server->isConnected( ))
        return false;

    if( server->nConnectionDescriptions() == 0 )
    {
        RefPtr<eqNet::ConnectionDescription> connDesc = 
            new eqNet::ConnectionDescription;
    
        connDesc->type = eqNet::CONNECTIONTYPE_TCPIP;
        
        const string globalServer = Global::getServer();
        const char*  envServer = getenv( "EQ_SERVER" );
        const string address   = !globalServer.empty() ? globalServer :
                                     envServer ? envServer : "localhost";
        const size_t colonPos  = address.rfind( ':' );

        if( colonPos == string::npos )
            connDesc->hostname = address;
        else
        {
            connDesc->hostname   = address.substr( 0, colonPos );
            string port          = address.substr( colonPos+1 );
            connDesc->TCPIP.port = atoi( port.c_str( ));
        }

        if( !connDesc->TCPIP.port )
            connDesc->TCPIP.port = EQ_DEFAULT_PORT;

        server->addConnectionDescription( connDesc );
    }

    if( connect( RefPtr_static_cast< Server, eqNet::Node >( server )))
    {
        server->_client = this;
        return true;
    }
    return false;
    // TODO: Use app-local server if failed
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
    _commandQueue.flush();
    return success;
}


eqBase::RefPtr<eqNet::Node> Client::createNode( const CreateReason reason )
{ 
    switch( reason )
    {
        case REASON_CLIENT_LAUNCH:
            return new Server;

        default:
            return new eqNet::Node;
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
    _commandQueue.flush();
    EQASSERT( !hasSessions( ));

    return;
}

void Client::processCommand()
{
    eqNet::Command* command = _commandQueue.pop();
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
    return eqNet::COMMAND_HANDLED;
}
