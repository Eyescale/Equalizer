
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

#ifndef EQ_TYPES_H
#define EQ_TYPES_H

#include <eq/fabric/types.h>
#include <eq/base/refPtr.h>

#include <map>
#include <vector>

namespace eq
{
namespace fabric
{
class ColorMask;
struct DrawableConfig;
class Frustum;
class Pixel;
class PixelViewport;
class Projection;
class Range;
class RenderContext;
class SubPixel;
class Viewport;
class Wall;
class Zoom;
template< class C, class OV, class LV, class CV, class NV > class ConfigVisitor;
template< class T, class C > class ElementVisitor;
template< class T > class LeafVisitor;
}

class Canvas;
class Channel;
class Client;
class Config;
class Frame;
class Image;
class Layout;
class Node;
class Observer;
class Pipe;
class Segment;
class Server;
class View;
class Window;
class X11Connection;
struct Statistic;

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

/** A visitor to traverse segments. @sa  Segment ::accept() */
typedef fabric::LeafVisitor< Segment > SegmentVisitor;

/** A visitor to traverse views. @sa View::accept() */
typedef fabric::LeafVisitor< View > ViewVisitor;

/** A visitor to traverse channels. @sa Channel::accept() */
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

/** A visitor to traverse configs and children. */
typedef fabric::ConfigVisitor< Config, ObserverVisitor, LayoutVisitor,
                               CanvasVisitor, NodeVisitor > ConfigVisitor;


//----- Vectors
/** A vector of pointers to eq::Config */
typedef std::vector< Config* >     Configs;
/** A vector of pointers to eq::Node */
typedef std::vector< Node* >     Nodes;
/** A vector of pointers to eq::Pipe */
typedef std::vector< Pipe* >     Pipes;
/** A vector of pointers to eq::Window */
typedef std::vector< Window* >   Windows;
/** A vector of pointers to eq::Channel */
typedef std::vector< Channel* >  Channels;
/** A vector of pointers to eq::Frame */
typedef std::vector< Frame* >    Frames;
/** A vector of pointers to eq::Image */
typedef std::vector< Image* >    Images;
/** A vector of pointers to eq::Observer */
typedef std::vector< Observer* > Observers;
/** A vector of pointers to eq::Canvas */
typedef std::vector< Canvas* >   Canvases;
/** A vector of pointers to eq::Layout */
typedef std::vector< Layout* >   Layouts;
/** A vector of pointers to eq::Segment */
typedef std::vector< Segment* >  Segments;
/** A vector of pointers to eq::View */
typedef std::vector< View* >     Views;
/** A vector of eq::Viewport */
typedef std::vector< Viewport >      Viewports;
/** A vector of eq::PixelViewport */
typedef std::vector< PixelViewport > PixelViewports;
/** A vector of eq::Statistic events */
typedef std::vector< Statistic >         Statistics;

/** A reference-counted pointer to an eq::Client */
typedef base::RefPtr< Client >        ClientPtr;
/** A reference-counted pointer to a const eq::Client */
typedef base::RefPtr< const Client >  ConstClientPtr;
/** A reference-counted pointer to an eq::Server */
typedef base::RefPtr< Server >        ServerPtr;

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


/** @cond IGNORE */
typedef base::RefPtr< X11Connection > X11ConnectionPtr;
    
// originator id -> statistics
typedef std::map< uint32_t, Statistics >        SortedStatistics;

// frame id, config statistics
typedef std::pair< uint32_t, SortedStatistics > FrameStatistics;
/** @endcond */

#ifdef EQ_USE_DEPRECATED
typedef Configs ConfigVector;
typedef Nodes NodeVector;
typedef Pipes PipeVector;
typedef Windows WindowVector;
typedef Channels ChannelVector;
typedef Frames FrameVector;
typedef Images ImageVector;
typedef Observers ObserverVector;
typedef Canvases CanvasVector;
typedef Layouts LayoutVector;
typedef Segments SegmentVector;
typedef Views ViewVector;
typedef Viewports ViewportVector;
typedef PixelViewports PixelViewportVector;
typedef Statistics StatisticVector;
typedef Strings StringVector
#endif
}
#endif // EQ_TYPES_H
