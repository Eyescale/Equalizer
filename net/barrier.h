
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_BARRIER_H
#define EQNET_BARRIER_H

#include "base.h"
#include "mobject.h"

#include <eq/base/lock.h>

namespace eqNet
{
    class Node;

    /**
     * A networked barrier.
     */
    class Barrier : public Mobject, public Base
    {
    public:
        /** 
         * Constructs a new barrier.
         */
        Barrier( const uint32_t height );

        /** 
         * Constructs a new barrier.
         */
        Barrier( const char* instanceInfo );

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
         *
         * The implementation assumes that the master node instance also enters
         * the barrier.
         */
        void enter();
        //@}

    protected:
        /** @sa Mobject::getInstanceInfo */
        virtual void getInstanceInfo( uint32_t* typeID, std::string& data );

    private:
        /** The height of the barrier, only set on the master. */
        uint32_t _height;

        /** Indicates if this instance is the master. */
        bool     _master;
        
        /** Slave nodes which have entered the barrier. */
        std::vector<Node*> _slaves; // XXX refptr?!
        
        /** The lock used for barrier synchronization. */
        eqBase::Lock _lock;

        /* The command handlers. */
        CommandResult _cmdEnter( Node* node, const Packet* pkg );
        CommandResult _cmdEnterReply( Node* node, const Packet* pkg );
    };
}

#endif // EQNET_BARRIER_H

