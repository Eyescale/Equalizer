
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

Network::Network( const uint id, Session* session )
        : eqNet::Network(id),
          _session(session)
{}

Network::~Network()
{
    // TODO: ConnectionDescription cleanup
}

Network* Network::create( const uint id, Session* session, 
    const eqNet::NetworkProtocol protocol )
{
    switch( protocol )
    {
        case eqNet::PROTO_TCPIP:
            return new SocketNetwork( id, session );

        case eqNet::PROTO_PIPE:
            return new PipeNetwork( id, session );

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

    if( typeid(this) == typeid( SocketNetwork* ))
    {
        if( description.parameters.TCPIP.address != NULL)
            desc->parameters.TCPIP.address = 
                strdup( description.parameters.TCPIP.address );
    }
    else if( typeid(this) == typeid( PipeNetwork* ))
    {
        if( description.parameters.PIPE.entryFunc != NULL)
            desc->parameters.PIPE.entryFunc =
                strdup( description.parameters.PIPE.entryFunc );
    }
   
    _descriptions[nodeID] = desc;
}
