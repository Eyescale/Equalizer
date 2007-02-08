
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "objectCM.h"

#include "nullCM.h"
#include "node.h"

eqNet::ObjectCM* eqNet::ObjectCM::ZERO = new eqNet::NullCM;
