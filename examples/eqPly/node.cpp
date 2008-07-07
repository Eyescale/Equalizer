
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "config.h"

using namespace eq::base;
using namespace std;

namespace eqPly
{
bool Node::configInit( const uint32_t initID )
{
    if( !eq::Node::configInit( initID ))
        return false;

    Config*    config = static_cast< Config* >( getConfig( ));
    return config->mapData( initID );
}

void Node::frameStart( const uint32_t frameID, const uint32_t frameNumber )
{
    startFrame( frameNumber ); // unlock pipe threads
    
    // Don't wait for pipes to release frame locally, sync not needed since all
    // dynamic data is multi-buffered
    releaseFrameLocal( frameNumber );
}
}
