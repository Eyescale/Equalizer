
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_TYPES_H
#define EQ_TYPES_H

#include <vector>

namespace eq
{

class Node;
class Pipe;
class Window;
class Channel;

typedef std::vector< Node* >    NodeVector;
typedef std::vector< Pipe* >    PipeVector;
typedef std::vector< Window* >  WindowVector;
typedef std::vector< Channel* > ChannelVector;

}
#endif // EQ_TYPES_H
