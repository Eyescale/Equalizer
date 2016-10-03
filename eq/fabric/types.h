
/* Copyright (c) 2007-2016, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
 *                          Cedric Stalder <cedric.stalder@gmail.com>
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

#include <eq/fabric/api.h>
#include <eq/fabric/errorCodes.h>
#include <eq/fabric/eventEnums.h>
#include <eq/fabric/eventType.h>
#include <co/types.h>
#include <lunchbox/refPtr.h>
#include <lunchbox/uint128_t.h>
#include <lunchbox/visitorResult.h>
#include <vmmlib/types.hpp>

#ifdef _WIN32
#  define EQ_DEFAULT_PORT (4242)
#else
// #241: Avoid using privilege ports below 1024
#  define EQ_DEFAULT_PORT (( getuid() % 64511 ) + 1024 )
#endif

namespace eq
{
namespace fabric
{
using namespace eventEnums;
using namespace eventTypes;
class Client;
class ColorMask;
class ConfigParams;
class Equalizer;
class Error;
class ErrorRegistry;
class FrameData;
class Frustum;
class Pixel;
class PixelViewport;
class Projection;
class Range;
class RenderContext;
class SubPixel;
class SwapBarrier;
class Tile;
class Viewport;
class Wall;
class WindowSettings;
class Zoom;
struct AxisEvent;
struct ButtonEvent;
struct CanvasPath;
struct ChannelPath;
struct DrawableConfig;
struct Event;
struct GPUInfo;
struct KeyEvent;
struct LayoutPath;
struct NodePath;
struct ObserverPath;
struct PipePath;
struct PointerEvent;
struct SegmentPath;
struct SizeEvent;
struct Statistic;
struct ViewPath;
struct WindowPath;

template< class, class > class Channel;
template< class, class > class Observer;
template< class, class, class > class Layout;
template< class, class, class > class Segment;
template< class, class, class > class View;
template< class, class, class, class > class Window;
template< class, class, class, class > class Canvas;
template< class, class, class, class > class Node;
template< class, class, class, class > class Pipe;
template< class, class, class, class, class, class > class Server;
template< class, class, class, class, class, class, class > class Config;

template< class > class LeafVisitor;
template< class, class > class ElementVisitor;
template< class, class, class, class, class> class ConfigVisitor;

/** A vector of eq::fabric::Error */
typedef std::vector< Error > Errors;
/** A vector of eq::Statistic events */
typedef std::vector< Statistic > Statistics;
/** A vector of eq::Viewport */
typedef std::vector< Viewport > Viewports;

using co::Strings;
using co::StringsCIter;
typedef co::ObjectOCommand EventOCommand;

using lunchbox::uint128_t;
using lunchbox::VisitorResult;
using lunchbox::TRAVERSE_CONTINUE;
using lunchbox::TRAVERSE_PRUNE;
using lunchbox::TRAVERSE_TERMINATE;
typedef lunchbox::RefPtr< Client > ClientPtr;
typedef lunchbox::RefPtr< const Client > ConstClientPtr;
typedef lunchbox::RefPtr< SwapBarrier > SwapBarrierPtr;
typedef lunchbox::RefPtr< const SwapBarrier > SwapBarrierConstPtr;

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
using vmml::Quaternionf; // !< A float quaternion
}
}

#endif // EQFABRIC_TYPES_H
