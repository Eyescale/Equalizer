
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_FULLSLAVECM_H
#define EQNET_FULLSLAVECM_H

#include <eq/net/objectCM.h>     // base class

#include <eq/net/commandQueue.h> // member
#include <eq/net/object.h>       // nested enum (Object::Version)
#include <eq/base/idPool.h>      // for EQ_ID_INVALID

namespace eqNet
{
    class Node;

    /** 
     * An object change manager handling full instance versions for slave
     * instances.
     */
    class FullSlaveCM : public ObjectCM
    {
    public:
        FullSlaveCM( Object* object );
        virtual ~FullSlaveCM();

        virtual void makeThreadSafe();

        /**
         * @name Versioning
         */
        //*{
        virtual uint32_t commitNB() { EQDONTCALL; return EQ_ID_INVALID; }
        virtual uint32_t commitSync( const uint32_t commitID )
            { EQDONTCALL; return Object::VERSION_NONE; }

        virtual void obsolete( const uint32_t version ) { EQDONTCALL; }

        virtual void setAutoObsolete( const uint32_t count,
                                      const uint32_t flags ) { EQDONTCALL; }
        virtual uint32_t getAutoObsoleteCount() const
            { EQDONTCALL; return 0; }

        virtual bool sync( const uint32_t version );

        virtual const void* getInitialData( uint64_t* size, uint32_t* version )
            { EQDONTCALL; return 0; }
        virtual void applyInitialData( const void* data, const uint64_t size,
                                       const uint32_t version );

        virtual uint32_t getHeadVersion() const;
        virtual uint32_t getVersion() const { return _version; }
        //*}

        virtual bool isMaster() const { return false; }

        virtual void addSlave( eqBase::RefPtr<Node> slave, 
                               const uint32_t instanceID )    { EQDONTCALL; }
        virtual void removeSlave( eqBase::RefPtr<Node> node ) { EQDONTCALL; }

    private:
        /** The managed object. */
        Object* _object;

        /** The current version. */
        uint32_t _version;

        /** The mutex, if thread safety is enabled. */
        eqBase::Lock* _mutex;

        /** The change queue. */
        CommandQueue _syncQueue;

        void _syncToHead();

        /* The command handlers. */
        CommandResult _cmdPushData( Command& command );
        CommandResult _reqInit( Command& command );

        CHECK_THREAD_DECLARE( _thread );
    };
}

#endif // EQNET_FULLSLAVECM_H
