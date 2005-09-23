
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "config.h"

#include "compound.h"
#include "window.h"

#include <eq/net/global.h>

using namespace eqs;

Config::Config()
        : _id(INVALID_ID)
{
}

Config::Config(const Config& from)
        : _id(INVALID_ID)
{
    const uint nCompounds = from.nCompounds();
    for( uint i=0; i<nCompounds; i++ )
    {
        Compound* compound = from.getCompound(i);
        _compounds.push_back( new Compound( *compound ));
    }

    const uint nWindows = from.nWindows();
    for( uint i=0; i<nWindows; i++ )
    {
        Window* window = from.getWindow(i);
        _windows.push_back( new Window( *window ));
    }
}
