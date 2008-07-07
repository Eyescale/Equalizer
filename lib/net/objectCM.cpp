
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "objectCM.h"

#include "nullCM.h"
#include "node.h"

eq::net::ObjectCM* eq::net::ObjectCM::ZERO = new eq::net::NullCM;
