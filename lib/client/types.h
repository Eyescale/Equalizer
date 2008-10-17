
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_TYPES_H
#define EQ_TYPES_H

#include <eq/base/refPtr.h>

#include <map>
#include <vector>

namespace eq
{

class Channel;
class Client;
class Frame;
class Image;
class Node;
class Pipe;
class Server;
class View;
class Window;
class X11Connection;
struct Statistic;

//----- Vectors
typedef std::vector< Node* >    NodeVector;
typedef std::vector< Pipe* >    PipeVector;
typedef std::vector< Window* >  WindowVector;
typedef std::vector< Channel* > ChannelVector;
typedef std::vector< Frame* >   FrameVector;
typedef std::vector< Image* >   ImageVector;
typedef std::vector< View* >    ViewVector;

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
