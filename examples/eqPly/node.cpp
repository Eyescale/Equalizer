
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

using namespace eqBase;
using namespace std;

namespace eqPly
{
bool Node::configInit( const uint32_t initID )
{
    if( !eq::Node::configInit( initID ))
        return false;

    eq::Config* config = getConfig();
    const bool  mapped = config->mapObject( &_initData, initID );
    EQASSERT( mapped );

    const string& filename = _initData.getFilename();
    EQINFO << "Loading model " << filename << endl;

    _model = new Model();

    if( _initData.useInvertedFaces() )
        _model->useInvertedFaces();

    if ( !_model->readFromFile( filename.c_str() ) )
    {
        EQWARN << "Can't load model: " << filename << endl;
        delete _model;
        _model = 0;
    }
    
    return true;
}

bool Node::configExit()
{
    delete _model;
    _model = 0;

    eq::Config* config = getConfig();
    config->unmapObject( &_initData );

    return eq::Node::configExit();
}

void Node::frameStart( const uint32_t frameID, const uint32_t frameNumber )
{
    startFrame( frameNumber ); // unlock pipe threads
    
    // Don't wait for pipes to release frame locally, sync not needed since all
    // dynamic data is multi-buffered
    releaseFrameLocal( frameNumber );
}
}
