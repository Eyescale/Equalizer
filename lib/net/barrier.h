
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_BARRIER_H
#define EQNET_BARRIER_H

#include <eq/net/object.h>
#include <eq/net/nodeID.h>

#include <eq/base/sema.h>

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
        //@}
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
        /** @sa Object::getInstanceData */
        virtual const void* getInstanceData( uint64_t* size )
            { *size = sizeof( _data ); return &_data; }

        /** @sa Object::init */
        virtual void init( const void* data, const uint64_t dataSize );

        /** @sa Object::pack */
        virtual const void* pack( uint64_t* size )
            {
                *size   = sizeof( _data.height );
                return &_data.height;
            }

        /** @sa Object::unpack */
        virtual void unpack( const void* data, const uint64_t size ) 
            { _data.height = *(uint32_t*)data; }

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

        struct EnteredBarrier
        {
            EnteredBarrier( eqBase::RefPtr<Node> _node, uint32_t _instanceID )
                    : node(_node), instanceID( _instanceID ) {}

            eqBase::RefPtr<Node> node;
            uint32_t             instanceID;
        };
        /** Slave nodes which have entered the barrier. */
        std::vector<EnteredBarrier> _enteredBarriers;
        
        /** The sema used for barrier leave notification. */
        eqBase::Sema _leaveNotify;

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

