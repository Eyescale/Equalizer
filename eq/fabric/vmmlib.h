
/* Copyright (c) 2011-2016, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQFABRIC_VMMLIB_H
#define EQFABRIC_VMMLIB_H

#include <eq/fabric/types.h>
#include <lunchbox/bitOperation.h>

#pragma warning(push)
#pragma warning(disable : 4996)
#  include <vmmlib/aabb.hpp>
#  include <vmmlib/frustum.hpp>
#  include <vmmlib/matrix.hpp>
#  include <vmmlib/vector.hpp>
#pragma warning(pop)

namespace lunchbox
{
template<> inline void byteswap( eq::fabric::Vector2ui& value )
{
    byteswap( value.x( ));
    byteswap( value.y( ));
}

template<> inline void byteswap( eq::fabric::Vector2i& value )
{
    byteswap( value.x( ));
    byteswap( value.y( ));
}

template<> inline void byteswap( eq::fabric::Vector2f& value )
{
    byteswap( value.x( ));
    byteswap( value.y( ));
}

template<> inline void byteswap( eq::fabric::Vector3f& value )
{
    byteswap( value.x( ));
    byteswap( value.y( ));
    byteswap( value.z( ));
}

template<> inline void byteswap( eq::fabric::Vector4f& value )
{
    byteswap( value.x( ));
    byteswap( value.y( ));
    byteswap( value.z( ));
    byteswap( value.w( ));
}

template<> inline void byteswap( eq::fabric::Vector4ui& value )
{
    byteswap( value.x( ));
    byteswap( value.y( ));
    byteswap( value.z( ));
    byteswap( value.w( ));
}

template<> inline void byteswap( eq::fabric::Vector4i& value )
{
    byteswap( value.x( ));
    byteswap( value.y( ));
    byteswap( value.z( ));
    byteswap( value.w( ));
}

template<> inline void byteswap( eq::fabric::Vector4ub& ) { /*NOP*/ }
template<> inline void byteswap( eq::fabric::Vector3ub& ) { /*NOP*/ }

template<> inline void byteswap( eq::fabric::Matrix4f& value )
{
    for( size_t i = 0; i < 16; ++i )
        byteswap( value.array[ i ]);
}

template<> inline void byteswap( eq::fabric::Frustumf& value )
{
    byteswap( value.left( ));
    byteswap( value.right( ));
    byteswap( value.bottom( ));
    byteswap( value.top( ));
    byteswap( value.nearPlane( ));
    byteswap( value.farPlane( ));
}

template<> inline void byteswap( eq::fabric::AABBf& value )
{
    byteswap( value._min );
    byteswap( value._max );
}
}

#endif // EQFABRIC_VMMLIB_H
