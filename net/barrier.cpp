
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "barrier.h"

using namespace eqNet;

Barrier::Barrier( const uint32_t height )
        : _height( height )
{
}

void Barrier::getInstanceInfo( uint32_t* typeID, std::string& data )
{
    *typeID = MOBJECT_EQNET_BARRIER;
    
    char height[8];
    snprintf( height, 8, "%d", _height );
    data = height;
}
