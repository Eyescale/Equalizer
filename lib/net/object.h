
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
        };

        /** Construct a new distributed object. */
        EQNET_API Object();

        /** Destruct the distributed object. */
        EQNET_API virtual ~Object();

        /** @internal @sa Dispatcher::dispatchCommand(). */
        EQNET_API virtual bool dispatchCommand( Command& command );

        /** @name Data Access */
        //@{
        /** @return true if the object is attached, mapped or registered. */
        bool isAttached() const { return _session != 0; }

        /**
         * @return the local node to which this object is mapped, or 0 if the
         *         object is not mapped.
         */
        EQNET_API LocalNodePtr getLocalNode();

        /**
         * @return the session to which this object is mapped, or 0 if the
         *         object is not mapped.
         */
        const Session* getSession() const { return _session; }

        /**
         * @return the session to which this object is mapped, or 0 if the
         *         object is not mapped.
         */
        Session* getSession() { return _session; }

        /** @return the object's unique identifier. */
        const base::UUID& getID() const { return _id; }

        /** @internal Set the object's unique identifier */
        EQNET_API void setID( const base::UUID& identifier );

        /** @return the node-wide unique object instance identifier. */
        uint32_t getInstanceID() const { return _instanceID; }

        /** @internal @return the master object instance identifier. */
        EQNET_API uint32_t getMasterInstanceID() const;

        /** 
         * @return true if this instance is the master version, false otherwise.
         */
        EQNET_API bool isMaster() const;
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
         * This method is a convenience function for <code>commitSync( commitNB(
         * ))</code>.
         *
         * Objects using the change type STATIC can not be committed.
         *
         * Master instances will increment new versions continously, starting at
         * VERSION_FIRST. If the object has not changed, no new version will
         * be generated, that is, the current version is returned. The high
         * value of the returned version will always be 0.
         *
         * Slave objects can be commited, but have certain caveats for
         * serialization. Please refer to the Programming Guide for more
         * details. Slave object commits will return a random version on a
         * successful commit, or VERSION_NONE if the object has not changed
         * since the last commit. The high value of a successful commit will
         * never be 0.
         *
         * @return the new head version.
         * @sa commitNB(), commitSync()
         */
        EQNET_API uint128_t commit();

        /** 
         * Start committing a new version of this object.
         * 
         * The commit transaction has to be completed using commitSync, passing
         * the returned identifier.
         *
         * @return the commit identifier to be passed to commitSync
         * @sa commitSync()
         */
        EQNET_API virtual uint32_t commitNB();
        
        /** 
         * Finalize a commit transaction.
         * 
         * @param commitID the commit identifier returned from commitNB
         * @return the new head version.
         * @sa commit()
         */
        EQNET_API virtual uint128_t commitSync( const uint32_t commitID );

        /** 
         * Automatically obsolete old versions.
         *
         * The versions for the last count commits are retained. The actual
         * number of versions retained may be less since a commit does not
         * always generate a new version.
         * 
         * @param count the number of versions to retain, excluding the head
         *              version.
         */
        EQNET_API void setAutoObsolete( const uint32_t count );

        /** @return get the number of versions this object retains. */
        EQNET_API uint32_t getAutoObsolete() const;

        /** 
         * Sync to a given version.
         *
         * Objects using the change type STATIC can not be synced.
         *
         * Syncing to VERSION_HEAD syncs up to the last received version, does
         * not block and always returns true. Syncing to VERSION_NEXT applies
         * one new version, potentially blocking. Syncing to VERSION_NONE does
         * nothing.
         *
         * Slave objects can be synced to VERSION_HEAD, VERSION_NEXT or to any
         * version generated by a commit on the master instance. Syncing to a
         * concrete version applies all pending deltas up to this version and
         * potentially blocks.
         *
         * Master objects can only be synced to VERSION_HEAD, VERSION_NEXT or to
         * any version generated by a commit on a slave instance. Syncing to a
         * concrete version applies only this version and potentially blocks.
         *
         * The different sync semantics for concrete versions originate from the
         * fact that master versions are continous and ordered, while slave
         * versions are random and unordered.
         *
         * This function is not thread safe, that is, calling sync()
         * simultaneously from multiple threads has to be protected by the
         * caller using a mutex.
         * 
         * @param version the version to synchronize (see above).
         * @return the last version applied.
         */
        EQNET_API uint128_t sync( const uint128_t& version = VERSION_HEAD );

        /** @return the latest available (head) version. */
        EQNET_API uint128_t getHeadVersion() const;

        /** @return the currently synchronized version. */
        EQNET_API uint128_t getVersion() const;

        /** @return the oldest available version. */
        EQNET_API uint128_t getOldestVersion() const;

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
        EQNET_API virtual void notifyNewHeadVersion( const uint128_t& version );

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
        EQNET_API bool send( NodePtr node, ObjectPacket& packet );

        /** Send a packet to peer object instance(s) on another node. */
        EQNET_API bool send( NodePtr node, ObjectPacket& packet,
                             const std::string& string );

        /** Send a packet to peer object instance(s) on another node. */
        EQNET_API bool send( NodePtr node, ObjectPacket& packet, 
                             const void* data, const uint64_t size );
        //@}

    protected:
        /** Copy constructor. */
        EQNET_API Object( const Object& );

        /** NOP assignment operator. */
        const Object& operator = ( const Object& ) { return *this; }

        /**
         * Swap the session.
         *
         * @param session the new session to use.
         *
         * @internal
         */
        void swapSession( Session* session ) { _session = session; }
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

        EQNET_API virtual void attachToSession( const base::UUID& id, 
                                                const uint32_t instanceID, 
                                                Session* session );

        /**
         * Called when the object is detached from the session from the receiver
         * thread. @internal
         */
        EQNET_API virtual void detachFromSession();

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
        base::UUID _id;

        /** A session-unique identifier of the concrete instance. */
        uint32_t _instanceID;

        /** The object's change manager. */
        ObjectCM* _cm;

        void _setChangeManager( ObjectCM* cm );
        const Nodes* _getSlaveNodes() const;

        /* The command handlers. */
        bool _cmdForward( Command& command );

        EQ_TS_VAR( _thread );
    };
    EQNET_API std::ostream& operator << ( std::ostream&, const Object& );
}
}

#endif // EQNET_OBJECT_H
