
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "configParams.h"

#include <eq/net/global.h>

using namespace eq;
using namespace std;

ConfigParams::ConfigParams()
{
    appName = eqNet::Global::getProgramName();
    compoundModes = 
        COMPOUND_MODE_2D    | 
        COMPOUND_MODE_DPLEX |
        COMPOUND_MODE_EYE   |
        COMPOUND_MODE_FSAA;
}

ConfigParams& ConfigParams::operator = ( const ConfigParams& rhs )
{
    if( this == &rhs )
        return *this;
 
    appName       = rhs.appName;
    compoundModes = rhs.compoundModes;
    return *this;
}
