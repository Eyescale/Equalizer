
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

//#include "plyFileIO.h"

using namespace eqBase;
using namespace std;

namespace eqPly
{
bool Node::configInit( const uint32_t initID )
{
    eq::Config* config = getConfig();
    const bool  mapped = config->mapObject( &_initData, initID );
    EQASSERT( mapped );

    const string& filename = _initData.getFilename();
    EQINFO << "Loading model " << filename << endl;

//    _model = PlyFileIO::read( filename.c_str( ));
//    if( !_model)
//        EQWARN << "Can't load model: " << filename << endl;
    _model = new Model();
    if ( !_model->readFromFile( filename.c_str() ) )
    {
        EQWARN << "Can't load model: " << filename << endl;
        delete _model;
    }
    
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
}
