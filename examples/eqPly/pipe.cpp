
/* 
 * Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
 * All rights reserved.
 *
 * The pipe object is responsible for maintaining the frame-specific data. The
 * identifier passed by the application contains the version of the frame data
 * corresponding to the rendered frame. The pipe's start frame callback
 * synchronizes the thread-local instance of the frame data to this version.
 */

#include "pipe.h"

#include "node.h"
#include <eq/eq.h>

using namespace std;
using namespace eqBase;

bool Pipe::configInit( const uint32_t initID )
{
    const Node*     node        = static_cast<Node*>( getNode( ));
    const InitData& initData    = node->getInitData();
    const uint32_t  frameDataID = initData.getFrameDataID();
    eq::Config*     config      = getConfig();

    const bool mapped = config->mapObject( &_frameData, frameDataID );
    EQASSERT( mapped );

    return eq::Pipe::configInit( initID );
}

bool Pipe::configExit()
{
    eq::Config* config = getConfig();
    config->unmapObject( &_frameData );

    return eq::Pipe::configExit();
}

void Pipe::frameStart( const uint32_t frameID, const uint32_t frameNumber )
{
    _frameData.sync( frameID );
    startFrame( frameNumber );
}

GLuint Pipe::getDisplayList( const void* key )
{
    return _displayLists[key];
}

GLuint Pipe::newDisplayList( const void* key )
{
    _displayLists[key] = glGenLists( 1 );
    return _displayLists[key];
}
