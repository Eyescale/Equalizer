
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "server.h"

#include "channel.h"
#include "compound.h"
#include "config.h"
#include "node.h"
#include "pipe.h"
#include "window.h"

#include <eq/packets.h>
#include <eq/base/refPtr.h>
#include <eq/net/connectionDescription.h>
#include <eq/net/init.h>
#include <eq/net/node.h>

#include <sstream>

using namespace eqs;
using namespace eqBase;
using namespace std;

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
    EQINFO << "new " << this << endl;
}

bool Server::run( int argc, char **argv )
{
    eqNet::init( argc, argv );

    RefPtr<eqNet::Connection> connection =
        eqNet::Connection::create(eqNet::TYPE_TCPIP);
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

    if( !_loadConfig( argc, argv ))
    {
        EQERROR << "Could not load configuration" << endl;
        return false;
    }
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

string Server::_genConfigName()
{
    ostringstream stringStream;
    stringStream << "EQ_CONFIG_" << (++_configID);
    return stringStream.str();
}

bool Server::_loadConfig( int argc, char **argv )
{
    // TODO
    Config*    config = new Config( this );
    const string name = _genConfigName();
    const bool mapped = mapSession( this, config, name );
    EQASSERT( mapped );

    Compound* top  = new Compound;
    top->setMode( Compound::MODE_SYNC );
    config->addCompound( top );

    eqs::Node* node = new eqs::Node();
    config->addNode( node );

    RefPtr<eqNet::ConnectionDescription> description =
        new eqNet::ConnectionDescription;
    description->launchCommand = "ssh -n %h %c >& %h.log";
    description->hostname      = "benjy";
    description->launchTimeout = 100000;
    node->addConnectionDescription( description );

    Pipe* pipe = new Pipe();
    node->addPipe( pipe );
    
    Window* window = new Window();
    pipe->addWindow( window );

    Channel* channel = new Channel();
    window->addChannel( channel );

    Compound* compound = new Compound();
    compound->setChannel( channel );

    eq::Wall wall = WALL_20INCH_16x10;
    compound->setWall( wall );
    top->addChild( compound );

    node = new eqs::Node();
    config->addNode( node );

    description = new eqNet::ConnectionDescription;
    description->launchCommand = "ssh -n eile@%h %c >& %h.log";
    description->hostname      = "go";
    description->launchTimeout = 100000;
    node->addConnectionDescription( description );

    pipe = new Pipe();
    node->addPipe( pipe );
    
    window = new Window();
    pipe->addWindow( window );

    channel = new Channel();
    window->addChannel( channel );

    compound = new Compound();
    compound->setChannel( channel );

    eq::Wall wall2 = WALL_12INCH_4x3;
    compound->setWall( wall2 );
    top->addChild( compound );

    addConfig( config );
    return true;
}

//---------------------------------------------------------------------------
// clones a config, used to create a per-application copy of the server config
//---------------------------------------------------------------------------
struct ReplaceChannelData
{
    Channel* oldChannel;
    Channel* newChannel;
};

static TraverseResult replaceChannelCB( Compound* compound, void* userData )
{
    ReplaceChannelData* data = (ReplaceChannelData*)userData;

    if( compound->getChannel() == data->oldChannel )
        compound->setChannel( data->newChannel );

    return TRAVERSE_CONTINUE;
}

Config* Server::_cloneConfig( Config* config )
{
    Config*      clone  = new Config( this );
    const string name   = _genConfigName();
    const bool   mapped = mapSession( this, clone, name );
    EQASSERT( mapped );

    const uint32_t nCompounds = config->nCompounds();
    for( uint32_t i=0; i<nCompounds; i++ )
    {
        Compound* compound      = config->getCompound(i);
        Compound* compoundClone = new Compound( *compound );

        clone->addCompound( compoundClone );
        // channel is replaced below
    }

    const uint32_t nNodes = config->nNodes();
    for( uint32_t i=0; i<nNodes; i++ )
    {
        eqs::Node* node      = config->getNode(i);
        eqs::Node* nodeClone = new eqs::Node();
        
        clone->addNode( nodeClone );

        const uint32_t nConnectionDescriptions =node->nConnectionDescriptions();
        for( uint32_t j=0; j<nConnectionDescriptions; j++ )
        {
            RefPtr<eqNet::ConnectionDescription> desc = 
                node->getConnectionDescription(j);

            nodeClone->addConnectionDescription( desc );
        }

        const uint32_t nPipes = node->nPipes();
        for( uint32_t j=0; j<nPipes; j++ )
        {
            Pipe* pipe      = node->getPipe(j);
            Pipe* pipeClone = new Pipe();
            
            nodeClone->addPipe( pipeClone );
            
            const uint32_t nWindows = pipe->nWindows();
            for( uint32_t k=0; k<nWindows; k++ )
            {
                Window* window      = pipe->getWindow(k);
                Window* windowClone = new Window();
            
                pipeClone->addWindow( windowClone );
            
                const uint32_t nChannels = window->nChannels();
                for( uint32_t l=0; l<nChannels; l++ )
                {
                    Channel* channel      = window->getChannel(l);
                    Channel* channelClone = new Channel();

                    windowClone->addChannel( channelClone );
                    
                    ReplaceChannelData data;
                    data.oldChannel = channel;
                    data.newChannel = channelClone;

                    for( uint32_t m=0; m<nCompounds; m++ )
                    {
                        Compound* compound = clone->getCompound(m);
                        Compound::traverse( compound, replaceChannelCB, 
                                            replaceChannelCB, NULL, &data );
                    }
                }
            }
        }
    }

    // utilisation data will be shared between copies
    return clone;
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
        reply.configID = INVALID_ID;
        node->send( reply );
        return eqNet::COMMAND_HANDLED;
    }

    Config* appConfig = _cloneConfig( config );

    // TODO: move to open: appConfig->setAppName( appName );
    appConfig->setRenderClient( packet->renderClient );

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

    _appConfigs.erase( packet->configID );
    delete config;
    return eqNet::COMMAND_HANDLED;
}
