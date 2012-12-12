
/* Copyright (c) 2007-2012, Stefan Eilemann <eile@equalizergraphics.com>
 *               2011-2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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
#include <eq/fabric/error.h>
#include <eq/fabric/vmmlib.h>
#include <co/types.h>
#include <lunchbox/refPtr.h>
#include <lunchbox/uint128_t.h>

namespace eq
{
namespace fabric
{
class ColorMask;
class ConfigParams;
class Equalizer;
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
class Zoom;
struct DrawableConfig;
struct GPUInfo;

using lunchbox::uint128_t;
using lunchbox::UUID;

class Client;
template< class, class > class Channel;
template< class, class > class Observer;
template< class, class, class > class Layout;
template< class, class, class > class Segment;
template< class, class, class > class View;
template< class, class, class > class Window;
template< class, class, class, class > class Canvas;
template< class, class, class, class > class Node;
template< class, class, class, class > class Pipe;
template< class, class, class, class, class, class > class Server;
template< class, class, class, class, class, class, class > class Config;
template< class, class > class ElementVisitor;
template< class > class LeafVisitor;
template< class, class, class, class, class> class ConfigVisitor;

typedef lunchbox::RefPtr< Client > ClientPtr;
typedef lunchbox::RefPtr< const Client > ConstClientPtr;
typedef lunchbox::RefPtr< SwapBarrier > SwapBarrierPtr;
typedef lunchbox::RefPtr< const SwapBarrier > SwapBarrierConstPtr;

struct CanvasPath;
struct ChannelPath;
struct LayoutPath;
struct NodePath;
struct ObserverPath;
struct PipePath;
struct SegmentPath;
struct ViewPath;
struct WindowPath;

typedef std::vector< std::string > Strings;
typedef Strings::const_iterator StringsCIter;

#ifndef EQ_2_0_API
using co::Serializable;
#endif
}
}

#ifndef EQ_2_0_API
namespace co
{
using eq::fabric::Error;
using eq::fabric::ErrorRegistry;
using eq::fabric::ERROR_NONE;
}
#endif

#endif // EQFABRIC_TYPES_H
