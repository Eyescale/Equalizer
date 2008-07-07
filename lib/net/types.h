
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_TYPES_H
#define EQNET_TYPES_H

#include <eq/base/refPtr.h>
#include <vector>

namespace eq
{
namespace net
{

class Node;
class Object;
class Connection;
class ConnectionDescription;

typedef eq::base::RefPtr< Node >                  NodePtr;
typedef eq::base::RefPtr< Connection >            ConnectionPtr;
typedef eq::base::RefPtr< ConnectionDescription > ConnectionDescriptionPtr;

typedef std::vector< NodePtr >                  NodeVector;
typedef std::vector< Object* >                  ObjectVector;
typedef std::vector< ConnectionPtr >            ConnectionVector;
typedef std::vector< ConnectionDescriptionPtr > ConnectionDescriptionVector;

}
}

#endif // EQNET_TYPES_H
