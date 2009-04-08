
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#include <eq/base/refPtr.h>
#include <vector>

namespace eq
{
namespace server
{

class Server;
class Config;
class Node;
class Pipe;
class Window;
class Channel;

class Canvas;
class Compound;
class ConnectionDescription;
class Frame;
class Layout;
class LoadBalancer;
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
typedef std::vector< LoadBalancer* > LoadBalancerVector;
typedef std::vector< Observer* >     ObserverVector;
typedef std::vector< Segment* >      SegmentVector;
typedef std::vector< View* >         ViewVector;

typedef base::RefPtr< Server > ServerPtr;
typedef base::RefPtr< ConnectionDescription >   ConnectionDescriptionPtr;
typedef std::vector< ConnectionDescriptionPtr > ConnectionDescriptionVector;
}
}
#endif // EQSERVER_TYPES_H
