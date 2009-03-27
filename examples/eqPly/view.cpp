
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

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
