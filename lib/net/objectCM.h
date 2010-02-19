
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQNET_OBJECTCM_H
#define EQNET_OBJECTCM_H

#include <eq/net/dispatcher.h>   // base class
#include <eq/net/types.h>

#define EQ_INSTRUMENT_MULTICAST
#ifdef EQ_INSTRUMENT_MULTICAST
#  include <eq/base/atomic.h>
#endif

namespace eq
{
namespace net
{
    class Node;
    class Object;

    /** 
     * The object change manager base class.
     *
     * Each object has a change manager to create and store version
     * information. The type of change manager depends on the object
     * implementation, and if it is the master object or a slave object.
     * @internal
     */
    class ObjectCM : public Dispatcher
    {
    public:
        /** Construct a new change manager. */
        ObjectCM() {}
        virtual ~ObjectCM() {}

        /** 
         * Make this object thread safe.
         * 
         * The caller has to ensure that no other thread is using this object
         * when this function is called. It is primarily used by the session
         * during object instantiation.
         * @sa Session::getObject().
         */
        virtual void makeThreadSafe() = 0;

        /**
         * @name Versioning
         */
        //@{
        /** 
         * Start committing a new version.
         * 
         * @return the commit identifier to be passed to commitSync
         * @sa commitSync
         */
        virtual uint32_t commitNB() = 0;
        
        /** 
         * Finalize a commit transaction.
         * 
         * @param commitID the commit identifier returned from commitNB
         * @return the new head version.
         */
        virtual uint32_t commitSync( const uint32_t commitID ) = 0;

        /** 
         * Explicitily obsolete all versions.
         * 
         * @param version the version to obsolete
         */
        virtual void obsolete( const uint32_t version ) = 0;

        /** 
         * Automatically obsolete old versions.
         * 
         * @param count the number of versions to retain, excluding the head
         *              version.
         * @param flags additional flags for the auto-obsoletion mechanism
         */
        virtual void setAutoObsolete( const uint32_t count,
                                      const uint32_t flags ) = 0;
 
        /** @return get the number of versions this object retains. */
        virtual uint32_t getAutoObsoleteCount() const = 0;

        /** 
         * Sync to a given version.
         *
         * @param version the version to synchronize, must be bigger than the
         *                current version.
         * @return the version of the object after the operation.
         */
        virtual uint32_t sync( const uint32_t version ) = 0;

        /** @return the latest available (head) version. */
        virtual uint32_t getHeadVersion() const = 0;

        /** @return the current version. */
        virtual uint32_t getVersion() const = 0;

        /** @return the oldest available version. */
        virtual uint32_t getOldestVersion() const = 0;
        //@}

        /** @return if this instance is the master version. */
        virtual bool isMaster() const = 0;

        /** @return the instance identifier of the master object. */
        virtual uint32_t getMasterInstanceID() const = 0;

        /** Set the master node. */
        virtual void setMasterNode( NodePtr node ) { /* nop */ }

        /** 
         * Add a subscribed slave to the managed object.
         * 
         * @param command the subscribe command initiating the add.
         * @return the first version the slave has to use from its cache.
         */
        virtual uint32_t addSlave( Command& command ) = 0;

        /** 
         * Remove a subscribed slave.
         * 
         * @param node the slave node. 
         */
        virtual void removeSlave( NodePtr node ) = 0;

        /** @return the vector of current slave nodes. */
        virtual const NodeVector* getSlaveNodes() const { return 0; }

        /** Apply the initial data after mapping. */
        virtual void applyMapData() = 0;

        /** Add existing instance data to the object (from session cache) */
        virtual void addInstanceDatas( const InstanceDataDeque& cache, 
                                       const uint32_t startVersion )
            { EQDONTCALL; }

        /** Add the old master as a slave. */
        virtual void addOldMaster( NodePtr node, const uint32_t instanceID ) =0;

        /** The default CM for unattached objects. */
        static ObjectCM* ZERO;

    protected:
#ifdef EQ_INSTRUMENT_MULTICAST
        static base::a_int32_t _hit;
        static base::a_int32_t _miss;
#endif        
    };
}
}

#endif // EQNET_OBJECTCM_H
