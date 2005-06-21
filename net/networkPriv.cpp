
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "network.h"

#include "pipeNetwork.h"
#include "socketNetwork.h"

using namespace eqNet;
using namespace eqNet::priv;
using namespace std;

Network::Network(const uint id)
        : _id(id)
{}

static Network* create(const uint id, const NetworkProtocol protocol)
{
    switch( protocol )
    {
        case PROTO_TCPIP:
            return new SocketNetwork(id);

        case PROTO_PIPE:
            return new PipeNetwork(id);

        default:
            WARN << "Protocol not implemented" << endl;
            return NULL;
    }
}
