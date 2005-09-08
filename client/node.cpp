
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "node.h"

#include <eq/net/connection.h>

using namespace eq;
using namespace std;

Node* Node::_localNode = new Node();
