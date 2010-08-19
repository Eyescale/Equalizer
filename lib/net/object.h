
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef EQNET_OBJECT_H
#define EQNET_OBJECT_H

#include <eq/net/dispatcher.h>    // base class
#include <eq/net/node.h>          // used in RefPtr
#include <eq/net/types.h>         // for Nodes
#include <eq/net/version.h>       // used as default parameter

namespace eq
{
namespace net
{
    class DataIStream;
    class DataOStream;
    class Node;
    class ObjectCM;
    class Session;
    struct ObjectPacket;

    /** 
     * A generic, distributed object.
     *
     * Please refer to the Programming Guide and examples on how to develop and
     * use distributed objects.
     * @sa eq::Object
     */
    class Object : public Dispatcher
    {
    public:
        /** Object change handling characteristics, see Programming Guide */
        enum ChangeType
        {
            STATIC,            //!< non-versioned, static object.
            INSTANCE,          //!< use only instance data
            DELTA,             //!< use pack/unpack delta
            UNBUFFERED,        //!< versioned, but don't retain versions
#ifdef EQ_USE_DEPRECATED
            DELTA_UNBUFFERED = UNBUFFERED
#endif
        };

        /** Construct a new distributed object. */
        EQ_EXPORT Object();

        /** Destruct the distributed object. */
        EQ_EXPORT virtual ~Object();

        /** 
         * Make this object thread safe.
         *
         * Only to be called on an unregistered and unmapped object. If you
         * don't call this function, certain operations, e.g., sync(), will not
         * be not-threadsafe.
         */
        EQ_EXPORT virtual void makeThreadSafe();  

        /** @sa Dispatcher::dispatchCommand(). */
        EQ_EXPORT virtual bool dispatchCommand( Command& command );

        /** @name Data Access */
        //@{
        /** @return true if the object has been made threadsafe, false if not.*/
        bool isThreadSafe() const      { return _threadSafe; }

        /** @return true if the object is attached, mapped or registered. */
        bool isAttached() const { return getID() <= EQ_ID_MAX; }

        /**
         * @return the local node to which this object is mapped, or 0 if the
         *         object is not mapped.
         */
        EQ_EXPORT NodePtr getLocalNode();

        /**
         * @return the session to which this object is mapped, or 0 if the
         *         object is not mapped.
         */
        const Session* getSession() const { return _session; }

        /**
         * @return the session to which this object is mapped, or 0 if the
         *         object is not mapped.
         */
        Session* getSession()             { return _session; }

        /** @return the session-wide unique object identifier. */
        uint32_t getID() const         { return _id; }

        /** @return the node-wide unique object instance identifier. */
        uint32_t getInstanceID() const { return _instanceID; }

        /** @return the master object instance identifier. @internal */
        EQ_EXPORT uint32_t getMasterInstanceID() const;

        /** 
         * @return true if this instance is the master version, false otherwise.
         */
        EQ_EXPORT bool isMaster() const;
        //@}

        /** @name Versioning */
        //@{
        /** @return how the changes are to be handled. */
        virtual ChangeType getChangeType() const { return STATIC; }

        /** 
         * Return if this object needs a commit.
         * 
         * This function is used for optimization, to detect early that no
         * commit is needed. If it returns true, pack() or getInstanceData()
         * will be executed. The serialization methods can still decide to not
         * write any data, upon which no new version will be created. If it
         * returns false, commitNB() and commitSync() will exit early.
         * 
         * @return true if a commit is needed.
         */
        virtual bool isDirty() const { return true; }

        /** 
         * Commit a new version of this object.
         * 
         * If the object has not changed no new version will be generated, that
         * is, the previous version number is returned. This method is a
         * convenience function for <code>commitNB(); commitSync();</code>
         *
         * Objects using the change type STATIC can not be committed. Slave
         * objects can be commited, but have certain caveats for
         * serialization. Please refer to the Programming Guide for more
         * details.
         *
         * @return the new head version.
         * @sa commitNB, commitSync
         */
        EQ_EXPORT uint32_t commit();

        /** 
         * Start committing a new version of this object.
         * 
         * The commit transaction has to be completed using commitSync, passing
         * the returned identifier.
         *
         * @return the commit identifier to be passed to commitSync
         * @sa commitSync
         */
        EQ_EXPORT virtual uint32_t commitNB();
        
        /** 
         * Finalizes a commit transaction.
         * 
         * @param commitID the commit identifier returned from commitNB
         * @return the new head version.
         */
        EQ_EXPORT virtual uint32_t commitSync( const uint32_t commitID );

        /** 
         * Explicitly obsoletes all versions including version.
         * 
         * The head version can not be obsoleted.
         * 
         * @param version the version to obsolete
         */
        EQ_EXPORT void obsolete( const uint32_t version );

        /** 
         * Automatically obsolete old versions.
         *
         * The versions for the last count commits are retained. Note that the
         * number of versions may be less since a commit may not generate a new
         * version.
         * 
         * @param count the number of versions to retain, excluding the head
         *              version.
         */
        EQ_EXPORT void setAutoObsolete( const uint32_t count );

        /** @return get the number of versions this object retains. */
        EQ_EXPORT uint32_t getAutoObsoleteCount() const;

        /** 
         * Sync to a given version.
         *
         * Syncing to VERSION_HEAD syncs to the last received version, does
         * not block, ignores the timeout and always returns true. Syncing to
         * VERSION_NEXT applies one new version, potentially blocking.
         *
         * Objects using the change type STATIC can not be synced. Master
         * objects can only be synced to VERSION_HEAD or VERSION_NEXT, since
         * slave objects commit do not generate a new version.
         * 
         * @param version the version to synchronize, must be bigger than the
         *                current version.
         * @return the version of the object after the operation.
         */
        EQ_EXPORT uint32_t sync( const uint32_t version = VERSION_HEAD );

        /** @return the latest available (head) version. */
        EQ_EXPORT uint32_t getHeadVersion() const;

        /** @return the currently synchronized version. */
        EQ_EXPORT uint32_t getVersion() const;

        /** @return the oldest available version. */
        EQ_EXPORT uint32_t getOldestVersion() const;

        /** 
         * Notification that a new head version was received by a slave object.
         *
         * The notification is send from the command thread, which is different
         * from the node main thread. The object should not be sync()'ed from
         * this notification, as this might lead to synchronization issues with
         * the application thread changing the object. Its purpose is to send a
         * message to the application, which then takes the appropriate action.
         * 
         * @param version The new head version.
         */
        virtual void notifyNewHeadVersion( const uint32_t version )
            { EQASSERT( getVersion()==VERSION_NONE||version<getVersion()+100 );}

        /** 
         * Notification that a new version was received by a master object.
         * @sa comment in notifyNewHeadVersion().
         */
        virtual void notifyNewVersion() {}
        //@}

        /** @name Serialization methods for instantiation and versioning. */
        //@{
        /** 
         * Serialize all instance information of this distributed object.
         *
         * @param os The output stream.
         */
        virtual void getInstanceData( DataOStream& os ) = 0;

        /**
         * Deserialize the instance data.
         *
         * This method is called during object mapping to populate slave
         * instances with the master object's data.
         * 
         * @param is the input stream.
         */
        virtual void applyInstanceData( DataIStream& is ) = 0;

        /** 
         * Serialize the modifications since the last call to commit().
         * 
         * No new version will be created if no data is written to the
         * output stream.
         * 
         * @param os the output stream.
         */
        virtual void pack( DataOStream& os ) { getInstanceData( os ); }

        /**
         * Deserialize a change.
         *
         * @param is the input data stream.
         */
        virtual void unpack( DataIStream& is ) { applyInstanceData( is ); }
        //@}

        /** @name Packet Transmission */
        //@{
        /** Send a packet to peer object instance(s) on another node. */
        EQ_EXPORT bool send( NodePtr node, ObjectPacket& packet );

        /** Send a packet to peer object instance(s) on another node. */
        EQ_EXPORT bool send( NodePtr node, ObjectPacket& packet,
                             const std::string& string );

        /** Send a packet to peer object instance(s) on another node. */
        EQ_EXPORT bool send( NodePtr node, ObjectPacket& packet, 
                             const void* data, const uint64_t size );
        //@}

        /** @name Notifications */
        /**
         * Notify that this object has been registered or mapped.
         *
         * The method is called from the thread initiating the registration or
         * mapping, after the operation has been completed successfully.
         * @sa isMaster()
         */
        virtual void notifyAttached() {};

        /**
         * Notify that this object will be deregistered or unmapped.
         *
         * The method is called from the thread initiating the deregistration or
         * unmapping, before the operation is executed.
         * @sa isMaster()
         */
        virtual void notifyDetach() {};
        //@}

    protected:
        /** Copy constructor. */
        EQ_EXPORT Object( const Object& );

        /** NOP assignment operator. */
        EQ_EXPORT const Object& operator = ( const Object& ) { return *this; }

        /** 
         * Setup the change manager.
         * 
         * @param type the type of the change manager.
         * @param master true if this object is the master.
         * @param masterInstanceID the instance identifier of the master object,
         *                         when master == false.
         * @internal
         */
        void setupChangeManager( const Object::ChangeType type, 
                                  const bool master, 
                              const uint32_t masterInstanceID = EQ_ID_INVALID );

        /**
         * Called when object is attached to session from the receiver thread.
         * @internal
         */
        EQ_EXPORT virtual void attachToSession( const uint32_t id, 
                                                const uint32_t instanceID, 
                                                Session* session );

        /**
         * Called when the object is detached from the session from the receiver
         * thread. @internal
         */
        EQ_EXPORT virtual void detachFromSession();

    private:
        /** Indicates if this instance is the copy on the server node. */
        Session*     _session;
        friend class Session;

        friend class DeltaMasterCM;
        friend class FullMasterCM;
        friend class MasterCM;
        friend class StaticMasterCM;
        friend class StaticSlaveCM;
        friend class UnbufferedMasterCM;
        friend class VersionedSlaveCM;

        /** The session-unique object identifier. */
        uint32_t     _id;

        /** A node-unique identifier of the concrete instance. */
        uint32_t     _instanceID;

        /** The object's change manager. */
        ObjectCM* _cm;

        /** Make synchronization thread safe. */
        bool _threadSafe;

        void _setChangeManager( ObjectCM* cm );
        const Nodes* _getSlaveNodes() const;

        /* The command handlers. */
        bool _cmdForward( Command& command );

        EQ_TS_VAR( _thread );
    };
    EQ_EXPORT std::ostream& operator << ( std::ostream&, const Object& );
}
}

#endif // EQNET_OBJECT_H
