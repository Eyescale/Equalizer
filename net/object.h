
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_OBJECT_H
#define EQNET_OBJECT_H

#include <eq/base/base.h>

#include "base.h"
#include "global.h"

namespace eqNet
{
    class Node;
    class Session;
    struct ObjectPacket;

    /** A generic, distributed object. */
    class Object
    {
    public:
        Object();
        virtual ~Object();
        
        Session* getSession() const   { return _session; }
        uint32_t getID() const        { return _id; }

        /** 
         * Handle the passed command packet.
         * 
         * This function is executed from the receiver thread and should not
         * block for other packages.
         *
         * @param node the node sending the command packet.
         * @param packet the command packet.
         * @return the result of the operation.
         */
        virtual CommandResult handleCommand( Node* node, 
                                             const ObjectPacket* packet );

    protected:
        friend class  Session;
        uint32_t _id;
        Session* _session;

    private:
    };
}

#endif // EQNET_OBJECT_H
