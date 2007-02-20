
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "client.h"

#include "commands.h"
#include "global.h"
#include "nodeFactory.h"
#include "packets.h"
#include "server.h"

#include <eq/net/command.h>
#include <eq/net/connection.h>

using namespace eq;
using namespace eqBase;
using namespace std;

Client::Client()
{
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

    return connect( RefPtr_static_cast< Server, eqNet::Node >( server ));
    // TODO: Use app-local server if failed
}

bool Client::disconnectServer( RefPtr<Server> server )
{
    if( !server->isConnected( ))
        return false;

    if( !disconnect( RefPtr_static_cast< Server, eqNet::Node >( server )))
        return false;

    return true;
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
#if 0
    while( config->isRunning( ))
    {
        config->unlockFrame();
        config->lockFrame();
    }
#else
    _used.waitGE( 1 );  // Wait to be used once (see Server::_cmdCreateConfig)
    _used.waitEQ( 0 );  // Wait to become unused (see Server::_cmdDestroyConfig)
#endif
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
