
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include "initData.h"
#include "plyModel.h" 

using namespace std;
using namespace eqBase;
using namespace eqNet;

bool ::Node::init( const uint32_t initID )
{
    eq::Config* config = getConfig();
    RefPtr<InitData> initData = RefPtr_static_cast< InitData, Mobject >( 
        config->getMobject( initID ));

    EQASSERT( initData.isValid( ));

    EQINFO << "Loading model " << initData->getFilename() << endl;

    _model = PlyFileIO::read( initData->getFilename().c_str( ));
    if( !_model)
        EQWARN << "Can't load model: " << initData->getFilename() << endl;

    return eq::Node::init( initID );
}

bool ::Node::exit()
{
    if( _model )
        delete _model;
    _model = NULL;

    return eq::Node::exit();
}
