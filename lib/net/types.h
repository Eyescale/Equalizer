
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_TYPES_H
#define EQNET_TYPES_H

#include <eq/base/refPtr.h>
#include <vector>

namespace eqNet
{

class Node;
class Object;
class Connection;
class ConnectionDescription;

typedef std::vector< eqBase::RefPtr< Node > >       NodeVector;
typedef std::vector< Object* >                      ObjectVector;
typedef std::vector< eqBase::RefPtr< Connection > > ConnectionVector;
typedef std::vector< eqBase::RefPtr< eqNet::ConnectionDescription > >
            ConnectionDescriptionVector;

}

#endif // EQNET_TYPES_H
