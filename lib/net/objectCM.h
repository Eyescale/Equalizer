
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_OBJECTCM_H
#define EQNET_OBJECTCM_H

#include <eq/net/base.h>

#include <eq/base/refPtr.h>

namespace eqNet
{
    class Node;
    class Object;

    /** 
     * The object change manager base class.
     *
     * Each object has a change manager to create and store version
     * information. The type of change manager depends on the object
     * implementation and if it is the master object or a slave object.
     */
    class ObjectCM : public Base
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
         * during object instanciation.
         * @sa Session::getObject().
         */
        virtual void makeThreadSafe() = 0;

        /**
         * @name Versioning
         */
        //*{
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
         * @return <code>true</code> if the version was synchronized,
         *         <code>false</code> if not.
         */
        virtual bool sync( const uint32_t version ) = 0;

        /** @return the latest available (head) version.
         */
        virtual uint32_t getHeadVersion() const = 0;

        /** @return the current version. */
        virtual uint32_t getVersion() const = 0;
        //*}

        /** 
         * @return if this instance is the master version.
         */
        virtual bool isMaster() const = 0;

        /** 
         * Add a subscribed slave to the managed object.
         * 
         * @param slave the slave.
         * @param instanceID the object instance identifier on the slave node.
         */
        virtual void addSlave( eqBase::RefPtr<Node> node, 
                               const uint32_t instanceID ) = 0;

        /** 
         * Remove a subscribed slave.
         * 
         * @param node the slave node. 
         */
        virtual void removeSlave( eqBase::RefPtr<Node> node ) = 0;

        static ObjectCM* ZERO;
    private:
    };
}

#endif // EQNET_OBJECTCM_H
