
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "compound.h"

using namespace eqs;


Compound::Compound(const Compound& from)
{
    const uint nCompounds = from.nCompounds();
    for( uint i=0; i<nCompounds; i++ )
    {
        Compound* compound = from.getCompound(i);
        _compounds.push_back( new Compound( *compound ));
    }
}
