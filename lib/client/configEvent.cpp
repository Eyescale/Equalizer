/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "configEvent.h"

using namespace eq;

std::ostream& eq::operator << ( std::ostream& os, const ConfigEvent* event )
{
    os << "config event " << (ConfigPacket*)event << ", type "
       << event->type;
    return os;
}
