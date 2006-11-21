
/* 
 * Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
 * All rights reserved.
 *
 * The pipe object is responsible for maintaining the frame-specific data. The
 * identifier passed by the application contains the version of the frame
 * data corresponding to the rendered frame. The pipe's start frame
 * callback synchronizes the thread-local instance of the frame data to this
 * version.
 */

#include "pipe.h"

using namespace std;
using namespace eqBase;
using namespace eqNet;

/*
 */
bool Pipe::init( const uint32_t initID )
{
    eq::Config* config = getConfig();

    RefPtr<InitData> initData  = (InitData*)config->getObject( initID );
    _frameData = initData->getFrameData();
    EQASSERT( _frameData );

    return eq::Pipe::init( initID );
}

bool Pipe::exit()
{
    _frameData = NULL;
    return eq::Pipe::exit();
}

void Pipe::startFrame( const uint32_t frameID )
{
    _frameData->sync( frameID );
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
