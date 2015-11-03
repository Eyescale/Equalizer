
/* Copyright (c) 2007-2013, Stefan Eilemann <eile@equalizergraphics.com>
 *               2011-2014, Daniel Nachbaur <danielnachbaur@gmail.com>
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

#include <eq/fabric/api.h>
#include <eq/fabric/errorCodes.h>
#include <eq/fabric/eventEnums.h>
#include <eq/fabric/vmmlib.h>
#include <co/types.h>
#include <lunchbox/refPtr.h>
#include <lunchbox/uint128_t.h>
#include <lunchbox/visitorResult.h>

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
class Client;
class ColorMask;
class ConfigParams;
class Equalizer;
class Error;
class ErrorRegistry;
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
struct CanvasPath;
struct ChannelPath;
struct DrawableConfig;
struct Event;
struct FrameData;
struct GPUInfo;
struct KeyEvent;
struct LayoutPath;
struct NodePath;
struct ResizeEvent;
struct ObserverPath;
struct PipePath;
struct PointerEvent;
struct SegmentPath;
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

}
}

#endif // EQFABRIC_TYPES_H
