
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "openParams.h"

#include <eq/client/global.h>
#include <eq/net/global.h>

using namespace eq;
using namespace std;

OpenParams::OpenParams()
        : address( Global::getServer( )),
          appName( eqNet::Global::getProgramName( ))
{
}

OpenParams& OpenParams::operator = ( const OpenParams& rhs )
{
    if( this == &rhs )
        return *this;
 
    address = rhs.address;
    appName = rhs.appName;
    return *this;
}
