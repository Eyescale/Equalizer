
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "init.h"

#include "global.h"
#include "node.h"
#include "nodeFactory.h"

#include <eq/net/init.h>

using namespace eq;
using namespace eqBase;
using namespace std;

bool eq::init( int argc, char** argv )
{
    char* argvListen[argc+1];
    
    for( int i=0; i<argc; i++ )
        argvListen[i] = argv[i];

    argvListen[argc] = "--eq-listen";

    Node* node = Global::getNodeFactory()->createNode();
    Node::setLocalNode( node );

    if( !eqNet::init( argc+1, argvListen ))
    {
        ERROR << "Failed to initialise Equalizer network layer" << endl;
        Node::setLocalNode( NULL );
        delete node;
        return false;
    }

    return true;
}

bool eq::exit()
{
    return eqNet::exit();
}
