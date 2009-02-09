
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "nodeFactory.h"

#include "channel.h"
#include "client.h"
#include "config.h"
#include "node.h"
#include "pipe.h"
#include "server.h"
#include "window.h"

namespace eq
{

Config* NodeFactory::createConfig( ServerPtr parent )
{
    return new Config( parent );
}
void NodeFactory::releaseConfig( Config* config )
{
    delete config;
}

Node* NodeFactory::createNode( Config* parent )
{
    return new Node( parent );
}
void NodeFactory::releaseNode( Node* node )
{
    delete node;
}

Pipe* NodeFactory::createPipe( Node* parent )
{
    return new Pipe( parent );
}
void NodeFactory::releasePipe( Pipe* pipe )
{
    delete pipe;
}

Window* NodeFactory::createWindow( Pipe* parent )
{
    return new Window( parent );
}
void NodeFactory::releaseWindow( Window* window )
{
    delete window;
}

Channel* NodeFactory::createChannel( Window* parent )
{
    return new Channel( parent );
}

void NodeFactory::releaseChannel( Channel* channel )
{
    delete channel;
}

}

