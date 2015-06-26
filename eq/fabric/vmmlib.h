
/* Copyright (c) 2011-2014, Stefan Eilemann <eile@eyescale.ch>
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

#define VMMLIB_CUSTOM_CONFIG
#ifndef NDEBUG
#  define VMMLIB_SAFE_ACCESSORS
#endif
#define VMMLIB_ALIGN( var ) var

#pragma warning(push)
#pragma warning(disable : 4996)
#  include <vmmlib/vmmlib.hpp>
#pragma warning(pop)

#include <lunchbox/bitOperation.h>

namespace eq
{
namespace fabric
{
using vmml::Matrix3d; //!< A 3x3 double matrix
using vmml::Matrix4d; //!< A 4x4 double matrix
using vmml::Matrix3f; //!< A 3x3 float matrix
using vmml::Matrix4f; //!< A 4x4 float matrix
using vmml::Vector2ui; //!< A two-component unsigned integer vector
using vmml::Vector2i; //!< A two-component integer vector
using vmml::Vector3ui; //!< A three-component unsigned integer vector
using vmml::Vector3i; //!< A three-component integer vector
using vmml::Vector4ui; //!< A four-component unsigned integer vector
using vmml::Vector4i; //!< A four-component integer vector
using vmml::Vector3d; //!< A three-component double vector
using vmml::Vector4d; //!< A four-component double vector
using vmml::Vector2f; //!< A two-component float vector
using vmml::Vector3f; //!< A three-component float vector
using vmml::Vector4f; //!< A four-component float vector
using vmml::Vector3ub; //!< A three-component byte vector
using vmml::Vector4ub; //!< A four-component byte vector
using vmml::Frustumf; //!< A frustum definition
using vmml::AABBf; //!< axis-aligned bounding box
}
}

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
    byteswap( value.near_plane( ));
    byteswap( value.far_plane( ));
}

template<> inline void byteswap( eq::fabric::AABBf& value )
{
    byteswap( value.getMin( ));
    byteswap( value.getMax( ));
}
}

#endif // EQFABRIC_VMMLIB_H
