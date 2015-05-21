
/* Copyright (c) 2011-2013, Stefan Eilemann <eile@eyescale.ch>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
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

#include "objectMap.h"

#include <eq/config.h>
#include <co/dataIStream.h>
#include <co/dataOStream.h>

namespace seq
{
namespace detail
{

ObjectMap::ObjectMap( eq::Config& config, co::ObjectFactory &factory )
        : co::ObjectMap( config, factory )
{}

ObjectMap::~ObjectMap()
{
}

void ObjectMap::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    co::ObjectMap::serialize( os, dirtyBits );

    if( dirtyBits & DIRTY_INITDATA )
        os << _initData;
    if( dirtyBits & DIRTY_FRAMEDATA )
        os << _frameData;
}

void ObjectMap::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
{
    co::ObjectMap::deserialize( is, dirtyBits );

    if( dirtyBits & DIRTY_INITDATA )
        is >> _initData;
    if( dirtyBits & DIRTY_FRAMEDATA )
        is >> _frameData;
}

void ObjectMap::setInitData( co::Object* object )
{
    const uint128_t identifier = object ? object->getID() : uint128_t();
    if( _initData == identifier )
        return;

    _initData = identifier;
    setDirty( DIRTY_INITDATA );
}

void ObjectMap::setFrameData( co::Object* object )
{
    const uint128_t identifier = object ? object->getID() : uint128_t();
    if( _frameData == identifier )
        return;

    _frameData = identifier;
    setDirty( DIRTY_FRAMEDATA );
}

}
}
