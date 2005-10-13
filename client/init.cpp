
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "init.h"

#include "node.h"

#include <eq/net/global.h>

using namespace eq;
using namespace eqBase;
using namespace std;

namespace eq
{
    bool initLocalNode( int argc, char** argv );
    bool exitLocalNode();
}

bool eq::init( int argc, char** argv )
{
    eqNet::init( argc, argv );
    return initLocalNode( argc, argv );
}

bool eq::initLocalNode( int argc, char** argv )
{
    RefPtr<eqNet::Connection> connection =
        eqNet::Connection::create(eqNet::TYPE_TCPIP);
    RefPtr<eqNet::ConnectionDescription> connDesc = 
        new eqNet::ConnectionDescription;

    if( !connection->listen( connDesc ))
        return false;

    Node* localNode = new Node();
    return localNode->listen( connection );
}

bool eq::exit()
{
    if( !exitLocalNode( ))
        return false;

    //eqNet::exit();
    return true;
}

bool eq::exitLocalNode()
{
    eqNet::Node* localNode = Node::getLocalNode();
    if( !localNode->stopListening( ))
        return false;

    return true;
}
