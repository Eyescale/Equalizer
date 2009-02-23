
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

    // All render data is static or multi-buffered, we can run asynchronously
    if( getIAttribute( IATTR_THREAD_MODEL ) == eq::UNDEFINED )
        setIAttribute( IATTR_THREAD_MODEL, eq::ASYNC );

    Config* config = static_cast< Config* >( getConfig( ));
    config->mapData( initID );
    return true;
}

bool Node::configExit()
{
    Config* config = static_cast< Config* >( getConfig( ));
    config->unmapData();
    
    return eq::Node::configExit();
}

}
