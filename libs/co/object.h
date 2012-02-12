
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_OBJECT_H
#define CO_OBJECT_H

#include <co/dispatcher.h>    // base class
#include <co/localNode.h>     // used in RefPtr
#include <co/types.h>         // for Nodes
#include <co/version.h>       // used as default parameter

namespace co
{
    class ObjectCM;
    struct NodeMapObjectReplyPacket;

#  define CO_COMMIT_NEXT EQ_UNDEFINED_UINT32 //!< the next commit incarnation

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
            NONE,              //!< @internal
            STATIC,            //!< non-versioned, static object.
            INSTANCE,          //!< use only instance data
            DELTA,             //!< use pack/unpack delta
            UNBUFFERED,        //!< versioned, but don't retain versions
        };

        /** Construct a new distributed object. */
        CO_API Object();

        /** Destruct the distributed object. */
        CO_API virtual ~Object();

        /** @name Data Access */
        //@{
        /** @return true if the object is attached, mapped or registered. */
        bool isAttached() const { return _instanceID != EQ_INSTANCE_INVALID; }

        /**
         * @return the local node to which this object is mapped, or 0 if the
         *         object is not mapped.
         */
        LocalNodePtr getLocalNode(){ return _localNode; };

        /**
         * Set the object's unique identifier.
         *
         * Only to be called on unattached objects. The application has to
         * ensure the uniqueness of the identifier in the peer-to-peer node
         * network. By default, each object has an identifier guaranteed to be
         * unique. During mapping, the identifier of the object will be
         * overwritten with the identifier of the master object.
         * @version 1.1.5
         */
        CO_API void setID( const base::UUID& identifier );

        /** @return the object's unique identifier. */
        const base::UUID& getID() const { return _id; }

        /** @return the node-wide unique object instance identifier. */
        uint32_t getInstanceID() const { return _instanceID; }

        /** @internal @return if this object keeps instance data buffers. */
        CO_API bool isBuffered() const;

        /** 
         * @return true if this instance is the master version, false otherwise.
         */
        CO_API bool isMaster() const;
        //@}

        /** @name Versioning */
        //@{
        /** @return how the changes are to be handled. */
        virtual ChangeType getChangeType() const { return STATIC; }

        /**
         * Return the compressor to be used for data transmission.
         *
         * This default implementation chooses the compressor with the highest
         * compression ratio with lossless compression for
         * EQ_COMPRESSOR_DATATYPE_BYTE tokens. The application may override this
         * method to deactivate compression by returning EQ_COMPRESSOR_NONE or
         * to select object-specific compressors.
         */
        CO_API virtual uint32_t chooseCompressor() const;

        /** 
         * Return if this object needs a commit.
         * 
         * This function is used for optimization, to detect early that no
         * commit is needed. If it returns true, pack() or getInstanceData()
         * will be executed. The serialization methods can still decide to not
         * write any data, upon which no new version will be created. If it
         * returns false, commit() will exit early.
         * 
         * @return true if a commit is needed.
         */
        virtual bool isDirty() const { return true; }

        /** 
         * Push the instance data of the object to the given nodes.
         *
         * Used to push object data from a Node, instead of pulling it during
         * mapping. Does not establish any mapping, that is, the receiving side
         * will typically use LocalNode::mapObject with VERSION_NONE to
         * establish a slave mapping. On each receiving node,
         * LocalNode::objectPush() is called once per node.
         *
         * Buffered objects do not re-serialize their instance data. Multicast
         * connections are preferred, that is, for a set of nodes sharing one
         * multicast connection the data is only send once.
         *
         * @param groupID An identifier to group a set of push operations.
         * @param typeID A per-push identifier.
         * @param nodes The vector of nodes to push to.
         */
        CO_API void push( const uint128_t& groupID, const uint128_t& typeID,
                          const Nodes& nodes );

        /** 
         * Commit a new version of this object.
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
         * The incarnation count is meaningful for buffered master objects. The
         * commit implementation will keep all instance data committed with an
         * incarnation count newer than <code>current_incarnation -
         * getAutoObsolete()</code>. By default, each call to commit creates a
         * new incarnation, retaining the data from last getAutoObsolete()
         * commit calls. When the application wishes to auto obsolete by another
         * metric than commit calls, it has to consistently provide an
         * incarnation counter. Buffers with a higher incarnation count than the
         * current are discarded. A typical use case is to tie the auto
         * obsoletion to an application-specific frame loop. Decreasing the
         * incarnation counter will lead to undefined results.
         *
         * @param incarnation the commit incarnation for auto obsoletion.
         * @return the new head version.
         */
        CO_API virtual uint128_t commit( const uint32_t incarnation =
                                         CO_COMMIT_NEXT );

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
        CO_API void setAutoObsolete( const uint32_t count );

        /** @return get the number of versions this object retains. */
        CO_API uint32_t getAutoObsolete() const;

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
         * simultaneously on the same object from multiple threads has to be
         * protected by the caller using a mutex.
         * 
         * @param version the version to synchronize (see above).
         * @return the last version applied.
         */
        CO_API virtual uint128_t sync( const uint128_t& version = VERSION_HEAD);

        /** @return the latest available (head) version. */
        CO_API uint128_t getHeadVersion() const;

        /** @return the currently synchronized version. */
        CO_API uint128_t getVersion() const;

        /** 
         * Notification that a new head version was received by a slave object.
         *
         * The notification is send from the receiver thread, which is different
         * from the node main thread. The object should not be sync()'ed from
         * this notification, as this might lead to deadlocks and
         * synchronization issues with the application thread changing the
         * object. Its purpose is to send a message to the application, which
         * then takes the appropriate action. In particular do not perform any
         * potentially blocking operations from this method.
         * 
         * @param version The new head version.
         */
        CO_API virtual void notifyNewHeadVersion( const uint128_t& version );

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
        CO_API bool send( NodePtr node, ObjectPacket& packet );

        /** Send a packet to peer object instance(s) on another node. */
        CO_API bool send( NodePtr node, ObjectPacket& packet,
                          const std::string& string );

        /** Send a packet to peer object instance(s) on another node. */
        CO_API bool send( NodePtr node, ObjectPacket& packet, 
                          const void* data, const uint64_t size );

        /** Send a packet to peer object instance(s) on another node. */
        template< class T >
        bool send( NodePtr node, ObjectPacket& packet, const std::vector<T>& v );
        //@}

        /** @name Notifications */
        /**
         * Notify that this object will be registered or mapped.
         *
         * The method is called from the thread initiating the registration or
         * mapping, before the operation is executed.
         */
        virtual void notifyAttach() {}

        /**
         * Notify that this object has been registered or mapped.
         *
         * The method is called from the thread initiating the registration or
         * mapping, after the operation has been completed successfully.
         * @sa isMaster()
         */
        virtual void notifyAttached() {}

        /**
         * Notify that this object will be deregistered or unmapped.
         *
         * The method is called from the thread initiating the deregistration or
         * unmapping, before the operation is executed.
         * @sa isMaster()
         */
        CO_API virtual void notifyDetach();

        /**
         * Notify that this object has been deregistered or unmapped.
         *
         * The method is called from the thread initiating the deregistration or
         * unmapping, after the operation has been executed.
         */
        virtual void notifyDetached() {}
        //@}

        /** @internal */
        //@{
        /** @internal @return the master object instance identifier. */
        uint32_t getMasterInstanceID() const;

        /** @internal @return the master object instance identifier. */
        NodePtr getMasterNode();

        /** @internal */
        void addSlave( Command& command, NodeMapObjectReplyPacket& reply );
        CO_API void removeSlave( NodePtr node ); //!< @internal
        CO_API void removeSlaves( NodePtr node ); //!< @internal
        void setMasterNode( NodePtr node ); //!< @internal
        /** @internal */
        void addInstanceDatas( const ObjectDataIStreamDeque&, const uint128_t&);

        /**
         * @internal
         * Setup the change manager.
         * 
         * @param type the type of the change manager.
         * @param master true if this object is the master.
         * @param localNode the node the object will be attached to.
         * @param masterInstanceID the instance identifier of the master object,
         *                         used when master == false.
         */
        void setupChangeManager( const Object::ChangeType type, 
                                 const bool master, LocalNodePtr localNode,
                                 const uint32_t masterInstanceID );
        /**
         * @internal
         * Called when object is attached from the receiver thread.
         */
        CO_API virtual void attach( const base::UUID& id, 
                                    const uint32_t instanceID );
        /**
         * @internal
         * Called when the object is detached from the local node from the
         * receiver thread.
         */
        CO_API virtual void detach();

        /** @internal Transfer the attachment from the given object. */
        void transfer( Object* from );

        void applyMapData( const uint128_t& version ); //!< @internal
        void sendInstanceData( Nodes& nodes ); //!< @internal
        //@}

    protected:
        /** Copy constructor. */
        CO_API Object( const Object& );

        /** NOP assignment operator. */
        const Object& operator = ( const Object& ) { return *this; }

    private:
        friend class DeltaMasterCM;
        friend class FullMasterCM;
        friend class MasterCM;
        friend class ObjectCM;
        friend class StaticMasterCM;
        friend class StaticSlaveCM;
        friend class UnbufferedMasterCM;
        friend class VersionedSlaveCM;

        /** The session-unique object identifier. */
        base::UUID _id;

        /** The node where this object is attached. */
        LocalNodePtr _localNode;

        /** A session-unique identifier of the concrete instance. */
        uint32_t _instanceID;

        /** The object's change manager. */
        ObjectCM* _cm;

        void _setChangeManager( ObjectCM* cm );

        EQ_TS_VAR( _thread );
    };
    CO_API std::ostream& operator << ( std::ostream&, const Object& );

    template< class T > inline bool
    Object::send( NodePtr node, ObjectPacket& packet, const std::vector<T>& v )
    {
        EQASSERT( isAttached() );
        packet.objectID  = _id;
        return node->send( packet, v );
    }
}

#endif // CO_OBJECT_H
