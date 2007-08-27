
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "rawVolModel.h"

using namespace eqBase;
using namespace std;

namespace eqVol
{
bool Node::configInit( const uint32_t initID )
{
    eq::Config* config = getConfig();
    const bool  mapped = config->mapObject( &_initData, initID );
    EQASSERT( mapped );

    return eq::Node::configInit( initID );
}

bool Node::configExit()
{

    eq::Config* config = getConfig();
    config->unmapObject( &_initData );

    return eq::Node::configExit();
}

eq::FrameData* Node::getFrameData( const uint32_t id, const uint32_t version )
{
	return eq::Node::getFrameData( id, version );
}


}
