
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_TYPES_H
#define EQNET_TYPES_H

#include <eq/base/refPtr.h>
#include <vector>

namespace eqNet
{

class Node;
class Connection;
class ConnectionDescription;

typedef std::vector< eqBase::RefPtr< Node > > NodeVector;
typedef std::vector< eqBase::RefPtr< Connection > > ConnectionVector;
typedef std::vector< eqBase::RefPtr< eqNet::ConnectionDescription > >
            ConnectionDescriptionVector;

}

#endif // EQNET_TYPES_H
