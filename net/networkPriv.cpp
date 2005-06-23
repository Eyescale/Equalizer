
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "networkPriv.h"

#include "connectionDescription.h"
#include "network.h"
#include "pipeNetwork.h"
#include "socketNetwork.h"

#include <eq/base/log.h>

using namespace eqNet::priv;
using namespace std;

Network::Network(const uint id)
        : eqNet::Network(),
          Base(id)
{}

Network::~Network()
{
    // TODO: ConnectionDescription cleanup
}

Network* Network::create( const uint id, const eqNet::NetworkProtocol protocol )
{
    switch( protocol )
    {
        case eqNet::PROTO_TCPIP:
            return new SocketNetwork(id);

        case eqNet::PROTO_PIPE:
            return new PipeNetwork(id);

        default:
            WARN << "Protocol not implemented" << endl;
            return NULL;
    }
}

void Network::addNode( const uint nodeID,
    const eqNet::ConnectionDescription& description )
{
    ConnectionDescription *desc = new ConnectionDescription();
    *desc = description;

    if( typeid(*this) == typeid( SocketNetwork ))
    {
        if( description.TCPIP.address != NULL)
            desc->TCPIP.address = strdup( description.TCPIP.address );
    }
    else if( typeid(*this) == typeid( PipeNetwork ))
    {
        if( description.PIPE.entryFunc != NULL)
            desc->PIPE.entryFunc = strdup( description.PIPE.entryFunc );
    }
   
    _descriptions[nodeID] = desc;
}
