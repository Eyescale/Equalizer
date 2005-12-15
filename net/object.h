
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_OBJECT_H
#define EQNET_OBJECT_H

#include <eq/base/base.h>

#include "global.h"

namespace eqNet
{
    class  Node;
    struct ObjectPacket;

    /** A generic, distributed object. */
    class Object
    {
    public:
        Object();
        virtual ~Object();
        
        uint getID() const { return _id; }

        virtual void handleCommand( Node* node, const ObjectPacket* packet );

    protected:
        friend class  Session;
        friend struct ObjectPacket;
        uint _id;
        uint _sessionID;

    private:
    };
}

#endif // EQNET_OBJECT_H
