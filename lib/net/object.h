
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_OBJECT_H
#define EQNET_OBJECT_H

#include <eq/net/base.h>          // base class
#include <eq/net/node.h>          // used in RefPtr
#include <eq/net/objectCM.h>      // called inline
#include <eq/net/types.h>         // for NodeVector
#include <eq/base/nonCopyable.h>  // base class

namespace eqNet
{
    class DataIStream;
    class DataOStream;
    class Node;
    class Session;
    struct ObjectPacket;

    /** A generic, distributed object. */
    class EQ_EXPORT Object : public Base, public eqBase::NonCopyable
    {
    public:
        /**
         * Flags for auto obsoletion.
         */
        enum ObsoleteFlags
        {
            AUTO_OBSOLETE_COUNT_VERSIONS = 0,
            AUTO_OBSOLETE_COUNT_COMMITS  = 1
        };

        /** Special version enums */
        enum Version
        {
            VERSION_NONE = 0,
            VERSION_HEAD = 0xffffffffu
        };

        /** 
         * Construct a new object.
         */
        Object();

        virtual ~Object();

        /** 
         * Make this object thread safe.
         * 
         * The caller has to ensure that no other thread is using this object
         * when this function is called. It is primarily used by the session
         * during object instanciation.
         * @sa Session::getObject().
         */
        virtual void makeThreadSafe();  
        bool isThreadSafe() const      { return _threadSafe; }

        eqBase::RefPtr<Node> getLocalNode();
        Session* getSession() const    { return _session; }
        /** @return the session-wide unique object identifier. */
        uint32_t getID() const         { return _id; }
        /** @return the node-wide unique object instance identifier. */
        uint32_t getInstanceID() const { return _instanceID; }

        /**
         * @name Versioning
         */
        //*{
        /** 
         * Commit a new version of this object.
         * 
         * If the object has not changed no new version will be generated, that
         * is, the previous version number is returned. This method is a
         * convenience function for commitNB(); commitSync()
         *
         * @return the new head version.
         * @sa commitNB, commitSync
         */
        uint32_t commit();

        /** 
         * Start committing a new version of this object.
         * 
         * The commit transaction has to be completed using commitSync, passing
         * the returned identifier.
         *
         * @return the commit identifier to be passed to commitSync
         * @sa commitSync
         */
        uint32_t commitNB() { return _cm->commitNB(); }
        
        /** 
         * Finalizes a commit transaction.
         * 
         * @param commitID the commit identifier returned from commitNB
         * @return the new head version.
         */
        uint32_t commitSync( const uint32_t commitID ) 
        { return _cm->commitSync( commitID ); }

        /** 
         * Explicitily obsoletes all versions including version.
         * 
         * The head version can not be obsoleted.
         * 
         * @param version the version to obsolete
         */
        void obsolete( const uint32_t version ) { _cm->obsolete( version ); }

        /** 
         * Automatically obsolete old versions.
         *
         * Flags are a bitwise combination of the following values:
         *
         * AUTO_OBSOLETE_COUNT_VERSIONS: count 'full' versions are retained.
         * AUTO_OBSOLETE_COUNT_COMMIT: The versions for the last count commits
         *   are retained. Note that the number of versions may be less since
         *   commit may not generate a new version. This flags takes precedence
         *   over AUTO_OBSOLETE_COUNT_VERSIONS.
         * 
         * @param count the number of versions to retain, excluding the head
         *              version.
         * @param flags additional flags for the auto-obsoletion mechanism
         */
        void setAutoObsolete( const uint32_t count, 
                           const uint32_t flags = AUTO_OBSOLETE_COUNT_VERSIONS )
            { _cm->setAutoObsolete( count, flags ); }

        /** @return get the number of versions this object retains. */
        uint32_t getAutoObsoleteCount() const 
            { return _cm->getAutoObsoleteCount(); }

        /** 
         * Sync to a given version.
         *
         * Syncing to VERSION_HEAD syncs to the last received version, does
         * not block, ignores the timeout and always returns true.
         * 
         * @param version the version to synchronize, must be bigger than the
         *                current version.
         * @return <code>true</code> if the version was synchronized,
         *         <code>false</code> if not.
         */
        bool sync( const uint32_t version = VERSION_HEAD
                   /*, const float timeout = EQ_TIMEOUT_INDEFINITE*/ )
            { return _cm->sync( version ); }

        /** 
         * Get the latest available version.
         * 
         * @return the head version.
         */
        uint32_t getHeadVersion() const { return _cm->getHeadVersion(); }

        /** 
         * Get the currently synchronized version.
         * 
         * @return the current version.
         */
        uint32_t getVersion() const { return _cm->getVersion(); }
        //*}

    protected:
        Object( const Object& from );

        /**
         * @name Automatic Instanciation and Versioning
         */
        //*{
        /** @return if the object is static (true) or versioned (false). */
        virtual bool isStatic() const { return true; }

        /** 
         * @return false if deltas are identical to the instance data, true if 
         *         deltas are different from instance data.
         */
        virtual bool usePack() const
            { return ( !_instanceData || _instanceData != _deltaData ||
                       _instanceDataSize != _deltaDataSize ); }

        /** 
         * Serialize the instance information about this managed object.
         *
         * The default implementation uses the data provided by setInstanceData.
         *
         * @param os The output stream.
         */
        virtual void getInstanceData( DataOStream& os );

        /**
         * Deserialize the instance data.
         *
         * This method is called during object mapping to populate slave
         * instances with the master object's data. The default implementation
         * writes the data into the memory declared by setInstanceData.
         * 
         * @param is the input stream.
         */
        virtual void applyInstanceData( DataIStream& is );

        /** 
         * Serialize the modifications since the last call to commit().
         * 
         * No new version will be created if no data is written to the
         * ostream. The default implementation uses the data provided by
         * setDeltaData or setInstanceData.
         * 
         * @param os the output stream.
         */
        virtual void pack( DataOStream& os );
        
        /**
         * Deserialize a change.
         *
         * The default implementation writes the data into the memory declared
         * by setDeltaData or setInstanceData.
         *
         * @param is the input data stream.
         */
        virtual void unpack( DataIStream& is );

        /** 
         * Set the instance data of this object.
         * 
         * The data is used by the default data distribution methods to
         * generate the instance data for this object. It also sets the pack
         * data, if it was not set already.
         *
         * @param data A pointer to this object's data
         * @param size The size of the data.
         */
        void setInstanceData( void* data, const uint64_t size );

        /** 
         * Set the delta data of this object.
         * 
         * The data is used by the default data distribution methods to
         * generate the delta data for this object. Complex objects are
         * strongly advised to implement their own delta and undelta methods for
         * performance. 
         *
         * @param data A pointer to this object's data
         * @param size The size of the data.
         */
        void setDeltaData( void* data, const uint64_t size )
            { _deltaData = data; _deltaDataSize = size; }
        //*}

        /** @return if this instance is the master version. */
        bool isMaster() const { return _cm->isMaster(); }

        /** 
         * Add a slave subscriber.
         * 
         * @param node the slave node.
         * @param instanceID the object instance identifier on the slave node.
         */
        void addSlave( eqBase::RefPtr<Node> node, const uint32_t instanceID )
            { _cm->addSlave( node, instanceID ); }

        /** 
         * Remove a subscribed slave.
         * 
         * @param node the slave node. 
         */
        void removeSlave( eqBase::RefPtr<Node> node )
            { _cm->removeSlave( node ); }

        /** @name Packet Transmission */
        //*{
        bool send( eqBase::RefPtr<Node> node, ObjectPacket& packet );
        bool send( eqBase::RefPtr<Node> node, ObjectPacket& packet, 
                   const std::string& string );
        bool send( eqBase::RefPtr<Node> node, ObjectPacket& packet, 
                   const void* data, const uint64_t size );

        //bool send( NodeVector& nodes, ObjectPacket& packet );
        bool send( NodeVector nodes, ObjectPacket& packet, const void* data,
                   const uint64_t size );
        //*}
        
    private:
        /** Indicates if this instance is the copy on the server node. */
        friend class Session;
        Session*     _session;

        friend class DeltaMasterCM;
        friend class DeltaSlaveCM;
        friend class FullMasterCM;
        friend class FullSlaveCM;
        friend class StaticMasterCM;
        friend class StaticSlaveCM;

        /** The session-unique object identifier. */
        uint32_t     _id;

        /** A node-unique identifier of the concrete instance. */
        uint32_t     _instanceID;

        /** A pointer to the object's instance data. */
        void*    _instanceData;
        uint64_t _instanceDataSize;

        /** A pointer to the object's delta data. */
        void*    _deltaData;
        uint64_t _deltaDataSize;

        /** The object's change manager. */
        ObjectCM* _cm;

        /** Make synchronization thread safe. */
        bool _threadSafe;

        /** @name Methods used by session during mapping. */
        //*{
        /** @return the type of the change manager to be used. */
        virtual ObjectCM::Type _getChangeManagerType() const;
        
        /** 
         * Setup the change manager.
         * 
         * @param type the type of the change manager.
         * @param master true if this object is the master. 
         */
        void _setupChangeManager( const ObjectCM::Type type, const bool master);
        //*}

        /** Common constructor code. */
        void _construct();

        void _setChangeManager( ObjectCM* cm );

        /* The command handlers. */
        CommandResult _cmdForward( Command& command )
            { return _cm->invokeCommand( command ); }

        CHECK_THREAD_DECLARE( _thread );
    };

    /** A helper struct bundling an object identifier and a version. */
    struct ObjectVersion
    {
        ObjectVersion() {}
        ObjectVersion( Object* object ) 
                : objectID( object->getID( )),
                  version( object->getVersion( )) {}

        uint32_t objectID;
        uint32_t version;
    };
    inline std::ostream& operator << ( std::ostream& os, 
                                       const ObjectVersion& ov )
    {
        os << " id " << ov.objectID << " v" << ov.version;
        return os;
    }
}

#endif // EQNET_OBJECT_H
