
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_TYPES_H
#define EQS_TYPES_H

#include <vector>

namespace eqs
{

class Compound;
class Config;
class Node;
class Pipe;
class Window;
class Channel;
class X11Connection;

typedef std::vector< Compound* > CompoundVector;
typedef std::vector< Config* >   ConfigVector;
typedef std::vector< Node* >     NodeVector;
typedef std::vector< Pipe* >     PipeVector;
typedef std::vector< Window* >   WindowVector;
typedef std::vector< Channel* >  ChannelVector;

}
#endif // EQS_TYPES_H
