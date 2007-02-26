
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "plyFileIO.h"

using namespace std;
using namespace eqBase;

bool Node::configInit( const uint32_t initID )
{
    eq::Config* config = getConfig();
    const bool  mapped = config->mapObject( &_initData, initID );
    EQASSERT( mapped );

    EQINFO << "Loading model " << _initData.getFilename() << endl;

    _model = PlyFileIO::read( _initData.getFilename().c_str( ));
    if( !_model)
        EQWARN << "Can't load model: " << _initData.getFilename() << endl;

    return eq::Node::configInit( initID );
}

bool Node::configExit()
{
    if( _model )
        delete _model;
    _model = NULL;

    eq::Config* config = getConfig();
    config->unmapObject( &_initData );

    return eq::Node::configExit();
}
