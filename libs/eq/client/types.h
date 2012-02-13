
/* Copyright (c) 2007-2012, Stefan Eilemann <eile@equalizergraphics.com>
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

#ifndef EQ_TYPES_H
#define EQ_TYPES_H

#include <eq/client/defines.h>
#include <eq/util/types.h>
#include <eq/fabric/focusMode.h>
#include <eq/fabric/queuePackets.h>
#include <eq/fabric/types.h>
#include <co/base/atomic.h>

#include <map>
#include <vector>

namespace eq
{
class Canvas;
class Channel;
class Client;
class CommandQueue;
class ComputeContext;
class Config;
class ConfigParams;
class Frame;
class FrameData;
class Image;
class Layout;
class MessagePump;
class Node;
class NodeFactory;
class Observer;
class Pipe;
class Segment;
class Server;
class SystemPipe;
class SystemWindow;
class View;
class Window;
struct ConfigEvent;
struct PixelData;
struct Statistic;
struct Event;

using fabric::ANAGLYPH;
using fabric::ASYNC;
using fabric::AUTO;
using fabric::DRAW_SYNC;
using fabric::FASTEST;
using fabric::FBO;
using fabric::HORIZONTAL;
using fabric::LOCAL_SYNC;
using fabric::NICEST;
using fabric::OFF;
using fabric::ON;
using fabric::PBUFFER;
using fabric::QUAD;
using fabric::RGBA16F;
using fabric::RGBA32F;
using fabric::UNDEFINED;
using fabric::VERTICAL;
using fabric::WINDOW;

using fabric::ColorMask;
using fabric::DrawableConfig;
using fabric::Frustum;
using fabric::Frustumf;
using fabric::GPUInfo;
using fabric::Pixel;
using fabric::PixelViewport;
using fabric::Projection;
using fabric::Range;
using fabric::RenderContext;
using fabric::SubPixel;
using fabric::TileTaskPacket;
using fabric::Viewport;
using fabric::Wall;
using fabric::Zoom;

/** A visitor to traverse segments. @sa  Segment::accept() */
typedef fabric::LeafVisitor< Segment > SegmentVisitor;

/** A visitor to traverse views. @sa View::accept() */
typedef fabric::LeafVisitor< View > ViewVisitor;

/** A visitor to traverse channels. @sa Channel::accept() */
typedef fabric::LeafVisitor< Observer > ObserverVisitor;

/** A visitor to traverse channels. @sa Channel::accept() */
typedef fabric::LeafVisitor< Channel > ChannelVisitor;

/** A visitor to traverse canvases and children. */
typedef fabric::ElementVisitor< Canvas, SegmentVisitor > CanvasVisitor;

/** A visitor to traverse windows and children. */
typedef fabric::ElementVisitor< Window, ChannelVisitor > WindowVisitor;   
    
/** A visitor to traverse pipes and children. */
typedef fabric::ElementVisitor< Pipe, WindowVisitor > PipeVisitor;

/** A visitor to traverse nodes and children. */
typedef fabric::ElementVisitor< Node, PipeVisitor > NodeVisitor;

/** A visitor to traverse layouts and children. */
typedef fabric::ElementVisitor< Layout, ViewVisitor > LayoutVisitor;

/** A visitor to traverse configs and children. */
typedef fabric::ConfigVisitor< Config, ObserverVisitor, LayoutVisitor,
                               CanvasVisitor, NodeVisitor > ConfigVisitor;

/** A visitor to traverse servers and children. */
typedef fabric::ElementVisitor< Server, ConfigVisitor > ServerVisitor;

//----- Vectors
/** A vector of pointers to eq::Config */
typedef std::vector< Config* > Configs;
/** A vector of pointers to eq::Server */
typedef std::vector< Server* > Servers;
/** A vector of pointers to eq::Node */
typedef std::vector< Node* > Nodes;
/** A vector of pointers to eq::Pipe */
typedef std::vector< Pipe* > Pipes;
/** A vector of pointers to eq::Window */
typedef std::vector< Window* > Windows;
/** A vector of pointers to eq::Channel */
typedef std::vector< Channel* > Channels;
/** A vector of pointers to eq::Frame */
typedef std::vector< Frame* > Frames;
/** A vector of pointers to eq::Image */
typedef std::vector< Image* > Images;
/** A vector of pointers to eq::Observer */
typedef std::vector< Observer* > Observers;
/** A vector of pointers to eq::Canvas */
typedef std::vector< Canvas* > Canvases;
/** A vector of pointers to eq::Layout */
typedef std::vector< Layout* > Layouts;
/** A vector of pointers to eq::Segment */
typedef std::vector< Segment* > Segments;
/** A vector of pointers to eq::View */
typedef std::vector< View* > Views;
/** A vector of eq::Viewport */
typedef std::vector< Viewport > Viewports;
/** A vector of eq::PixelViewport */
typedef std::vector< PixelViewport > PixelViewports;
/** A vector of eq::Statistic events */
typedef std::vector< Statistic > Statistics;

/** A const_iterator over a eq::Config vector */
typedef Configs::const_iterator ConfigsCIter;
/** A const_iterator over a eq::Server vector */
typedef Servers::const_iterator ServersCIter;
/** A const_iterator over a eq::Node vector */
typedef Nodes::const_iterator NodesCIter;
/** A const_iterator over a eq::Pipe vector */
typedef Pipes::const_iterator PipesCIter;
/** A const_iterator over a eq::Window vector */
typedef Windows::const_iterator WindowsCIter;
/** A const_iterator over a eq::Channel vector */
typedef Channels::const_iterator ChannelsCIter;
/** An iterator over a eq::Frame vector */
typedef Frames::iterator FramesIter;
/** A const_iterator over a eq::Frame vector */
typedef Frames::const_iterator FramesCIter;
/** A const_iterator over a eq::Image vector */
typedef Images::const_iterator ImagesCIter;
/** A const_iterator over a eq::Observer vector */
typedef Observers::const_iterator ObserversCIter;
/** A const_iterator over a eq::Canvas vector */
typedef Canvases::const_iterator CanvasesCIter;
/** A const_iterator over a eq::Layout vector */
typedef Layouts::const_iterator LayoutsCIter;
/** A const_iterator over a eq::Segment vector */
typedef Segments::const_iterator SegmentsCIter;
/** A const_iterator over a eq::View vector */
typedef Views::const_iterator ViewsCIter;
/** A const_iterator over a eq::Viewport vector */
typedef Viewports::const_iterator ViewportsCIter;
/** A const_iterator over a eq::PixelViewport vector */
typedef PixelViewports::const_iterator PixelViewportsCIter;
/** A const_iterator over a eq::Statistic events vector */
typedef Statistics::const_iterator StatisticsCIter;

/** A reference-counted pointer to an eq::Client */
typedef co::base::RefPtr< Client >        ClientPtr;
/** A reference-counted pointer to a const eq::Client */
typedef co::base::RefPtr< const Client >  ConstClientPtr;
/** A reference-counted pointer to an eq::Server */
typedef co::base::RefPtr< Server >        ServerPtr;

namespace util
{
template< class > class BitmapFont;
template< class > class ObjectManager;
}

/** The bitmap font used in the client library. */
typedef util::BitmapFont< const void* > BitmapFont;

/** The OpenGL object manager used in the client library. */
typedef util::ObjectManager< const void* > ObjectManager;

using fabric::Matrix3d;   //!< A 3x3 double matrix
using fabric::Matrix4d;   //!< A 4x4 double matrix
using fabric::Matrix3f;   //!< A 3x3 float matrix
using fabric::Matrix4f;   //!< A 4x4 float matrix
using fabric::Vector2i;   //!< A two-component integer vector
using fabric::Vector3i;   //!< A three-component integer vector
using fabric::Vector4i;   //!< A four-component integer vector
using fabric::Vector3d;   //!< A three-component double vector
using fabric::Vector4d;   //!< A four-component double vector
using fabric::Vector2f;   //!< A two-component float vector
using fabric::Vector3f;   //!< A three-component float vector
using fabric::Vector4f;   //!< A four-component float vector
using fabric::Vector3ub;  //!< A three-component byte vector
using fabric::Frustumf;   //!< A frustum definition

using fabric::FocusMode;
using fabric::FOCUSMODE_FIXED;
using fabric::FOCUSMODE_RELATIVE_TO_ORIGIN;
using fabric::FOCUSMODE_RELATIVE_TO_OBSERVER;

/** Frustum culling helper */
typedef vmml::frustum_culler< float >  FrustumCullerf;

/** A vector of std::strings */
typedef std::vector< std::string > Strings;
/** A vector of bytes */
typedef std::vector< uint8_t >    Vectorub;
/** A vector of unsigned shorts */
typedef std::vector< uint16_t >   Vectorus;

/** A const_iterator over a std::string vector */
typedef Strings::const_iterator StringsCIter;

using co::base::a_int32_t;
using co::base::uint128_t;
using co::base::UUID;

/** @cond IGNORE */
typedef co::WorkerThread< CommandQueue > Worker; // instantiated in worker.cpp

// originator serial -> statistics
typedef std::map< uint32_t, Statistics > SortedStatistics;

// frame id, config statistics
typedef std::pair< uint32_t, SortedStatistics > FrameStatistics;
/** @endcond */
}

/** @cond IGNORE */
// GLEW
struct GLEWContextStruct;
struct WGLEWContextStruct;
struct GLXEWContextStruct;
typedef struct GLEWContextStruct GLEWContext;
typedef struct WGLEWContextStruct WGLEWContext;
typedef struct GLXEWContextStruct GLXEWContext;
/** @endcond */

#endif // EQ_TYPES_H
