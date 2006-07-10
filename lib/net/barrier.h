
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_BARRIER_H
#define EQNET_BARRIER_H

#include <eq/net/object.h>

#include <eq/base/lock.h>

namespace eqNet
{
    class Node;

    /**
     * A networked barrier.
     */
    class Barrier : public Object
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

        /** A flag if the master instance has entered already. */
        bool               _masterEntered;
        /** Slave nodes which have entered the barrier. */
        NodeVector _enteredNodes;
        
        /** The lock used for synchronizing the master instance. */
        eqBase::Lock _masterNotify;
        /** The lock used for synchronizing the slave instances. */
        eqBase::Lock _slaveNotify;
        /** The lock used for thread-safety synchronization of _enteredNodes. */
        eqBase::Lock _leaveNotify;
        /** Flag for the master to enter leave synchronization. */
        bool         _waitForLeave;

        /** Mutex protecting concurrent access to some data. */
        eqBase::Lock _mutex;

        /** Common constructor function. */
        void _construct();

        /* The command handlers. */
        CommandResult _cmdEnter( Node* node, const Packet* pkg );
        CommandResult _cmdEnterReply( Node* node, const Packet* pkg );

#ifdef CHECK_THREADSAFETY
        pthread_t _threadID;
#endif
    };
}

#endif // EQNET_BARRIER_H

