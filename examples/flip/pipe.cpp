
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "pipe.h"

using namespace std;

bool Pipe::init( const uint32_t initID )
{
    eqNet::Session* session = getSession();
    _initData       = (InitData*)session->getMobject( initID );

    EQASSERT(_initData);
    EQINFO << "InitData " << _initData << " id " << initID << " filename "
           << _initData->getFilename() << endl;

    return eq::Pipe::init( initID );
}
