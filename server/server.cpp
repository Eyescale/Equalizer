
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
                     reinterpret_cast<CommandFcn>( 
                         &eqs::Server::_cmdChooseConfig ));
    registerCommand( eq::CMD_SERVER_RELEASE_CONFIG, this, 
                     reinterpret_cast<CommandFcn>( 
                         &eqs::Server::_cmdReleaseConfig ));
    INFO << "new " << this << endl;
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
    {
        ERROR << "Could not create listening socket" << endl;
        return false;
    }
    if( !listen( connection ))
        return false;

    if( !_loadConfig( argc, argv ))
    {
        ERROR << "Could not load configuration" << endl;
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
    ASSERT( mapped );

    eqs::Node* node = new eqs::Node();
    config->addNode( node );

    RefPtr<eqNet::ConnectionDescription> description =
        new eqNet::ConnectionDescription;
    description->launchCommand = "ssh -n %h %c";
    description->hostname      = "localhost";
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
    Config*     clone = new Config( this );
    const string name = _genConfigName();
    const bool mapped = mapSession( this, clone, name );
    ASSERT( mapped );

    const uint nCompounds = config->nCompounds();
    for( uint i=0; i<nCompounds; i++ )
    {
        Compound* compound      = config->getCompound(i);
        Compound* compoundClone = new Compound();

        // TODO clone tree
        clone->addCompound( compoundClone );
        compoundClone->setChannel( compound->getChannel() ); // replaced below
    }

    const uint nNodes = config->nNodes();
    for( uint i=0; i<nNodes; i++ )
    {
        eqs::Node* node      = config->getNode(i);
        eqs::Node* nodeClone = new eqs::Node();
        
        clone->addNode( nodeClone );

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
            
            nodeClone->addPipe( pipeClone );
            
            const uint nWindows = pipe->nWindows();
            for( uint k=0; k<nWindows; k++ )
            {
                Window* window      = pipe->getWindow(k);
                Window* windowClone = new Window();
            
                pipeClone->addWindow( windowClone );
            
                const uint nChannels = window->nChannels();
                for( uint l=0; l<nChannels; l++ )
                {
                    Channel* channel      = window->getChannel(l);
                    Channel* channelClone = new Channel();

                    windowClone->addChannel( channelClone );
                    
                    ReplaceChannelData data;
                    data.oldChannel = channel;
                    data.newChannel = channelClone;

                    for( uint m=0; m<nCompounds; m++ )
                    {
                        Compound* compound      = clone->getCompound(m);
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
void Server::handlePacket( eqNet::Node* node, const eqNet::Packet* packet )
{
    ASSERT( node );
    switch( packet->datatype )
    {
        case eq::DATATYPE_EQ_SERVER:
            handleCommand( node, packet );
            break;
            
        default:
            ERROR << "unimplemented " << packet << endl;
            abort();
    }
}

void Server::_handleRequests()
{
    Node*          node;
    eqNet::Packet* packet;

    while( true )
    {
        _requestQueue.pop( &node, &packet );
        dispatchPacket( node, packet );
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

    const string& name = appConfig->getName();
    reply.sessionNameLength = name.size() + 1;
    
    node->send( reply );
    node->send( name.c_str(), reply.sessionNameLength );
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
