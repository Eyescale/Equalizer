
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "pipe.h"

using namespace std;
using namespace eqBase;
using namespace eqNet;

bool Pipe::init( const uint32_t initID )
{
    eq::Config* config = getConfig();
    _initData = RefPtr_static_cast< InitData, Mobject >( 
        config->getMobject( initID ));

    EQASSERT(_initData);
    EQINFO << "InitData " << _initData.get() << " id " << initID << " filename "
           << _initData->getFilename() << endl;

    return eq::Pipe::init( initID );
}

bool Pipe::exit()
{
    _initData = NULL;
    return eq::Pipe::exit();
}
