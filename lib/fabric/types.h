
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#ifndef EQFABRIC_TYPES_H
#define EQFABRIC_TYPES_H

#include <eq/base/base.h>
#include <eq/base/refPtr.h>
#include <vmmlib/vmmlib.hpp>

namespace eq
{
namespace fabric
{
class PixelViewport;

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
/** A three-component byte vector */
typedef vmml::vector< 3, unsigned char > Vector3ub;
typedef vmml::frustum< float >  Frustumf; //!< A frustum definition

class Client;
template< class, class > class Channel;
template< class, class > class Observer;
template< class, class, class > class Layout;
template< class, class, class > class Segment;
template< class, class, class > class Window;
template< class, class, class, class > class Canvas;
template< class, class, class, class > class Node;
template< class, class, class, class > class Pipe;
template< class, class, class, class > class Server;
template< class, class, class, class, class, class, class > class Config;
template< class, class > class ElementVisitor;
template< class > class LeafVisitor;

typedef base::RefPtr< Client > ClientPtr;
typedef base::RefPtr< const Client > ConstClientPtr;
}
}
#endif // EQFABRIC_TYPES_H
