
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "configUpdateDataVisitor.h"

#include "channel.h"

using namespace std;

namespace eq
{
namespace server
{
ConfigUpdateDataVisitor::ConfigUpdateDataVisitor()
    : _lastDrawChannel( 0 )
    , _lastDrawWindow( 0 )
    , _lastDrawPipe( 0 )
{}


ConfigVisitor::Result ConfigUpdateDataVisitor::visitPre( Node* node )
{
    _lastDrawPipe = 0;
    return TRAVERSE_CONTINUE;
}
ConfigVisitor::Result ConfigUpdateDataVisitor::visitPost( Node* node )
{
    node->setLastDrawPipe( _lastDrawPipe );
    return TRAVERSE_CONTINUE;
}

ConfigVisitor::Result ConfigUpdateDataVisitor::visitPre( Pipe* pipe )
{
    _lastDrawWindow = 0;
    return TRAVERSE_CONTINUE;
}
ConfigVisitor::Result ConfigUpdateDataVisitor::visitPost( Pipe* pipe )
{
    pipe->setLastDrawWindow( _lastDrawWindow );
    if( _lastDrawWindow )
        _lastDrawPipe = pipe;
    return TRAVERSE_CONTINUE;
}

ConfigVisitor::Result ConfigUpdateDataVisitor::visitPre( Window* window )
{
    _lastDrawChannel = 0;
    return TRAVERSE_CONTINUE;
}
ConfigVisitor::Result ConfigUpdateDataVisitor::visitPost( Window* window )
{
    window->setLastDrawChannel( _lastDrawChannel );
    if( _lastDrawChannel )
        _lastDrawWindow = window;
    return TRAVERSE_CONTINUE;
}

ConfigVisitor::Result ConfigUpdateDataVisitor::visit( Channel* channel )
{
    if( channel->getLastDrawCompound( ))
        _lastDrawChannel = channel;

    return TRAVERSE_CONTINUE;
}
 
}
}
