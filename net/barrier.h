
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_BARRIER_H
#define EQNET_BARRIER_H

#include "object.h"

namespace eqNet
{
    class Barrier : public Object
    {
    public:
        /** 
         * Constructs a new barrier.
         */
        Barrier( const uint32_t size );

        /**
         * Destructs the barrier.
         */
        virtual ~Barrier();

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
        void _reqInit( eqNet::Barrier* barrier, const eqNet::Packet* packet );
        void _reqExit( eqNet::Barrier* barrier, const eqNet::Packet* packet );
        void _reqStop( eqNet::Barrier* barrier, const eqNet::Packet* packet );
    };
}

#endif // EQNET_BARRIER_H

