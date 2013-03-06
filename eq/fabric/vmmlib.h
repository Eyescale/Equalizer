
/* Copyright (c) 2011-2012, Stefan Eilemann <eile@eyescale.ch>
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

namespace eq
{
namespace fabric
{
typedef vmml::matrix< 3, 3, double > Matrix3d; //!< A 3x3 double matrix
typedef vmml::matrix< 4, 4, double > Matrix4d; //!< A 4x4 double matrix
typedef vmml::matrix< 3, 3, float >  Matrix3f; //!< A 3x3 float matrix
typedef vmml::matrix< 4, 4, float >  Matrix4f; //!< A 4x4 float matrix
typedef vmml::vector< 2, int > Vector2i; //!< A two-component integer vector
typedef vmml::vector< 3, int > Vector3i; //!< A three-component integer vector
typedef vmml::vector< 4, int > Vector4i; //!< A four-component integer vector
typedef vmml::vector< 3, double >Vector3d; //!< A three-component double vector
typedef vmml::vector< 4, double >Vector4d; //!< A four-component double vector
typedef vmml::vector< 2, float > Vector2f; //!< A two-component float vector
typedef vmml::vector< 3, float > Vector3f; //!< A three-component float vector
typedef vmml::vector< 4, float > Vector4f; //!< A four-component float vector
typedef vmml::vector< 3, uint8_t > Vector3ub; //!< A three-component byte vector
typedef vmml::vector< 4, uint8_t > Vector4ub; //!< A four-component byte vector
typedef vmml::frustum< float >  Frustumf; //!< A frustum definition
}
}

namespace lunchbox
{
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

template<> inline void byteswap( eq::fabric::Vector4i& value )
{
    byteswap( value.x( ));
    byteswap( value.y( ));
    byteswap( value.z( ));
    byteswap( value.w( ));
}

template<> inline void byteswap( eq::fabric::Vector4ub& value ) { /*NOP*/ }
template<> inline void byteswap( eq::fabric::Vector3ub& value ) { /*NOP*/ }

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
}

#endif // EQFABRIC_VMMLIB_H
