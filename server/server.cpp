
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

using namespace eqs;
using namespace eqBase;
using namespace std;

#define SMALL_PACKET_SIZE 1024

Server::Server()
        : _configID(1)
{
    for( int i=0; i<eq::CMD_SERVER_ALL; i++ )
        _cmdHandler[i] =  &eqs::Server::_cmdUnknown;

    _cmdHandler[eq::CMD_SERVER_CHOOSE_CONFIG] = &eqs::Server::_cmdChooseConfig;
    _cmdHandler[eq::CMD_SERVER_RELEASE_CONFIG] =&eqs::Server::_cmdReleaseConfig;
}

bool Server::run( int argc, char **argv )
{
    eqNet::init( argc, argv );

    RefPtr<eqNet::Connection> connection =
        eqNet::Connection::create(eqNet::TYPE_TCPIP);
    RefPtr<eqNet::ConnectionDescription> connDesc = 
        new eqNet::ConnectionDescription;

    connDesc->parameters.TCPIP.port = 4242;
    if( !connection->listen( connDesc ))
        return false;

    if( !listen( connection ))
        return false;

    if( !_loadConfig( argc, argv ))
        return false;

    _handleRequests();
    return stopListening();
}

RefPtr<eqNet::Node> Server::createNode()
{
    return new Node();
}

void Server::handleDisconnect( Node* node )
{
    Node::handleDisconnect( node );
    // TODO: free up resources requested by disconnected node
}

bool Server::_loadConfig( int argc, char **argv )
{
    // TODO
    Config*    config = new Config();
    const bool mapped = mapSession( this, config, "EQ_CONFIG_" + _configID++ );
    ASSERT( mapped );

    eqs::Node* node = new eqs::Node();
    config->addNode( node );

    RefPtr<eqNet::ConnectionDescription> description =
        new eqNet::ConnectionDescription;
    description->launchCommand = "ssh -n %h %c";
    description->hostname      = "localhost";
    description->launchTimeout = 10000;
    node->addConnectionDescription( description );

    Pipe* pipe = new Pipe();
    node->addPipe( pipe );
    
    Window* window = new Window();
    pipe->addWindow( window );

    Channel* channel = new Channel();
    window->addChannel( channel );

    Compound* compound = new Compound();
    compound->setChannel( channel );
    config->addCompound( compound );

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
    Config     *clone  = new Config();
    const bool  mapped = mapSession( this, clone, "EQ_CONFIG_" + _configID++ );
    ASSERT( mapped );

    const uint nCompounds = config->nCompounds();
    for( uint i=0; i<nCompounds; i++ )
    {
        Compound* compound      = config->getCompound(i);
        Compound* compoundClone = new Compound();
        
        compoundClone->setChannel( compound->getChannel() ); // replaced below
        clone->addCompound( compoundClone );
    }

    const uint nNodes = config->nNodes();
    for( uint i=0; i<nNodes; i++ )
    {
        eqs::Node* node      = config->getNode(i);
        eqs::Node* nodeClone = new eqs::Node();

        const uint nConnectionDescriptions = node->nConnectionDescriptions();
        for( uint j=0; j<nConnectionDescriptions; j++ )
        {
            RefPtr<eqNet::ConnectionDescription> desc = 
                node->getConnectionDescription(j);

            nodeClone->addConnectionDescription( desc );
        }

        const uint nPipes = node->nPipes();
        for( uint j=0; j<nPipes; j++ )
        {
            Pipe* pipe      = node->getPipe(j);
            Pipe* pipeClone = new Pipe();
            
            const uint nWindows = pipe->nWindows();
            for( uint k=0; k<nWindows; k++ )
            {
                Window* window      = pipe->getWindow(k);
                Window* windowClone = new Window();
            
                const uint nChannels = window->nChannels();
                for( uint l=0; l<nChannels; l++ )
                {
                    Channel* channel      = window->getChannel(l);
                    Channel* channelClone = new Channel();
                    
                    ReplaceChannelData data;
                    data.oldChannel = channel;
                    data.newChannel = channelClone;

                    for( uint m=0; m<nCompounds; m++ )
                    {
                        Compound* compound      = clone->getCompound(m);
                        Compound::traverse( compound, replaceChannelCB, 
                                            replaceChannelCB, NULL, &data );
                    }

                    windowClone->addChannel( channelClone );
                }
            
                pipeClone->addWindow( windowClone );
            }
            
            nodeClone->addPipe( pipeClone );
        }
        
        clone->addNode( nodeClone );
    }

    // utilisation data will be shared between copies
    return clone;
}

//===========================================================================
// packet handling methods
//===========================================================================
void Server::handlePacket( eqNet::Node* node, const eqNet::Packet* packet )
{
    switch( packet->datatype )
    {
        case eq::DATATYPE_EQ_SERVER:
            if( packet->command < eq::CMD_SERVER_ALL )
            {
                (this->*_cmdHandler[packet->command])( node, packet );
            }
            else
                ERROR << "Unknown command " << packet->command << endl;
            break;
            
        case eq::DATATYPE_EQ_CONFIG:
        case eq::DATATYPE_EQ_NODE:
        {
            const eq::ConfigPacket* configPacket = (eq::ConfigPacket*)
                packet;
            const uint              configID     = configPacket->configID;
            Config*                 config       = _appConfigs[configID];
            ASSERT( config );
            
            config->handlePacket( node, configPacket );
            break;
        }
        default:
            ERROR << "unimplemented" << endl;
            abort();
    }
}

void Server::pushRequest( eqNet::Node* node, const eqNet::Packet* packet )
{
    Request* request;

    _freeRequestsLock.set();
    if( _freeRequests.empty( ))
        request = new Request;
    else
    {
        request = _freeRequests.front();
        _freeRequests.pop_front();
    }
    _freeRequestsLock.unset();

    request->node = node;

    const size_t packetSize = packet->size;
    if( packetSize <= SMALL_PACKET_SIZE )
    {
        if( !request->packet )
            request->packet = (eqNet::Packet*)malloc( SMALL_PACKET_SIZE );
    }
    else
    {
        if( request->packet )
            free( request->packet );

        request->packet = (eqNet::Packet*)malloc( packetSize );
    }
        
    memcpy( request->packet, packet, packetSize );
    request->packet->command++; // REQ must always follow CMD
    _requests.push( request );
}

void Server::_freeRequest( Request* request )
{
    if( request->packet->size > SMALL_PACKET_SIZE )
    {
        free( request->packet );
        request->packet = NULL;
    }
    
    _freeRequestsLock.set();
    _freeRequests.push_back( request );
    _freeRequestsLock.unset();
}

void Server::_handleRequests()
{
    while( true )
    {
        Request*       request = _requests.pop();
        handlePacket( request->node, request->packet );
        _freeRequest( request );
    }
}

void Server::_cmdChooseConfig( eqNet::Node* node, const eqNet::Packet* pkg )
{
    eq::ServerChooseConfigPacket* packet = (eq::ServerChooseConfigPacket*)pkg;
    INFO << "Handle choose config " << packet << endl;

    char* appName = (char*)alloca( packet->appNameLength );
    node->recv( appName, packet->appNameLength );

    char* renderClient = (char*)alloca( packet->renderClientLength );
    node->recv( renderClient, packet->renderClientLength );

    // TODO
    Config* config = nConfigs()>0 ? getConfig(0) : NULL;
    
    eq::ServerChooseConfigReplyPacket reply( packet );

    if( config==NULL )
    {
        reply.configID = INVALID_ID;
        node->send( reply );
        return;
    }

    Config* appConfig = _cloneConfig( config );

    appConfig->setAppName( appName );
    appConfig->setRenderClient( renderClient );

    reply.configID = appConfig->getID();
    _appConfigs[reply.configID] = appConfig;

    node->send( reply );
}

void Server::_cmdReleaseConfig( eqNet::Node* node, const eqNet::Packet* pkg )
{
    eq::ServerReleaseConfigPacket* packet = (eq::ServerReleaseConfigPacket*)pkg;
    INFO << "Handle release config " << packet << endl;

    Config* config = _appConfigs[packet->configID];
    if( !config )
    {
        WARN << "Release request for unknown config" << endl;
        return;
    }

    _appConfigs.erase( packet->configID );
    delete config;
}
