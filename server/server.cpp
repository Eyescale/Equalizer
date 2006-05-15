
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
#include <eq/client/nodeFactory.h>
#include <eq/client/packets.h>
#include <eq/net/connectionDescription.h>
#include <eq/net/init.h>
#include <eq/net/node.h>

#include <sstream>

using namespace eqs;
using namespace eqBase;
using namespace std;

eq::NodeFactory* eq::createNodeFactory()
{
    return new eq::NodeFactory;
}

Server::Server()
        : eqNet::Node( eq::CMD_SERVER_ALL ),
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
        eqNet::Connection::create(eqNet::Connection::TYPE_TCPIP);
    RefPtr<eqNet::ConnectionDescription> connDesc = 
        new eqNet::ConnectionDescription;

    connDesc->TCPIP.port = 4242;
    if( !connection->listen( connDesc ))
    {
        EQERROR << "Could not create listening socket" << endl;
        return false;
    }
    if( !listen( connection ))
        return false;

#ifdef EQLOADER
    if( nConfigs() == 0 )
    {
        EQERROR << "No configurations loaded" << endl;
        return false;
    }
#else
    if( !_loadConfig( argc, argv ))
    {
        EQERROR << "Could not load configuration" << endl;
        return false;
    }
#endif

    EQINFO << disableFlush << "Running server: " << endl << indent 
           << Global::instance() << this << exdent << enableFlush;

    _handleRequests();
    return stopListening();
}

RefPtr<eqNet::Node> Server::createNode()
{
    return new Node( eq::CMD_NODE_ALL );
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

bool Server::_loadConfig( int argc, char **argv )
{
    // TODO
    Config*    config = new Config();
    addConfig( config );
    config->setLatency(1);

    Compound* top  = new Compound;
    top->setMode( Compound::MODE_SYNC );
    config->addCompound( top );

    eqs::Node* node = new eqs::Node();
    config->addNode( node );

    RefPtr<eqNet::ConnectionDescription> description =
        new eqNet::ConnectionDescription;
    description->launchCommand = "ssh -n %h %c >& %h.log";
    description->hostname      = "localhost";
    description->launchTimeout = 100000;
    node->addConnectionDescription( description );

    Pipe* pipe = new Pipe();
    node->addPipe( pipe );

    Window* window = new Window();
    pipe->addWindow( window );

    eq::PixelViewport pvp1( 164, 350, 840, 525 );
    window->setPixelViewport( pvp1 );

    Channel* channel = new Channel();
    window->addChannel( channel );

    Compound* compound = new Compound();
    compound->setChannel( channel );

    eq::Wall wall = WALL_20INCH_16x10;
    wall.translate( wall.bottomLeft[0], 0, 0 );
    compound->setWall( wall );
    top->addChild( compound );

#if 1 // second window?
#  if 1 // on separate node?
    node = new eqs::Node();
    config->addNode( node );

    description = new eqNet::ConnectionDescription;
    description->launchCommand = "ssh -n %h %c >& %h.2.log";
    description->hostname      = "localhost";
//     description->hostname      = "go";
    description->launchTimeout = 100000;
    node->addConnectionDescription( description );
#  endif
    pipe = new Pipe();
    node->addPipe( pipe );
    
    window = new Window();
    pipe->addWindow( window );

    eq::PixelViewport pvp2( 1004, 434, 512, 357 );
    window->setPixelViewport( pvp2 );

    channel = new Channel();
    window->addChannel( channel );

    compound = new Compound();
    compound->setChannel( channel );

    eq::Wall wall2 = WALL_12INCH_4x3;
    wall2.translate( wall2.bottomRight[0], 0, 0 );
    compound->setWall( wall2 );
    top->addChild( compound );
#endif

    return true;
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
