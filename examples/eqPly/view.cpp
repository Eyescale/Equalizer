
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "view.h"

namespace eqPly
{

View::View()
        : eq::View()
        , _modelID( EQ_ID_INVALID )
{}

View::~View()
{
    _modelID = EQ_ID_INVALID;
}

void View::serialize( eq::net::DataOStream& os, const uint64_t dirtyBits )
{
    eq::View::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_MODEL )
        os << _modelID;
}

void View::deserialize( eq::net::DataIStream& is, const uint64_t dirtyBits )
{
    eq::View::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_MODEL )
        is >> _modelID;
}

void View::setModelID( const uint32_t id )
{
    _modelID = id;
    setDirty( DIRTY_MODEL );
}

}
