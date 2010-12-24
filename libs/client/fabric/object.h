
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQFABRIC_OBJECT_H
#define EQFABRIC_OBJECT_H

#include <eq/fabric/api.h>
#include <eq/fabric/error.h>        // enum
#include <eq/fabric/serializable.h> // base class
#include <co/objectVersion.h>   // member

namespace eq
{
namespace fabric
{
    /**
     * Internal base class for all distributed, inheritable Equalizer objects.
     *
     * This class provides common data storage used by all Equalizer resource
     * entities. Do not subclass directly.
     */
    class Object : public Serializable
    {
    public:
        /** @name Data Access. */
        //@{
        /** Set the name of the object. @version 1.0 */
    	EQFABRIC_API void setName( const std::string& name );

        /** @return the name of the object. @version 1.0 */
    	EQFABRIC_API const std::string& getName() const;

        /**
         * Set user-specific data.
         *
         * Registration, mapping, commit and sync of the user data object are
         * automatically executed when committing and syncing this object. Not
         * all instances of the object have to set a user data object. All
         * instances have to set the same type of object.
         * @version 1.0
         */
    	EQFABRIC_API void setUserData( co::Object* userData );

        /** @return the user-specific data. @version 1.0 */
        co::Object* getUserData() { return _userData; }

        /** @return the user-specific data. @version 1.0 */
        const co::Object* getUserData() const { return _userData; }
        //@}

        /** @name Error Information. */
        //@{
        /** 
         * Set an error code why the last operation failed.
         * 
         * The error will be transmitted to the originator of the request, for
         * example to Config::init when set from within configInit().
         *
         * @param error the error message.
         * @version 1.0
         */
        EQFABRIC_API void setError( const int32_t error );

        /** @return the error from the last failed operation. @version 1.0 */
        co::base::Error getError() const { return _error; }
        //@}

        /** @name Data Access */
        //@{
        /** 
         * Return the set of tasks this channel might execute in the worst case.
         * 
         * It is not guaranteed that all the tasks will be actually executed
         * during rendering.
         * 
         * @return the tasks.
         * @warning Experimental - may not be supported in the future
         */
        uint32_t getTasks() const { return _tasks; }
        //@}

        /** @return true if the view has data to commit. @version 1.0 */
        EQFABRIC_API virtual bool isDirty() const;

        EQFABRIC_API virtual uint32_t commitNB(); //!< @internal

        /** @internal Back up app-specific data, excluding child data. */
        EQFABRIC_API virtual void backup();

        /** @internal Restore the last backup. */
        EQFABRIC_API virtual void restore();

        /**
         * The changed parts of the object since the last pack().
         *
         * Subclasses should define their own bits, starting at DIRTY_CUSTOM.
         */
        enum DirtyBits
        {
            DIRTY_NAME       = Serializable::DIRTY_CUSTOM << 0, // 1
            DIRTY_USERDATA   = Serializable::DIRTY_CUSTOM << 1, // 2
            DIRTY_ERROR      = Serializable::DIRTY_CUSTOM << 2, // 4
            DIRTY_TASKS      = Serializable::DIRTY_CUSTOM << 3, // 8
            DIRTY_REMOVED    = Serializable::DIRTY_CUSTOM << 4, // 16
            // Leave room for binary-compatible patches
            DIRTY_CUSTOM     = Serializable::DIRTY_CUSTOM << 6, // 64
            DIRTY_OBJECT_BITS = DIRTY_NAME | DIRTY_USERDATA | DIRTY_ERROR
        };

    protected:
        /** Construct a new Object. */
        EQFABRIC_API Object();
        
        /** Destruct the object. */
        EQFABRIC_API virtual ~Object();

        /**
         * @return true if this instance shall hold the master instance of the
         *         user data object, false otherwise.
         */
        virtual bool hasMasterUserData() { return false; }

        /** @internal Set the tasks this entity might potentially execute. */
        EQFABRIC_API void setTasks( const uint32_t tasks );

        EQFABRIC_API virtual void notifyDetach();

        EQFABRIC_API virtual void serialize( co::DataOStream& os,
                                          const uint64_t dirtyBits );

        EQFABRIC_API virtual void deserialize( co::DataIStream& is,
                                            const uint64_t dirtyBits );


        /** @internal @return the bits to be re-committed by the master. */
        virtual uint64_t getRedistributableBits() const
            { return DIRTY_OBJECT_BITS; }

        /**
         * @internal
         * Remove the given child on the master during the next commit.
         * @sa removeChild
         */
        EQFABRIC_API void postRemove( const Object* child );

        /** @internal Execute the slave remove request. @sa postRemove */
        virtual void removeChild( const co::base::UUID& ) { EQUNIMPLEMENTED; }

        /** @internal commit, register child slave instance with the server. */
        template< class C, class PKG, class S >
        void commitChild( C* child, S* sender );

        /** @internal commit, register child slave instances with the server. */
        template< class C, class PKG, class S >
        void commitChildren( const std::vector< C* >& children, S* sender );

        /** @internal commit, register child slave instances with the server. */
        template< class C, class PKG >
        void commitChildren( const std::vector< C* >& children )
            { commitChildren< C, PKG, Object >( children, this ); }

        /** @internal commit all children. */
        template< class C >
        void commitChildren( const std::vector< C* >& children );

        /** @internal sync all children to head version. */
        template< class C >
        void syncChildren( const std::vector< C* >& children );

        /** @internal unmap/deregister all children. */
        template< class P, class C >
        inline void releaseChildren( const std::vector< C* >& children );

    private:
        struct BackupData
        {
            /** The application-defined name of the object. */
            std::string name;

            /** The user data parameters if no _userData object is set. */
            co::ObjectVersion userData;
        }
            _data, _backup;

        /** The user data. */
        co::Object* _userData;

        /** Worst-case set of tasks. */
        uint32_t _tasks;

        /** The reason for the last error. */
        co::base::Error _error;

        /** The identifiers of removed children since the last slave commit. */
        std::vector< co::base::UUID > _removedChildren;

        union // placeholder for binary-compatible changes
        {
            char dummy[8];
        };
    };

    // Template Implementation
    template< class C, class PKG, class S > inline void
    Object::commitChild( C* child, S* sender )
    {
        if( !child->isAttached( ))
        {
            EQASSERT( !isMaster( ));
            co::LocalNodePtr localNode = child->getConfig()->getLocalNode();
            PKG packet;
            packet.requestID = localNode->registerRequest();

            co::NodePtr node = child->getServer().get();
            sender->send( node, packet );

            uint128_t identifier;
            localNode->waitRequest( packet.requestID, identifier );
            EQCHECK( localNode->mapObject( child, identifier,
                                           co::VERSION_NONE ));
        }
        child->commit();
    }

    template< class C, class PKG, class S > inline void
    Object::commitChildren( const std::vector< C* >& children, S* sender )
    {
        // TODO Opt: async register and commit
        for( typename std::vector< C* >::const_iterator i = children.begin();
             i != children.end(); ++i )
        {
            C* child = *i;
            commitChild< C, PKG, S >( child, sender );
        }
    }

    template< class C >
    inline void Object::commitChildren( const std::vector< C* >& children )
    {
        // TODO Opt: async commit
        for( typename std::vector< C* >::const_iterator i = children.begin();
             i != children.end(); ++i )
        {
            C* child = *i;
            EQASSERT( child->isAttached( ));
            child->commit();
        }
    }

    template< class C >
    inline void Object::syncChildren( const std::vector< C* >& children )
    {
        for( typename std::vector< C* >::const_iterator i = children.begin();
             i != children.end(); ++i )
        {
            C* child = *i;
            EQASSERT( child->isMaster( )); // slaves are synced using version
            child->sync();
        }
    }
 
    template< class P, class C >
    inline void Object::releaseChildren( const std::vector< C* >& children )
    {
        while( !children.empty( ))
        {
            C* child = children.back();
            if( !child->isAttached( ))
            {
                EQASSERT( isMaster( ));
                return;
            }

            getLocalNode()->releaseObject( child );
            if( !isMaster( ))
            {
                static_cast< P* >( this )->_removeChild( child );
                static_cast< P* >( this )->release( child );
            }
        }
    }
}
}
#endif // EQFABRIC_OBJECT_H
