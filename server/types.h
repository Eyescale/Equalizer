
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

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
class Segment;
class View;


typedef std::vector< Config* >   ConfigVector;
typedef std::vector< Node* >     NodeVector;
typedef std::vector< Pipe* >     PipeVector;
typedef std::vector< Window* >   WindowVector;
typedef std::vector< Channel* >  ChannelVector;

typedef std::vector< Canvas* >   CanvasVector;
typedef std::vector< Compound* > CompoundVector;
typedef std::vector< Frame* >    FrameVector;
typedef std::vector< Layout* >   LayoutVector;
typedef std::vector< Segment* >  SegmentVector;
typedef std::vector< View* >     ViewVector;

typedef base::RefPtr< Server > ServerPtr;
typedef base::RefPtr< ConnectionDescription >   ConnectionDescriptionPtr;
typedef std::vector< ConnectionDescriptionPtr > ConnectionDescriptionVector;
}
}
#endif // EQSERVER_TYPES_H
