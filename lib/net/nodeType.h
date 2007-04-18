
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_NODETYPE_H
#define EQNET_NODETYPE_H

namespace eqNet
{
    /** Node types to identify connecting nodes. */
    enum NodeType
    {
        TYPE_EQNET_INVALID,         //!< Invalid type
        TYPE_EQNET_NODE,            //!< A plain eqNet::Node
        TYPE_EQNET_USER = 0x100     //!< Application-specific types
    };
}

#endif // EQNET_NODETYPE_H
