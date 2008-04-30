
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_TYPES_H
#define EQ_TYPES_H

#include <eq/base/refPtr.h>

#include <map>
#include <vector>

namespace eq
{

class Node;
class Pipe;
class Window;
class Channel;
class Frame;
class Image;
class X11Connection;
struct Statistic;

typedef std::vector< Node* >    NodeVector;
typedef std::vector< Pipe* >    PipeVector;
typedef std::vector< Window* >  WindowVector;
typedef std::vector< Channel* > ChannelVector;
typedef std::vector< Frame* >   FrameVector;
typedef std::vector< Image* >   ImageVector;

typedef eqBase::RefPtr< X11Connection > X11ConnectionPtr;

typedef std::vector< Statistic >                Statistics;

// originator id -> statistics
typedef std::map< uint32_t, Statistics >        SortedStatistics;

// frame id, config statistics
typedef std::pair< uint32_t, SortedStatistics > FrameStatistics;


}
#endif // EQ_TYPES_H
