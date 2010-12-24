
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

#include <eq/admin/api.h>
#include <co/base/refPtr.h>

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

typedef co::base::RefPtr< Client > ClientPtr;
typedef co::base::RefPtr< const Client > ConstClientPtr;
typedef co::base::RefPtr< Server > ServerPtr;

/** A visitor to traverse segments. @sa  Segment::accept() */
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
/** A vector of pointers to eq::admin::Config */
typedef std::vector< Config* >     Configs;
/** A vector of pointers to eq::admin::Node */
typedef std::vector< Node* >     Nodes;
/** A vector of pointers to eq::admin::Pipe */
typedef std::vector< Pipe* >     Pipes;
/** A vector of pointers to eq::admin::Window */
typedef std::vector< Window* >   Windows;
/** A vector of pointers to eq::admin::Channel */
typedef std::vector< Channel* >  Channels;
/** A vector of pointers to eq::admin::Observer */
typedef std::vector< Observer* > Observers;
/** A vector of pointers to eq::admin::Canvas */
typedef std::vector< Canvas* >   Canvass;
/** A vector of pointers to eq::admin::Layout */
typedef std::vector< Layout* >   Layouts;
/** A vector of pointers to eq::admin::Segment */
typedef std::vector< Segment* >  Segments;
/** A vector of pointers to eq::admin::View */
typedef std::vector< View* >     Views;
}
}
#endif // EQADMIN_TYPES_H
