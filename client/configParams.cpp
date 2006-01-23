
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "configParams.h"

#include <eq/net/global.h>

using namespace eq;
using namespace std;

ConfigParams::ConfigParams()
{
    renderClient = eqNet::Global::getProgramName();
//     compoundModes = 
//         COMPOUND_MODE_2D    | 
//         COMPOUND_MODE_DPLEX |
//         COMPOUND_MODE_EYE   |
//         COMPOUND_MODE_FSAA;
}

ConfigParams& ConfigParams::operator = ( const ConfigParams& rhs )
{
    if( this == &rhs )
        return *this;
 
    renderClient  = rhs.renderClient;
    compoundModes = rhs.compoundModes;
    return *this;
}
