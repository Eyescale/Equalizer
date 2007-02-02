
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_OBJECT_H
#define EQNET_OBJECT_H

#include <eq/net/base.h>
#include <eq/net/commands.h>
#include <eq/net/commandQueue.h>
#include <eq/net/global.h>
#include <eq/net/types.h>

#include <eq/base/idPool.h>
#include <eq/base/referenced.h>

namespace eqNet
{
    class Node;    class Session;
    struct ObjectPacket;

    /** A generic, distributed object. */
    class EQ_EXPORT Object : public Base, public eqBase::Referenced
    {
    public:
        /**
         * The type (class) identifier of the object
         *
         * The type classifies two things: the concrete type of the object for
         * instantiation, and the level of functionality (unmanaged, managed,
         * versioned) the object supports.
         *
         * Unmanaged objects have to provide command handlers for the commands
         * they will receive, see Base::registerCommand().
         *
         * Managed objects have to implement in addition getInstanceData() and
         * potentially releaseInstanceData() to enable object instanciation.
         * 
         * Versioned objects have additionally to implement pack(), unpack() and
         * potentially releasePackData() for version management.
         */
        enum Type
        {
            TYPE_BARRIER,              //!< eqNet::Barrier
            TYPE_INTERNAL = 0x100,     //!< internal eq object types
            TYPE_CUSTOM   = 0x200      //!< app-space versioned objects
        };

        /**
         * The share policy defines the scope of the object during
         * instanciation.
         * @sa Session::getObject()
         */
        enum SharePolicy
        {
            SHARE_UNDEFINED,//!< Not registered with the session
            SHARE_NODE,     //!< All threads on the a node share the same object
            SHARE_THREAD,   //!< One instance per thread
            SHARE_NEVER     //!< A new instance is created for the caller
        };

        /** 
         * The thread safety defines if critical sections are protected by the
         * object.
         *
         * Note that thread-safety will only be enabled during object
         * instanciation. There is no safe way to automatically enable thread
         * safety for an existing object, since the object may be in a cs in
         * another thread. An assertion will be generated if this condition is
         * encountered.
         * @sa makeThreadSafe(), Session::getObject()
         */
        enum ThreadSafety
        {
            CS_AUTO,   //*< Safe if SHARE_NODE, otherwise unsafe
            CS_SAFE,   //*< Protect critical sections
            CS_UNSAFE  //*< Do not protect critical sections
        };

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
        bool isThreadSafe() const      { return ( _mutex!=NULL ); }

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
         * is, the previous or initial (0) version number is returned. This
         * method is a convenience function for commitNB(); commitSync()
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
        uint32_t commitNB();
        
        /** 
         * Finalizes a commit transaction.
         * 
         * @param commitID the commit identifier returned from commitNB
         * @return the new head version.
         */
        uint32_t commitSync( const uint32_t commitID );

        /** 
         * Explicitily obsoletes all versions including version.
         * 
         * The head version can not be obsoleted.
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
         *   commit may not generate a new version. This flags takes precedence
         *   over AUTO_OBSOLETE_COUNT_VERSIONS.
         * 
         * @param count the number of versions to retain, excluding the head
         *              version.
         * @param flags additional flags for the auto-obsoletion mechanism
         */
        void setAutoObsolete( const uint32_t count, 
                           const uint32_t flags = AUTO_OBSOLETE_COUNT_VERSIONS )
            { _nVersions = count; _obsoleteFlags = flags; }

        /** @return get the number of versions this object retains. */
        uint32_t getAutoObsoleteCount() const { return _nVersions; }

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
                   /*, const float timeout = EQ_TIMEOUT_INDEFINITE*/ );

        /** 
         * Get the latest available version.
         * 
         * @return the head version.
         */
        uint32_t getHeadVersion() const;

        /** 
         * Get the currently synchronized version.
         * 
         * @return the current version.
         */
        uint32_t getVersion() const { return _version; }

        //*}

        /**
         * Get the unique type identifier.
         *
         * Each object type (class) has a unique identifier for instanciation
         * and registration in the session. When subclassing, create a new enum
         * for each class and return it from this function, starting at
         * Object::TYPE_CUSTOM.
         *
         * @return the unique type identifier.
         * @sa Session::instanciateObject, Object::Type
         */
        virtual uint32_t getTypeID() const = 0;

    protected:
        Object( const Object& from );

        /**
         * @name Automatic Instanciation and Versioning
         */
        //*{
        /** @return if the object is static (true) or versioned (false). */
        virtual bool isStatic() const { return true; }

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
        virtual const void* getInstanceData( uint64_t* size )
            { *size = _instanceDataSize; return _instanceData; }

        /** 
         * Release the instance data obtained by getInstanceData().
         * 
         * @param data the data.
         */
        virtual void releaseInstanceData( const void* data ){}

        /** 
         * Instanciates this object on a remote node.
         * 
         * @param node the remote node.
         * @param policy the share policy for the remote node instance.
         * @param version the version to instanciate.
         */
        void instanciateOnNode( eqBase::RefPtr<Node> node,
                                const SharePolicy policy, 
                                const uint32_t version, const bool threadSafe );

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
        virtual const void* pack( uint64_t* size )
            { *size = _deltaDataSize; return _deltaData; }

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
        virtual void unpack( const void* data, const uint64_t size ) 
            { 
                EQASSERT( size == _deltaDataSize );
                memcpy( _deltaData, data, size );
            }

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

        /** 
         * @return if this instance is the master version.
         */
        bool isMaster() const { return _master; }

        /** 
         * Add a subscribed slave to the managed object.
         * 
         * @param slave the slave.
         */
        void addSlave( eqBase::RefPtr<Node> slave );

        /** 
         * @return the vector of registered slaves.
         */
        const NodeVector& getSlaves() const
            { return _slaves; }

        /** @name Packet Transmission */
        //*{
        bool send( eqBase::RefPtr<Node> node, ObjectPacket& packet );
        bool send( eqBase::RefPtr<Node> node, ObjectPacket& packet, 
                   const std::string& string );
        bool send( eqBase::RefPtr<Node> node, ObjectPacket& packet, 
                   const void* data, const uint64_t size );

        bool send( NodeVector& nodes, ObjectPacket& packet );
        bool send( NodeVector nodes, ObjectPacket& packet, const void* data,
                   const uint64_t size );
        //*}
        
    private:
        /** Indicates if this instance is the copy on the server node. */
        friend class Session;
        Session*     _session;

        /** The session-unique object identifier. */
        uint32_t     _id;

        /** A node-unique identifier of the concrete instance. */
        uint32_t     _instanceID;

        /** Master (writable) instance if <code>true</code>. */
        bool         _master;
        
        /** The list of subsribed slaves, kept on the master only. */
        NodeVector _slaves;

        /** The current version. */
        uint32_t _version;

        /** The number of commits, needed for auto-obsoletion. */
        uint32_t _commitCount;

        /** The number of old versions to retain. */
        uint32_t _nVersions;

        /** The flags for automatic version obsoletion. */
        uint32_t _obsoleteFlags;

        /** The mutex, if thread safety is enabled. */
        eqBase::Lock* _mutex;

        /** A pointer to the object's instance data. */
        void*    _instanceData;
        uint64_t _instanceDataSize;

        /** A pointer to the object's delta data. */
        void*    _deltaData;
        uint64_t _deltaDataSize;

        struct InstanceData
        {
            InstanceData() : data(NULL), size(0), maxSize(0), commitCount(0) {}
            void*    data;
            uint64_t size;
            uint64_t maxSize;

            uint32_t commitCount;
        };
        struct ChangeData : public InstanceData
        {
            ChangeData() : version(0) {}
            uint32_t version;
        };
        
        /** The list of full instance datas, head version first. */
        std::deque<InstanceData> _instanceDatas;

        /** The list of change datas, (head-1):head change first. */
        std::deque<ChangeData> _changeDatas;
        
        std::vector<InstanceData> _instanceDataCache;
        std::vector<ChangeData>   _changeDataCache;

        /** The slave queue for changes. */
        CommandQueue _syncQueue;

        /** Common constructor code. */
        void _construct();

        void _syncToHead();

        uint32_t _commitInitial();
        void _setInitialVersion( const void* ptr, const uint64_t size );
        void _obsolete();
        void _checkConsistency() const;

        /* The command handlers. */
        CommandResult _cmdSync( Command& command );
        CommandResult _reqSync( Command& pkg );
        CommandResult _cmdCommit( Command& pkg );

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
