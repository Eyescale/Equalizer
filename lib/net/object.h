
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_OBJECT_H
#define EQNET_OBJECT_H

#include "commands.h"
#include "global.h"
#include "requestQueue.h"

#include <eq/base/base.h>
#include <eq/base/refPtr.h>
#include <eq/net/base.h>

namespace eqNet
{
    class Node;
    class Session;
    struct ObjectPacket;

    /** A generic, distributed object. */
    class Object : public Base, public eqBase::Referenced
    {
    public:
        /**
         * The type (class) identifier of the object
         *
         * The type classifies two things: the concrete type of the object for
         * instantiation and the level of functionality (unmanaged, managed,
         * versioned) the object supports.
         *
         * Unmanaged objects have to provide command handlers for the commands
         * they will receive, see Base::registerCommand().
         *
         * Managed objects have to implement getInstanceData() and potentially
         * releaseInstanceData().
         * 
         * Versioned objects have to implement pack(), unpack() and potentially
         * releasePackData().
         */
        enum Type
        {
            TYPE_UNMANAGED,                     // A unmanaged object
            TYPE_MANAGED,                       // A managed object
            TYPE_BARRIER,                       // eqNet::Barrier
            TYPE_MANAGED_CUSTOM,                // managed objects up to:
            TYPE_VERSIONED        = 0x40000000, // A versioned object
            TYPE_VERSIONED_CUSTOM               // user versioned objects
        };

        enum InstState
        {
            INST_UNKNOWN = 0,
            INST_GETMASTERID,
            INST_GETMASTER,
            INST_GOTMASTER,
            INST_INIT,
            INST_ERROR
        };

        /** 
         * Construct a new object.
         * 
         * @param typeID the type (class) identifier of the object.
         * @param nCommands the number of commands handled by the object.
         */
        Object( const uint32_t typeID, const uint32_t nCommands );
        virtual ~Object();
        
        Session* getSession() const   { return _session; }
        uint32_t getID() const        { return _id; }

        int32_t getTypeID() const     { return _typeID; }

        /**
         * @name Automatic Instanciation
         */
        //*{
        /** 
         * Return the instance information about this managed object.
         *
         * The returned pointer has to point to at least size bytes of memory
         * allocated by the object. The data will be released before the next
         * call to this object, that is, this function may return a pointer to
         * member variables if no other threads are accessing the object.
         * 
         * @param size the size of the returned data, or unchanged for unmanaged
         *             objects.
         * @return the instance data, or <code>NULL</code> for unmanaged
         *         objects.
         */
        virtual const void* getInstanceData( uint64_t* size ){ return NULL; }

        /** 
         * Release the instance data obtained by getInstanceData().
         * 
         * @param data the data.
         */
        virtual void releaseInstanceData( const void* data ){}
        //*}

        /**
         * @name Versioning
         */
        //*{
        /** 
         * Commit a new version of this object.
         * 
         * If the object is not versioned or has not changed, no new version
         * will be generated, that is, the previous or initial (0) version
         * number is returned.
         *
         * @return the head version.
         */
        uint32_t commit();

        /** 
         * Explicitily obsoletes all versions including version.
         * 
         * @param version the version to obsolete
         */
        void obsolete( const uint32_t version );

        /** 
         * Automatically obsoletes old versions.
         *
         * Flags are a bitwise combination of the following values:
         *
         * AUTO_OBSOLETE_COUNT_VERSIONS: count 'full' versions are retained.
         * AUTO_OBSOLETE_COUNT_COMMIT: The versions for the last count commits
         *   are retained. Note that the number of versions may be less since
         *   commit may not generate a new version.
         * 
         * @param count the number of versions to retain.
         * @param flags additional flags for the auto-obsoletion mechanism
         */
        void setAutoObsolete( const uint32_t count, const uint32_t flags );

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

        /** 
         * Pack the changes since the last call to commit().
         * 
         * The returned pointer has to point to at least size bytes of memory
         * allocated by the object. The data will be released before the next
         * call to this object, that is, this function may return a pointer to
         * member variables if no other threads are accessing the object.
         * 
         * @param size the size of the returned data, or untouched for
         *             unversioned or unchanged objects.
         * @return the delta data, or <code>NULL</code> if the object has not
         *             changed or is not versioned.
         */
        virtual const void* pack( uint64_t* size ){ return NULL; }

        /** 
         * Release the delta data obtained by pack().
         * 
         * @param data the data.
         */
        virtual void releasePackData( const void* data ){}

        /**
         * Unpack a change.
         *
         * @param data the change delta. 
         */
        virtual void unpack( const void* data, const uint64_t size ) {}
        //*}

    protected:
        /** 
         * @return if this instance is the master version.
         */
        bool isMaster() const { return _master; }

        /** 
         * Add a subscribed slave to the managed object.
         * 
         * @param slave the slave.
         */
        void addSlave( eqBase::RefPtr<Node> slave ) 
            { _slaves.push_back( slave ); }

        /** 
         * @return the vector of registered slaves.
         */
        std::vector< eqBase::RefPtr<Node> >& getSlaves()
            { return _slaves; }

    private:
        /** Indicates if this instance is the copy on the server node. */
        friend class Session;
        Session*     _session;

        /** The unique instance identifier. */
        uint32_t     _id;

        /** Master (writable) instance if <code>true</code>. */
        bool         _master;
        
        /** The list of subsribed slaves, kept on the master only. */
        std::vector< eqBase::RefPtr<Node> > _slaves;

        /** The type identifier of the object. */
        uint32_t _typeID;

        /** The current version. */
        uint32_t _version;

        /** The slave queue for changes. */
        RequestQueue _syncQueue;

        /* The command handlers. */
        CommandResult _cmdSync( Node* node, const Packet* pkg )
            { _syncQueue.push( node, pkg ); return eqNet::COMMAND_HANDLED; }

        void _reqSync( Node* node, const Packet* pkg );
    };
}

#endif // EQNET_OBJECT_H
