
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_BARRIER_H
#define EQNET_BARRIER_H

#include <eq/net/object.h>
#include <eq/net/nodeID.h>
#include <eq/base/monitor.h>

namespace eqNet
{
    class Node;

    /**
     * A networked, versioned barrier.
     */
    class Barrier : public Object
    {
    public:
        /** 
         * Constructs a new barrier.
         */
        Barrier( eqBase::RefPtr<Node> master, const uint32_t height=0 );

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
         *
         * After a change, the barrier should be committed and synced to the
         * same version on all nodes entering the barrier.
         */
        //*{
        void setHeight( const uint32_t height ){ _data.height = height; }
        void increase() { ++_data.height; }

        const uint32_t getHeight() const { return _data.height; }
        //*}

        /** @name Operations */
        //*{
        /** 
         * Enters the barrier and blocks until the barrier has been reached.
         *
         * The implementation assumes that the master node instance also enters
         * the barrier.
         */
        void enter();
        //*}

    protected:
        /** @sa Object::init */
        virtual void init( const void* data, const uint64_t dataSize );

    private:
        struct Data
        {
            /** The master barrier node. */
            NodeID   master;
            /** The height of the barrier, only set on the master. */
            uint32_t height;
        }
            _data;

        eqBase::RefPtr<Node> _master;

        /** Slave nodes which have entered the barrier. */
        std::vector< eqBase::RefPtr<Node> > _enteredNodes;
        
        /** The monitor used for barrier leave notification. */
        eqBase::Monitor<uint32_t> _leaveNotify;

        /** Common constructor function. */
        void _construct();

        /* The command handlers. */
        CommandResult _cmdEnter( Command& command );
        CommandResult _cmdEnterReply( Command& command );

        CHECK_THREAD_DECLARE( _thread );
    };
}

#endif // EQNET_BARRIER_H

