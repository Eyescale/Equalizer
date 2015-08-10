
/* Copyright (c) 2007-2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQSERVER_TYPES_H
#define EQSERVER_TYPES_H

#include <eq/fabric/eye.h>
#include <eq/fabric/focusMode.h>
#include <eq/fabric/types.h>
#include <lunchbox/refPtr.h>
#include <vector>

namespace eq
{
namespace server
{

class Canvas;
class Channel;
class ChannelListener;
class Compound;
class CompoundListener;
class CompoundVisitor;
class Config;
class ConfigVisitor;
class DFREqualizer;
class Equalizer;
class Frame;
class FrameData;
class FramerateEqualizer;
class Layout;
class LoadEqualizer;
class MonitorEqualizer;
class Node;
class NodeFactory;
class Observer;
class Pipe;
class Segment;
class Server;
class TileEqualizer;
class TileQueue;
class TreeEqualizer;
class View;
class ViewEqualizer;
class Window;

typedef std::vector< Config* >   Configs;
typedef std::vector< Node* >     Nodes;
typedef std::vector< Pipe* >     Pipes;
typedef std::vector< Window* >   Windows;
typedef std::vector< Channel* >  Channels;

typedef std::vector< Canvas* >       Canvases;
typedef std::vector< Compound* >     Compounds;
typedef std::vector< Frame* >        Frames;
typedef std::vector< TileQueue* >    TileQueues;
typedef std::vector< Layout* >       Layouts;
typedef std::vector< Equalizer* >    Equalizers;
typedef std::vector< Observer* >     Observers;
typedef std::vector< Segment* >      Segments;
typedef std::vector< View* >         Views;

using lunchbox::uint128_t;
using lunchbox::uint128_t;
using lunchbox::Strings;
using lunchbox::StringsCIter;

typedef Configs::const_iterator ConfigsCIter;
typedef Configs::iterator ConfigsIter;
typedef Canvases::const_iterator CanvasesCIter;
typedef Canvases::iterator CanvasesIter;
typedef Channels::const_iterator ChannelsCIter;
typedef Channels::iterator ChannelsIter;
typedef Compounds::const_iterator CompoundsCIter;
typedef Compounds::iterator CompoundsIter;
typedef Frames::const_iterator FramesCIter;
typedef Frames::iterator FramesIter;
typedef Layouts::const_iterator LayoutsCIter;
typedef Layouts::iterator LayoutsIter;
typedef Nodes::const_iterator NodesCIter;
typedef Nodes::iterator NodesIter;
typedef Observers::const_iterator ObserversCIter;
typedef Observers::iterator ObserversIter;
typedef Pipes::const_iterator PipesCIter;
typedef Pipes::iterator PipesIter;
typedef TileQueues::const_iterator TileQueuesCIter;
typedef TileQueues::iterator TileQueuesIter;
typedef Views::const_iterator ViewsCIter;
typedef Views::iterator ViewsIter;
typedef Equalizers::const_iterator EqualizersCIter;
typedef Equalizers::iterator EqualizersIter;
typedef Windows::const_iterator WindowsCIter;
typedef Windows::iterator WindowsIter;

typedef lunchbox::RefPtr< Server > ServerPtr;
typedef lunchbox::RefPtr< const Server > ConstServerPtr;

using fabric::ConfigParams;
using fabric::DrawableConfig;
using fabric::Error;
using fabric::Event;
using fabric::EventOCommand;
using fabric::Frustumf;
using fabric::Matrix4f;
using fabric::Pixel;
using fabric::PixelViewport;
using fabric::Projection;
using fabric::Range;
using fabric::RenderContext;
using fabric::Statistic;
using fabric::Statistics;
using fabric::SubPixel;
using fabric::SwapBarrier;
using fabric::SwapBarrierConstPtr;
using fabric::SwapBarrierPtr;
using fabric::Tile;
using fabric::Vector2i;
using fabric::Vector3f;
using fabric::Vector3ub;
using fabric::Vector4f;
using fabric::Vector4i;
using fabric::Viewport;
using fabric::Viewports;
using fabric::Wall;
using fabric::WindowSettings;
using fabric::Zoom;

using fabric::NodePath;
using fabric::PipePath;
using fabric::WindowPath;
using fabric::ChannelPath;
using fabric::CanvasPath;
using fabric::SegmentPath;
using fabric::ObserverPath;
using fabric::LayoutPath;
using fabric::ViewPath;

using fabric::CORE;
using fabric::SOCKET;
using fabric::EYES_STEREO;
using fabric::UNDEFINED;

/** A visitor to traverse segments. @sa Segment::accept() */
typedef fabric::LeafVisitor< Segment > SegmentVisitor;

/** A visitor to traverse views. @sa View::accept() */
typedef fabric::LeafVisitor< View > ViewVisitor;

/** A visitor to traverse layouts and children. */
typedef fabric::ElementVisitor< Layout, ViewVisitor > LayoutVisitor;

/** A visitor to traverse observers. @sa Observer::accept() */
typedef fabric::LeafVisitor< Observer > ObserverVisitor;

/** A visitor to traverse channels. @sa Channel::accept() */
typedef fabric::LeafVisitor< Channel > ChannelVisitor;

/** A visitor to traverse Canvas and children. */
typedef fabric::ElementVisitor< Canvas, SegmentVisitor > CanvasVisitor;

/** A visitor to traverse windows and children. */
typedef fabric::ElementVisitor< Window, ChannelVisitor > WindowVisitor;

/** A visitor to traverse pipes and children. */
typedef fabric::ElementVisitor< Pipe, WindowVisitor > PipeVisitor;

/** A visitor to traverse nodes and children. */
typedef fabric::ElementVisitor< Node, PipeVisitor > NodeVisitor;

/** A visitor to traverse layouts and children. */
typedef fabric::ElementVisitor< Layout, ViewVisitor > LayoutVisitor;

/** A visitor to traverse servers and children. */
typedef fabric::ElementVisitor< Server, ConfigVisitor > ServerVisitor;

using fabric::Eye;
using fabric::EYE_UNDEFINED;
using fabric::EYE_CYCLOP;
using fabric::EYE_LEFT;
using fabric::EYE_RIGHT;
using fabric::EYES_STEREO;
using fabric::EYES_ALL;
using fabric::NUM_EYES;

using fabric::FocusMode;
using fabric::FOCUSMODE_FIXED;
using fabric::FOCUSMODE_RELATIVE_TO_ORIGIN;
using fabric::FOCUSMODE_RELATIVE_TO_OBSERVER;
}
}
#endif // EQSERVER_TYPES_H
