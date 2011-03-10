
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <eq/defines.h>
#include <eq/util/types.h>
#include <eq/fabric/types.h>
#include <co/base/refPtr.h>
#include <co/base/uuid.h>

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
class GLXEventHandler;
class GLXWindow;
class GLXWindowEvent;
class GLXWindowIF;
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
class X11Connection;
struct ConfigEvent;
struct PixelData;
struct Statistic;
struct Event;

typedef fabric::ColorMask ColorMask;
typedef fabric::DrawableConfig DrawableConfig;
typedef fabric::Frustum Frustum;
typedef fabric::Pixel Pixel;
typedef fabric::PixelViewport PixelViewport;
typedef fabric::Projection Projection;
typedef fabric::Range Range;
typedef fabric::RenderContext RenderContext;
typedef fabric::SubPixel SubPixel;
typedef fabric::Viewport Viewport;
typedef fabric::Wall Wall;
typedef fabric::Zoom Zoom;

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


//----- Vectors
/** A vector of pointers to eq::Config */
typedef std::vector< Config* > Configs;
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

/** A reference-counted pointer to an eq::Client */
typedef co::base::RefPtr< Client >        ClientPtr;
/** A reference-counted pointer to a const eq::Client */
typedef co::base::RefPtr< const Client >  ConstClientPtr;
/** A reference-counted pointer to an eq::Server */
typedef co::base::RefPtr< Server >        ServerPtr;

/** The OpenGL object manager used in the client library. */
typedef util::ObjectManager< const void* > ObjectManager;

typedef fabric::Matrix3d Matrix3d;   //!< A 3x3 double matrix
typedef fabric::Matrix4d Matrix4d;   //!< A 4x4 double matrix
typedef fabric::Matrix3f Matrix3f;   //!< A 3x3 float matrix
typedef fabric::Matrix4f Matrix4f;   //!< A 4x4 float matrix
typedef fabric::Vector2i Vector2i;   //!< A two-component integer vector
typedef fabric::Vector3i Vector3i;   //!< A three-component integer vector
typedef fabric::Vector4i Vector4i;   //!< A four-component integer vector
typedef fabric::Vector3d Vector3d;   //!< A three-component double vector
typedef fabric::Vector4d Vector4d;   //!< A four-component double vector
typedef fabric::Vector2f Vector2f;   //!< A two-component float vector
typedef fabric::Vector3f Vector3f;   //!< A three-component float vector
typedef fabric::Vector4f Vector4f;   //!< A four-component float vector
typedef fabric::Vector3ub Vector3ub; //!< A three-component byte vector
typedef fabric::Frustumf Frustumf;   //!< A frustum definition

/** Frustum culling helper */
typedef vmml::frustum_culler< float >  FrustumCullerf;

/** A vector of std::string */
typedef std::vector< std::string >   Strings;
/** A vector of bytes */
typedef std::vector< uint8_t >    Vectorub;
/** A vector of unsigned shorts */
typedef std::vector< uint16_t >   Vectorus;

typedef co::base::uint128_t uint128_t;
typedef co::base::UUID UUID;

/** @cond IGNORE */
typedef co::base::RefPtr< X11Connection > X11ConnectionPtr;
    
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
