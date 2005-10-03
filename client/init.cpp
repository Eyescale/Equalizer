
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "init.h"

#include "node.h"

#include <eq/net/global.h>

using namespace eq;
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
    eqBase::RefPtr<eqNet::Connection> connection =
        eqNet::Connection::create(eqNet::TYPE_TCPIP);
    eqNet::ConnectionDescription connDesc;

    if( !connection->listen( connDesc ))
        return false;

    eqNet::Node* localNode = Node::getLocalNode();
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
