
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_TYPES_H
#define EQNET_TYPES_H

#include <eq/base/hash.h>
#include <eq/base/refPtr.h>
#include <vector>

namespace eq
{
namespace net
{

class Node;
class Session;
class Object;
class Connection;
class ConnectionDescription;

typedef base::RefPtr< Node >                  NodePtr;
typedef base::RefPtr< Connection >            ConnectionPtr;
typedef base::RefPtr< ConnectionDescription > ConnectionDescriptionPtr;

typedef std::vector< NodePtr >                   NodeVector;
typedef stde::hash_map< uint32_t, Session* >     SessionHash;
typedef std::vector< Object* >                   ObjectVector;
typedef stde::hash_map< uint32_t, ObjectVector > ObjectVectorHash;
typedef std::vector< ConnectionPtr >             ConnectionVector;
typedef std::vector< ConnectionDescriptionPtr >  ConnectionDescriptionVector;

}
}

#endif // EQNET_TYPES_H
