
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <eq/fabric/types.h>
#include <co/base/refPtr.h>
#include <co/base/uuid.h>
#include <vector>

namespace eq
{
namespace fabric
{
class Projection;
class RenderContext;
class Wall;

struct NodePath;
struct PipePath;
struct WindowPath;
struct ChannelPath;
struct CanvasPath;
struct SegmentPath;
struct ObserverPath;
struct LayoutPath;
struct ViewPath;

template< typename T, typename L > class ElementVisitor;
template< typename T > class LeafVisitor;
template< typename T, typename C  > class ElementVisitor;
}

namespace server
{

class Canvas;
class Channel;
class Compound;
class Config;
class Equalizer;
class Frame;
class Layout;
class Node;
class NodeFactory;
class Observer;
class Pipe;
class Segment;
class Server;
class View;
class Window;

typedef std::vector< Config* >   Configs;
typedef std::vector< Node* >     Nodes;
typedef std::vector< Pipe* >     Pipes;
typedef std::vector< Window* >   Windows;
typedef std::vector< Channel* >  Channels;

typedef std::vector< Canvas* >       Canvases;
typedef std::vector< Compound* >     Compounds;
typedef std::vector< Frame* >        Frames;
typedef std::vector< Layout* >       Layouts;
typedef std::vector< Equalizer* >    Equalizers;
typedef std::vector< Observer* >     Observers;
typedef std::vector< Segment* >      Segments;
typedef std::vector< View* >         Views;

typedef co::base::uint128_t uint128_t;
typedef co::base::UUID UUID;

typedef Compounds::iterator CompoundsIter;
typedef Compounds::const_iterator CompoundsCIter;
typedef Observers::iterator ObserversIter;
typedef Observers::const_iterator ObserversCIter;
typedef Canvases::iterator CanvasesIter;
typedef Canvases::const_iterator CanvasesCIter;

typedef co::base::RefPtr< Server > ServerPtr;
typedef co::base::RefPtr< const Server > ConstServerPtr;

typedef fabric::Vector4i Vector4i;   //!< A four-component integer vector
typedef fabric::Vector3f Vector3f;   //!< A three-component float vector
typedef fabric::Vector3ub Vector3ub; //!< A three-component byte vector
typedef fabric::Matrix4f Matrix4f;   //!< A 4x4 float matrix
typedef fabric::Frustumf Frustumf;   //!< A frustum definition
typedef fabric::Projection Projection;
typedef fabric::RenderContext RenderContext;
typedef fabric::Wall Wall;

typedef fabric::NodePath NodePath;
typedef fabric::PipePath PipePath;
typedef fabric::WindowPath WindowPath;
typedef fabric::ChannelPath ChannelPath;
typedef fabric::CanvasPath CanvasPath;
typedef fabric::SegmentPath SegmentPath;
typedef fabric::ObserverPath ObserverPath;
typedef fabric::LayoutPath LayoutPath;
typedef fabric::ViewPath ViewPath;

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

class ConfigVisitor;
class ServerVisitor;

}
}
#endif // EQSERVER_TYPES_H
