
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "pipe.h"

using namespace std;
using namespace eqBase;
using namespace eqNet;

bool Pipe::init( const uint32_t initID )
{
    eq::Config* config = getConfig();

    _initData  = (InitData*)config->getObject( initID );
    _frameData = _initData->getFrameData();
    EQASSERT( _frameData );

    EQASSERT(_initData);
    EQINFO << "InitData " << _initData.get() << " id " << initID << " filename "
           << _initData->getFilename() << endl;

    return eq::Pipe::init( initID );
}

bool Pipe::exit()
{
    _initData = NULL;
    _frameData = NULL;
    return eq::Pipe::exit();
}

void Pipe::startFrame( const uint32_t frameID )
{
    _frameData->sync( frameID );
}
