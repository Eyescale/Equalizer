
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "statEvent.h"
#include "node.h"

EQ_EXPORT std::string eq::StatEvent::typeNames[TYPE_ALL] = 
{
    std::string( "channel clear     " ),
    std::string( "channel draw      " ),
    std::string( "channel assemble  " ),
    std::string( "channel readback  " ),
    std::string( "channel transmit  " ),
    std::string( "channel transmit n" ),
    std::string( "channel wait frame" )
};
