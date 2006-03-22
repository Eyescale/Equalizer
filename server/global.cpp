
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "global.h"

using namespace eqs;
using namespace eqBase;
using namespace std;

static Global *_instance = NULL;

Global::Global()
{
    for( int i=0; i<Node::IATTR_ALL; ++i )
        _nodeIAttributes[i] = EQ_UNDEFINED;
}

Global* Global::instance()
{
    if( !_instance ) 
        _instance = new Global();

    return _instance;
}

