
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "global.h"
#include "nodeFactory.h"

using namespace eq;
using namespace std;

NodeFactory* Global::_nodeFactory = createNodeFactory();
string       Global::_server;
