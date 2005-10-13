
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "compound.h"

#include "channel.h"

using namespace eqs;


Compound::Compound(const Compound& from)
        : _channel( NULL )
{
    const uint nCompounds = from.nCompounds();
    for( uint i=0; i<nCompounds; i++ )
    {
        Compound* compound = from.getCompound(i);
        _compounds.push_back( new Compound( *compound ));
    }
}

void Compound::init()
{
    const uint nCompounds = this->nCompounds();
    for( uint i=0; i<nCompounds; i++ )
    {
        Compound* child = getCompound(i);
        child->init();
    }

    Channel* channel = getChannel();
    if( channel )
        channel->refUsed();
}
