
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "global.h"
#include "nodeFactory.h"

using namespace eq;

NodeFactory* Global::_nodeFactory = createNodeFactory();
