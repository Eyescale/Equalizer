
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "config.h"

#include "packets.h"
#include "server.h"

#include <eq/net/global.h>

using namespace eq;
using namespace std;

Config::Config( const uint id, Server* server )
        : _server(server)
{
    ASSERT( id != INVALID_ID );
    ASSERT( server );

    _id = id;

    for( int i=0; i<CMD_CONFIG_ALL; i++ )
        _cmdHandler[i] = &eq::Config::_cmdUnknown;

    _cmdHandler[CMD_CONFIG_INIT_REPLY] = &eq::Config::_cmdInitReply;
}

bool Config::map()
{ 
    eqNet::Node* localNode = eqNet::Node::getLocalNode();
    if( !localNode )
        return false;

    return localNode->mapSession( _server, this, _id ); 
}

bool Config::init()
{
    ConfigInitPacket packet( _id );
    packet.requestID = _requestHandler.registerRequest();
    _server->send( packet );
    return ( _requestHandler.waitRequest( packet.requestID ) != 0 );
}

bool Config::exit()
{
}

void Config::handleCommand( const ConfigPacket* packet )
{
    VERB << "handleCommand " << packet << endl;
    ASSERT( packet->command < CMD_CONFIG_ALL );

    (this->*_cmdHandler[packet->command])(packet);
}

void Config::_cmdUnknown( const ConfigPacket* packet )
{
    ERROR << "unimplemented" << endl;
    abort();
}

void Config::_cmdInitReply( const ConfigPacket* pkg )
{
    ConfigInitReplyPacket* packet = (ConfigInitReplyPacket*)pkg;
    INFO << "handle init reply " << packet << endl;

    _requestHandler.serveRequest( packet->requestID, (void*)(packet->result) );
}
