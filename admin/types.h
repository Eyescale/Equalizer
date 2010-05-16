
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQADMIN_TYPES_H
#define EQADMIN_TYPES_H

#include <eq/admin/base.h>
#include <eq/base/refPtr.h>

namespace eq
{
namespace fabric
{
template< class C, class OV, class LV, class CV, class NV > class ConfigVisitor;
template< class T, class C > class ElementVisitor;
template< class T > class LeafVisitor;
}

namespace admin
{
class Canvas;
class Channel;
class Client;
class Config;
class Layout;
class Node;
class Observer;
class Pipe;
class Segment;
class Server;
class View;
class Window;

typedef base::RefPtr< Client > ClientPtr;
typedef base::RefPtr< Server > ServerPtr;


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
typedef std::vector< Config* >     ConfigVector;
/** A vector of pointers to eq::Node */
typedef std::vector< Node* >     NodeVector;
/** A vector of pointers to eq::Pipe */
typedef std::vector< Pipe* >     PipeVector;
/** A vector of pointers to eq::Window */
typedef std::vector< Window* >   WindowVector;
/** A vector of pointers to eq::Channel */
typedef std::vector< Channel* >  ChannelVector;
/** A vector of pointers to eq::Observer */
typedef std::vector< Observer* > ObserverVector;
/** A vector of pointers to eq::Canvas */
typedef std::vector< Canvas* >   CanvasVector;
/** A vector of pointers to eq::Layout */
typedef std::vector< Layout* >   LayoutVector;
/** A vector of pointers to eq::Segment */
typedef std::vector< Segment* >  SegmentVector;
/** A vector of pointers to eq::View */
typedef std::vector< View* >     ViewVector;
}
}
#endif // EQADMIN_TYPES_H
