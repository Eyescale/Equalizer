
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "init.h"

#include "client.h"
#include "global.h"
#include "node.h"
#include "nodeFactory.h"

#include <eq/net/init.h>

using namespace eq;
using namespace eqBase;
using namespace std;

static bool isRenderNode( int argc, char** argv )
{
    for( int i=1; i<argc; i++ )
        if( strcmp( argv[i], "--eq-client" ) == 0 )
            return true;
    return false;
}

bool eq::init( int argc, char** argv )
{
    eqNet::Node* node;

    if( isRenderNode( argc, argv ))
    {
        node = Global::getNodeFactory()->createNode();
        INFO << "Init libeq for render node" << endl;
    }
    else
    {
        node = new Client;
        INFO << "Init libeq for client" << endl;
    }

    Node::setLocalNode( node );

    char* argvListen[argc+1];
    
    for( int i=0; i<argc; i++ )
        argvListen[i] = argv[i];

    argvListen[argc] = "--eq-listen";

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
