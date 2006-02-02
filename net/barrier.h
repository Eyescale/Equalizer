
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_BARRIER_H
#define EQNET_BARRIER_H

#include "base.h"
#include "mobject.h"

namespace eqNet
{
    class Node;

    class Barrier : public Mobject
    {
    public:
        /** 
         * Constructs a new barrier.
         */
        Barrier( const uint32_t height );

        /**
         * Destructs the barrier.
         */
        virtual ~Barrier(){}

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
        /** @sa Mobject::getInstanceInfo */
        virtual void getInstanceInfo( uint32_t* typeID, std::string& data );

    private:
        uint32_t _height;
    };
}

#endif // EQNET_BARRIER_H

