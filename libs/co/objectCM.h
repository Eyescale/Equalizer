
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_OBJECTCM_H
#define CO_OBJECTCM_H

#include <co/dispatcher.h>   // base class
#include <co/objectVersion.h> // VERSION_FOO values
#include <co/types.h>

//#define EQ_INSTRUMENT_MULTICAST
#ifdef EQ_INSTRUMENT_MULTICAST
#  include <co/base/atomic.h>
#endif

namespace co
{
    struct NodeMapObjectReplyPacket;

    /**
     * @internal
     * The object change manager base class.
     *
     * Each object has a change manager to create and store version information.
     * The type of change manager depends on the object implementation, and if
     * it is the master object or a slave object.
     */
    class ObjectCM : public Dispatcher
    {
    public:
        /** Construct a new change manager. */
        ObjectCM( Object* object );
        virtual ~ObjectCM() {}

        /** Initialize the change manager. */
        virtual void init() = 0;

        /** @name Versioning */
        //@{
        /** @sa Object::push() */
        virtual void push( const uint128_t& groupID, const uint128_t& typeID,
                           Nodes& nodes );

        /** 
         * Start committing a new version.
         * 
         * @param incarnation the commit incarnation for auto obsoletion.
         * @return the commit identifier to be passed to commitSync
         * @sa commitSync
         */
        virtual uint32_t commitNB( const uint32_t incarnation )
            { EQUNIMPLEMENTED; return EQ_UNDEFINED_UINT32; }

        /** 
         * Finalize a commit transaction.
         * 
         * @param commitID the commit identifier returned from commitNB
         * @return the new head version.
         */
        virtual uint128_t commitSync( const uint32_t commitID )
            { EQUNIMPLEMENTED; return VERSION_NONE; }

        /** Increase the count of how often commit() was called. */
        virtual void increaseCommitCount( const uint32_t incarnation )
            { /* NOP */ }

        /** 
         * Automatically obsolete old versions.
         * 
         * @param count the number of versions to retain, excluding the head
         *              version.
         */
        virtual void setAutoObsolete( const uint32_t count ){ EQUNIMPLEMENTED; }
 
        /** @return get the number of versions this object retains. */
        virtual uint32_t getAutoObsolete() const { EQUNIMPLEMENTED; return 0; }

        /** 
         * Sync to a given version.
         *
         * @param version the version to synchronize, must be bigger than the
         *                current version.
         * @return the version of the object after the operation.
         */
        virtual uint128_t sync( const uint128_t& version )
            { EQUNIMPLEMENTED; return VERSION_FIRST; }

        /** @return the latest available (head) version. */
        virtual uint128_t getHeadVersion() const = 0;

        /** @return the current version. */
        virtual uint128_t getVersion() const = 0;
        //@}

        /** @return if this object keeps instance data buffers. */
        virtual bool isBuffered() const{ return false; }

        /** @return if this instance is the master version. */
        virtual bool isMaster() const = 0;

        /** @return the instance identifier of the master object. */
        virtual uint32_t getMasterInstanceID() const = 0;

        /** Set the master node. */
        virtual void setMasterNode( NodePtr ) { /* nop */ }

        /** @return the master node, may be 0. */
        virtual NodePtr getMasterNode() { return 0; } 

        /** 
         * Add a subscribed slave to the managed object.
         * 
         * @param command the subscribe command initiating the add.
         * @param reply the reply packet.
         * @return the first version the slave has to use from its cache.
         */
        virtual void addSlave(Command& command, NodeMapObjectReplyPacket& reply)
            { EQUNIMPLEMENTED; }

        /** 
         * Remove a subscribed slave.
         * 
         * @param node the slave node. 
         */
        virtual void removeSlave( NodePtr node )
            { EQUNIMPLEMENTED; }

        /** Remove all subscribed slaves from the given node. */
        virtual void removeSlaves( NodePtr node ) = 0;

        /** @return the vector of current slave nodes. */
        virtual const Nodes* getSlaveNodes() const { return 0; }

        /** Apply the initial data after mapping. */
        virtual void applyMapData( const uint128_t& version )
            { EQUNIMPLEMENTED; }

        /** Add existing instance data to the object (from local node cache) */
        virtual void addInstanceDatas( const ObjectDataIStreamDeque&,
                                       const uint128_t& )
            { EQDONTCALL; }

        /** Speculatively send instance data to all nodes. */
        virtual void sendInstanceData( Nodes& nodes ){}

        /** @internal @return the object. */
        const Object* getObject( ) const { return _object; }

        /** @internal Swap the object. */
        void setObject( Object* object )
            { EQASSERT( object ); _object = object; }

        /** The default CM for unattached objects. */
        static ObjectCM* ZERO;

    protected:
        /** The managed object. */
        Object* _object;

#ifdef EQ_INSTRUMENT_MULTICAST
        static base::a_int32_t _hit;
        static base::a_int32_t _miss;
#endif

    private:
        bool _cmdPush( Command& command );
    };
}

#endif // CO_OBJECTCM_H
