
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

using namespace std;

bool Node::init( const uint32_t initID )
{
    eqNet::Session* session  = getSession();
    InitData*       initData = (InitData*)session->getMobject( initID );

    EQASSERT(initData);

    _model = PlyFileIO::read( initData->getFilename( ));
    if( !_model)
        return false;

    return eq::Node::init( initID );
}

