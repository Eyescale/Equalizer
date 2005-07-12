
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "nodePriv.h"
#include "sessionPriv.h"

using namespace eqNet::priv;

Node::Node( const uint id )
        : Base(id),
          eqNet::Node()
{
}

Network* Node::_findBestNetwork( Node* toNode )
{
    for( size_t i=0; i<_networks.size(); i++ )
    {
        Network* network = _networks[i];
        if( toNode->isInNetwork( network ))
            return network;
    }
    return NULL;
    // TODO: find union of our, to networks
    // find fastest network in union
    // find gateway if union is empty.
}
