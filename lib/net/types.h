
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_TYPES_H
#define EQNET_TYPES_H

#include <eq/base/refPtr.h>
#include <vector>

namespace eqNet
{
    class Node;

    typedef std::vector< eqBase::RefPtr<Node> > NodeVector;
}

#endif // EQNET_TYPES_H
