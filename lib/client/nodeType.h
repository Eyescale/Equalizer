
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_NODETYPE_H
#define EQ_NODETYPE_H

#include <eq/net/nodeType.h> // 'base' enum

namespace eq
{
    /** Node types to identify connecting nodes. */
    enum NodeType
    {
        TYPE_EQ_SERVER = eqNet::TYPE_EQNET_USER  //!< A server node
    };
}

#endif // EQ_NODETYPE_H
