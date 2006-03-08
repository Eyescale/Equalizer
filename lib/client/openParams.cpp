
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "openParams.h"

#include <eq/net/global.h>

using namespace eq;
using namespace std;

OpenParams::OpenParams()
{
    appName      = eqNet::Global::getProgramName();
}

OpenParams& OpenParams::operator = ( const OpenParams& rhs )
{
    if( this == &rhs )
        return *this;
 
    appName       = rhs.appName;
    return *this;
}
