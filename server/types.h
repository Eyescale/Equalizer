
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
#include <eq/base/refPtr.h>
#include <eq/base/uuid.h>
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

class Config;
class Node;
class Pipe;
class Window;
class Channel;
class Server;
class Canvas;
class Compound;
class ConnectionDescription;
class Frame;
class Layout;
class Equalizer;
class Observer;
class Segment;
class View;

typedef std::vector< Config* >   ConfigVector;
typedef std::vector< Node* >     NodeVector;
typedef std::vector< Pipe* >     PipeVector;
typedef std::vector< Window* >   WindowVector;
typedef std::vector< Channel* >  ChannelVector;

typedef std::vector< Canvas* >       CanvasVector;
typedef std::vector< Compound* >     CompoundVector;
typedef std::vector< Frame* >        FrameVector;
typedef std::vector< Layout* >       LayoutVector;
typedef std::vector< Equalizer* >    EqualizerVector;
typedef std::vector< Observer* >     ObserverVector;
typedef std::vector< Segment* >      SegmentVector;
typedef std::vector< View* >         ViewVector;

typedef base::RefPtr< Server > ServerPtr;
typedef base::RefPtr< ConnectionDescription >   ConnectionDescriptionPtr;
typedef std::vector< ConnectionDescriptionPtr > ConnectionDescriptionVector;

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

/** A visitor to traverse segments. @sa  Segment ::accept() */
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

}
}
#endif // EQSERVER_TYPES_H
