
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_DELTAMASTERCM_H
#define EQNET_DELTAMASTERCM_H

#include "objectInstanceDataOStream.h"

#include <eq/net/nodeID.h>             // for NodeIDHash
#include <eq/net/objectCM.h>           // base class
#include <eq/net/types.h>              // for NodeVector

#include <deque>

namespace eq
{
namespace net
{
    class Node;
    class ObjectDeltaDataOStream;

    /** 
     * An object change manager handling full versions and deltas for the master
     * instance.
     */
    class DeltaMasterCM : public ObjectCM
    {
    public:
        DeltaMasterCM( Object* object );
        virtual ~DeltaMasterCM();

        virtual void notifyAttached();
        virtual void makeThreadSafe(){}

        /**
         * @name Versioning
         */
        //*{
        virtual uint32_t commitNB();
        virtual uint32_t commitSync( const uint32_t commitID );

        virtual void obsolete( const uint32_t version ) { EQUNIMPLEMENTED; }

        virtual void setAutoObsolete( const uint32_t count, 
                                      const uint32_t flags )
            { _nVersions = count; _obsoleteFlags = flags; }
        
        virtual uint32_t getAutoObsoleteCount() const { return _nVersions; }

        virtual uint32_t sync( const uint32_t version )
            { EQDONTCALL; return _version; }

        virtual uint32_t getHeadVersion() const { return _version; }
        virtual uint32_t getVersion() const     { return _version; }
        virtual uint32_t getOldestVersion() const;
        //*}

        virtual bool isMaster() const { return true; }
        virtual uint32_t getMasterInstanceID() const
            { EQDONTCALL; return EQ_ID_INVALID; }
        virtual void addSlave( NodePtr node, const uint32_t instanceID,
                               const uint32_t version );
        virtual void removeSlave( NodePtr node );

        virtual void applyMapData() { EQDONTCALL; }

    private:
        /** The managed object. */
        Object* _object;

        /** The list of subsribed slave nodes. */
        NodeVector _slaves;

        /** The number of object instances subscribed per slave node. */
        NodeIDHash< uint32_t > _slavesCount;

        /** The current version. */
        uint32_t _version;

        /** The number of commits, needed for auto-obsoletion. */
        uint32_t _commitCount;

        /** The number of old versions to retain. */
        uint32_t _nVersions;

        /** The flags for automatic version obsoletion. */
        uint32_t _obsoleteFlags;

        /** Registers request packets waiting for a return value. */
        base::RequestHandler _requestHandler;

        struct InstanceData
        {
            InstanceData( const Object* object )
                    : os( object ), commitCount(0) {}

            ObjectInstanceDataOStream os;
            uint32_t commitCount;
        };
        
        typedef ObjectDeltaDataOStream DeltaData;
        
        /** The list of full instance datas, head version first. */
        std::deque< InstanceData* > _instanceDatas;

        /** The list of delta datas, (head-1):head delta first. */
        std::deque< DeltaData* > _deltaDatas;
        
        std::vector< InstanceData* > _instanceDataCache;
        std::vector< DeltaData* >   _deltaDataCache;

        DeltaData*   _newDeltaData();
        InstanceData* _newInstanceData();

        void _obsolete();
        void _checkConsistency() const;

        /* The command handlers. */
        CommandResult _cmdCommit( Command& pkg );
        CommandResult _cmdDiscard( Command& pkg ) { return COMMAND_HANDLED; }

        CHECK_THREAD_DECLARE( _thread );
    };
}
}

#endif // EQNET_DELTAMASTERCM_H
