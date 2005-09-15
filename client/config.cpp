
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "config.h"

#include <eq/net/global.h>

using namespace eq;
using namespace std;

Config::Config( const uint id )
        : _id(id)
{
    ASSERT( id != INVALID_ID );
}
