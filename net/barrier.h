
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_BARRIER_H
#define EQNET_BARRIER_H

#include "base.h"
#include "object.h"

namespace eqNet
{
    class Node;

    class Barrier : public Object
    {
    public:
        /** 
         * Constructs a new barrier.
         */
        Barrier( Node* master, const uint32_t height );

        /**
         * Destructs the barrier.
         */
        virtual ~Barrier(){}

        /** 
         * Releases and destructs the barrier on all nodes.
         * @todo move to Object?
         */
        void release();

        /**
         * @name Operations
         */
        //@{
        /** 
         * Enters the barrier and blocks until the barrier has been reached.
         */
        void enter();
        //@}

    protected:

    private:
        /** The command functions. */
        void _cmdInit( eqNet::Node* node, const eqNet::Packet* packet );
    };
}

#endif // EQNET_BARRIER_H

