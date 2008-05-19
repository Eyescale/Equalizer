
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_TYPES_H
#define EQS_TYPES_H

#include <eq/base/refPtr.h>
#include <vector>

namespace eqs
{

class Server;
class Config;
class Node;
class Pipe;
class Window;
class Channel;

class Compound;
class Frame;

typedef std::vector< Config* >   ConfigVector;
typedef std::vector< Node* >     NodeVector;
typedef std::vector< Pipe* >     PipeVector;
typedef std::vector< Window* >   WindowVector;
typedef std::vector< Channel* >  ChannelVector;

typedef std::vector< Compound* > CompoundVector;
typedef std::vector< Frame* >    FrameVector;

typedef eqBase::RefPtr< eqs::Server > ServerPtr;

}
#endif // EQS_TYPES_H
