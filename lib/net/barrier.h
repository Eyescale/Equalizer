
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
        Barrier( const void* instanceData );

        /**
         * Destructs the barrier.
         */
        virtual ~Barrier(){}

        /**
         * @name Data Access
         */
        const uint32_t getHeight(){ return _height; }
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
        /** @sa Mobject::getInstanceData */
        virtual const void* getInstanceData( uint64_t* size );

    private:
        /** The height of the barrier, only set on the master. */
        uint32_t _height;

        /** Slave nodes which have entered the barrier. */
        std::vector<Node*> _slaves; // XXX refptr?!
        
        /** The main lock used for barrier synchronization. */
        eqBase::Lock _enterLock;
        /** The lock used for thread-safety synchronization of _slaves. */
        eqBase::Lock _leaveLock;
        /** Flag for the master to enter leave synchronization. */
        bool         _waitForLeave;

        /* The command handlers. */
        CommandResult _cmdEnter( Node* node, const Packet* pkg );
        CommandResult _cmdEnterReply( Node* node, const Packet* pkg );
    };
}

#endif // EQNET_BARRIER_H

