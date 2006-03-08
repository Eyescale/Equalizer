
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_VERSIONEDOBJECT_H
#define EQNET_VERSIONEDOBJECT_H

#include "mobject.h"

#include "commands.h"
#include "requestQueue.h"

namespace eqNet
{
    class  Node;


    /** 
     * A versioned and managed object.
     * 
     * The versioning is similar to source code revisioning systems with the
     * constraint that only one instance, the master, can commit changes.
     * The delta is computed by the pack() routine and immediately send to all
     * subscribed proxies. The proxies buffer the delta and apply them using
     * unpack() when sync() is called. Sync'ed versions cannot be rolled back,
     * that is, a version smaller than the current version cannot be
     * synchronized. Versioned objects are obtained like Mobjects. 
     */
    class VersionedObject : public Mobject, public Base
    {
    public:
        VersionedObject( const uint32_t mobjectType, 
                         const uint32_t nCommands=CMD_VERSIONED_OBJECT_CUSTOM );
        virtual ~VersionedObject();
        
        /** 
         * Commit a new version of this object.
         * 
         * @return the new head version, or <code>0</code> upon error.
         */
        uint32_t commit();

        /** 
         * Sync to a given version.
         * 
         * @param version the version to synchronize, must be bigger than the
         *                current version.
         * @param timeout A timeout to wait for the version to be committed.
         * @return <code>true</code> if the version was synchronized,
         *         <code>false</code> if not.
         */
        bool sync( const uint32_t version, 
                   const float timeout = EQ_TIMEOUT_INDEFINITE );

        /** 
         * Sync to latest received version.
         * 
         * This function unpacks all received deltas. It does not block.
         */
        void sync();

        /** 
         * Get the latest available version.
         * 
         * @return the head version.
         */
        uint32_t getHeadVersion();

        /** 
         * Get the currently synchronized version.
         * 
         * @return the current version.
         */
        uint32_t getCurrentVersion() const { return _version; }

    protected: 
        /** 
         * Pack the changes since the last call.
         * 
         * The returned pointer has to point to at least size bytes of memory
         * allocated by the object. It can be free'd or reused on the next call
         * to this mobject.
         * 
         * @param size the size of the returned data.
         * @return the delta data.
         */
        virtual const void* pack( uint64_t* size ) = 0;

        /** 
         * Unpack a change.
         * 
         * @param data a data containing the changes.
         * @param size the size of the data.
         */
        virtual void unpack( const void* data, const uint64_t size ) = 0;

    private:
        /** The current version. */
        uint32_t _version;

        RequestQueue _syncQueue;

        /* The command handlers. */
        CommandResult _cmdSync( Node* node, const Packet* pkg )
            { _syncQueue.push( node, pkg ); return eqNet::COMMAND_HANDLED; }

        void _reqSync( Node* node, const Packet* pkg );
    };
}

#endif // EQNET_VERSIONEDOBJECT_H
