
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_TYPES_H
#define EQ_TYPES_H

#include <eq/base/refPtr.h>
#include <eq/client/pixelViewport.h>

#include <map>
#include <vector>

namespace eq
{

class Canvas;
class Channel;
class Client;
class Config;
class Frame;
class Image;
class Layout;
class Node;
class Pipe;
class Segment;
class Server;
class View;
class Viewport;
class Window;
class X11Connection;
struct Statistic;

//----- Vectors
typedef std::vector< Node* >     NodeVector;
typedef std::vector< Pipe* >     PipeVector;
typedef std::vector< Window* >   WindowVector;
typedef std::vector< Channel* >  ChannelVector;

typedef std::vector< Frame* >    FrameVector;
typedef std::vector< Image* >    ImageVector;

typedef std::vector< Canvas* >   CanvasVector;
typedef std::vector< Layout* >   LayoutVector;
typedef std::vector< Segment* >  SegmentVector;
typedef std::vector< View* >     ViewVector;
typedef std::vector< Viewport* > ViewportVector;

typedef std::vector< PixelViewport > PixelViewportVector;


//----- Reference Pointers
typedef base::RefPtr< X11Connection > X11ConnectionPtr;
typedef base::RefPtr< Client >        ClientPtr;
typedef base::RefPtr< Server >        ServerPtr;

//----- Others
typedef std::vector< Statistic >                Statistics;

// originator id -> statistics
typedef std::map< uint32_t, Statistics >        SortedStatistics;

// frame id, config statistics
typedef std::pair< uint32_t, SortedStatistics > FrameStatistics;


}
#endif // EQ_TYPES_H
