
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "server.h"

#include "channel.h"
#include "compound.h"
#include "config.h"
#include "global.h"
#include "node.h"
#include "pipe.h"
#include "window.h"

#include <eq/base/refPtr.h>
#include <eq/client/packets.h>
#include <eq/net/connectionDescription.h>
#include <eq/net/init.h>
#include <eq/net/node.h>

#include <sstream>

using namespace eqs;
using namespace eqBase;
using namespace std;

Server::Server()
        : eqNet::Node( eq::CMD_SERVER_CUSTOM ),
          _configID(0)
{
    registerCommand( eq::CMD_SERVER_CHOOSE_CONFIG, this, 
                     reinterpret_cast<CommandFcn>( &eqs::Server::pushRequest ));
    registerCommand( eq::REQ_SERVER_CHOOSE_CONFIG, this, 
                     reinterpret_cast<CommandFcn>(
                         &eqs::Server::_reqChooseConfig ));
    registerCommand( eq::CMD_SERVER_RELEASE_CONFIG, this, 
                     reinterpret_cast<CommandFcn>( 
                         &eqs::Server::pushRequest ));
    registerCommand( eq::REQ_SERVER_RELEASE_CONFIG, this, 
                     reinterpret_cast<CommandFcn>( 
                         &eqs::Server::_reqReleaseConfig ));
    EQINFO << "New server @" << (void*)this << endl;
}

bool Server::run( int argc, char **argv )
{
    eqNet::init( argc, argv );

    RefPtr<eqNet::Connection> connection =
        eqNet::Connection::create( eqNet::Connection::TYPE_TCPIP );
    RefPtr<eqNet::ConnectionDescription> connDesc = 
        connection->getDescription();

    connDesc->TCPIP.port = 4242;
    if( !connection->listen( ))
    {
        EQERROR << "Could not create listening socket" << endl;
        return false;
    }
    if( !listen( connection ))
        return false;

    if( nConfigs() == 0 )
    {
        EQERROR << "No configurations loaded" << endl;
        return false;
    }

    EQINFO << disableFlush << "Running server: " << endl << indent 
           << Global::instance() << this << exdent << enableFlush;

    _handleRequests();
    return stopListening();
}

void Server::handleDisconnect( Node* node )
{
    Node::handleDisconnect( node );
    // TODO: free up resources requested by disconnected node
}

void Server::addConfig( Config* config )
{ 
    config->_server = this;
    _configs.push_back( config );
}

void Server::mapConfig( Config* config )
{
    ostringstream stringStream;
    stringStream << "EQ_CONFIG_" << (++_configID);

    const bool   mapped = mapSession( this, config, stringStream.str( ));
    EQASSERT( mapped );
}

//===========================================================================
// packet handling methods
//===========================================================================
eqNet::CommandResult Server::handlePacket( eqNet::Node* node,
                                           const eqNet::Packet* packet )
{
    EQASSERT( node );
    switch( packet->datatype )
    {
        case eq::DATATYPE_EQ_SERVER:
            return handleCommand( node, packet );
            
        default:
            EQUNIMPLEMENTED;
            return eqNet::COMMAND_ERROR;
    }
}

void Server::_handleRequests()
{
    Node*          node;
    eqNet::Packet* packet;

    while( true )
    {
        _requestQueue.pop( &node, &packet );

        switch( dispatchPacket( node, packet ))
        {
            case eqNet::COMMAND_HANDLED:
                break;

            case eqNet::COMMAND_ERROR:
                EQERROR << "Error handling command packet" << endl;
                abort();

            case eqNet::COMMAND_RESCHEDULE:
            case eqNet::COMMAND_PROPAGATE:
                EQUNIMPLEMENTED;
        }
    }
}

eqNet::CommandResult Server::_reqChooseConfig( eqNet::Node* node, const eqNet::Packet* pkg )
{
    eq::ServerChooseConfigPacket* packet = (eq::ServerChooseConfigPacket*)pkg;
    EQINFO << "Handle choose config " << packet << endl;

    // TODO
    Config* config = nConfigs()>0 ? getConfig(0) : NULL;
    
    eq::ServerChooseConfigReplyPacket reply( packet );

    if( config==NULL )
    {
        reply.configID = EQ_INVALID_ID;
        node->send( reply );
        return eqNet::COMMAND_HANDLED;
    }

    Config* appConfig = new Config( *config );
    appConfig->setApplicationNode( node );

    mapConfig( appConfig );

    // TODO: move to open: appConfig->setAppName( appName );
    const string rendererInfo = packet->rendererInfo;
    const size_t colonPos     = rendererInfo.find( ':' );
    const string workDir      = rendererInfo.substr( 0, colonPos );
    const string renderClient = rendererInfo.substr( colonPos + 1 );
 
    appConfig->setWorkDir( workDir );
    appConfig->setRenderClient( renderClient );

    reply.configID = appConfig->getID();
    _appConfigs[reply.configID] = appConfig;

    const string& name = appConfig->getName();
    
    node->send( reply, name );
    return eqNet::COMMAND_HANDLED;
}

eqNet::CommandResult Server::_reqReleaseConfig( eqNet::Node* node, const eqNet::Packet* pkg )
{
    eq::ServerReleaseConfigPacket* packet = (eq::ServerReleaseConfigPacket*)pkg;
    EQINFO << "Handle release config " << packet << endl;

    Config* config = _appConfigs[packet->configID];
    if( !config )
    {
        EQWARN << "Release request for unknown config" << endl;
        return eqNet::COMMAND_HANDLED;
    }

    const bool unmapped = unmapSession( config );
    EQASSERT( unmapped );

    _appConfigs.erase( packet->configID );
    delete config;
    return eqNet::COMMAND_HANDLED;
}

std::ostream& eqs::operator << ( std::ostream& os, const Server* server )
{
    if( !server )
        return os;
    
    const uint32_t nConfigs = server->nConfigs();
    
    os << disableFlush << disableHeader << "server " << endl;
    os << "{" << endl << indent;
    
    for( uint32_t i=0; i<nConfigs; i++ )
        os << server->getConfig(i);
    
    os << exdent << "}"  << enableHeader << enableFlush << endl;

    return os;
}
