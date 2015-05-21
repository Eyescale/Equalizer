
/* Copyright (c) 2009-2014, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2012, Daniel Nachbaur <danielnachbaur@gmail.com>
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
#include <eq/fabric/types.h>
#include <co/objectOCommand.h>      // used inline send()
#include <co/objectVersion.h>       // member
#include <co/serializable.h>        // base class

namespace eq
{
namespace fabric
{
namespace detail { class Object; }

/**
 * Internal base class for all distributed, inheritable Equalizer objects.
 *
 * This class provides common data storage used by all Equalizer resource
 * entities. Do not subclass directly.
 */
class Object : public co::Serializable
{
public:
    /** @name Data Access. */
    //@{
    /** Set the name of the object. @version 1.0 */
    EQFABRIC_API virtual void setName( const std::string& name );

    /** @return the name of the object. @version 1.0 */
    EQFABRIC_API const std::string& getName() const;

    /**
     * Set user-specific data.
     *
     * Registration, mapping, commit and sync of the user data object are
     * automatically executed when committing and syncing this object. Not all
     * instances of the object have to set a user data object. All instances
     * have to set the same type of object.
     * @version 1.0
     */
    EQFABRIC_API void setUserData( co::Object* userData );

    /** @return the user-specific data. @version 1.0 */
    EQFABRIC_API co::Object* getUserData();

    /** @return the user-specific data. @version 1.0 */
    EQFABRIC_API const co::Object* getUserData() const;
    //@}

    /** @name Data Access */
    //@{
    /**
     * Return the set of tasks this channel might execute in the worst case.
     *
     * It is not guaranteed that all the tasks will be actually executed during
     * rendering.
     *
     * @return the tasks.
     * @warning Experimental - may not be supported in the future
     */
    EQFABRIC_API uint32_t getTasks() const;

    EQFABRIC_API uint32_t getSerial() const; //!< @internal unique serial number
    //@}

    /** @return true if the object has data to commit. @version 1.0 */
    EQFABRIC_API virtual bool isDirty() const;

    /** @internal */
    EQFABRIC_API virtual uint128_t commit( const uint32_t incarnation =
                                           CO_COMMIT_NEXT );

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
        DIRTY_TASKS      = Serializable::DIRTY_CUSTOM << 2, // 4
        DIRTY_REMOVED    = Serializable::DIRTY_CUSTOM << 3, // 8
        DIRTY_SERIAL     = Serializable::DIRTY_CUSTOM << 4, // 16
        // Leave room for binary-compatible patches
        DIRTY_CUSTOM     = Serializable::DIRTY_CUSTOM << 6, // 64
        DIRTY_OBJECT_BITS = DIRTY_NAME | DIRTY_USERDATA
    };

protected:
    /** Construct a new Object. */
    EQFABRIC_API Object();

    /** Construct an unmapped, unregistered copy of an object. */
    EQFABRIC_API Object( const Object& );

    /** Destruct the object. */
    EQFABRIC_API virtual ~Object();

    /** NOP assignment operator. @version 1.1.1 */
    EQFABRIC_API Object& operator = ( const Object& from );

    /**
     * @return true if this instance shall hold the master instance of the
     *         user data object, false otherwise.
     */
    virtual bool hasMasterUserData() { return false; }

    /** @return the latency to be used for keeping user data versions. */
    virtual uint32_t getUserDataLatency() const { return 0; }

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
    EQFABRIC_API void postRemove( Object* child );

    /** @internal Execute the slave remove request. @sa postRemove */
    virtual void removeChild( const uint128_t& ) { LBUNIMPLEMENTED; }

    /** @internal commit, register child slave instance with the server. */
    template< class C, class S >
    void commitChild( C* child, S* sender, uint32_t cmd,
                      const uint32_t incarnation );

    /** @internal commit slave instance to the server. */
    template< class C > inline
    void commitChild( C* child, const uint32_t incarnation )
        {
            LBASSERT( child->isAttached( ));
            child->commit( incarnation );
        }

    /** @internal commit, register child slave instances with the server. */
    template< class C, class S >
    void commitChildren( const std::vector< C* >& children, S* sender,
                         uint32_t cmd, const uint32_t incarnation );

    /** @internal commit, register child slave instances with the server. */
    template< class C >
    void commitChildren( const std::vector< C* >& children, uint32_t cmd,
                         const uint32_t incarnation )
        { commitChildren< C, Object >( children, this, cmd, incarnation ); }

    /** @internal commit all children. */
    template< class C >
    void commitChildren( const std::vector< C* >& children,
                         const uint32_t incarnation );

    /** @internal sync all children to head version. */
    template< class C >
    void syncChildren( const std::vector< C* >& children );

    /** @internal unmap/deregister all children. */
    template< class P, class C >
    inline void releaseChildren( const std::vector< C* >& children );

    /** @internal sync master object to the given slave commit. */
    EQFABRIC_API bool _cmdSync( co::ICommand& command );

private:
    detail::Object* const _impl;
};

// Template Implementation
template< class C, class S > inline void
Object::commitChild( C* child, S* sender, uint32_t cmd,
                     const uint32_t incarnation )
{
    if( !child->isAttached( ))
    {
        LBASSERT( !isMaster( ));
        co::LocalNodePtr localNode = child->getConfig()->getLocalNode();
        lunchbox::Request< uint128_t > request =
            localNode->registerRequest< uint128_t >();
        co::NodePtr node = child->getServer().get();
        sender->send( node, cmd ) << request;

        LBCHECK( localNode->mapObject( child, request.wait(),
                                       co::VERSION_NONE ));
    }
    child->commit( incarnation );
}

template< class C, class S > inline void
Object::commitChildren( const std::vector< C* >& children, S* sender,
                        uint32_t cmd, const uint32_t incarnation )
{
    // TODO Opt: async register and commit
    for( typename std::vector< C* >::const_iterator i = children.begin();
         i != children.end(); ++i )
    {
        C* child = *i;
        commitChild< C, S >( child, sender, cmd, incarnation );
    }
}

template< class C >
inline void Object::commitChildren( const std::vector< C* >& children,
                                    const uint32_t incarnation )
{
    // TODO Opt: async commit
    for( typename std::vector< C* >::const_iterator i = children.begin();
         i != children.end(); ++i )
    {
        C* child = *i;
        LBASSERT( child->isAttached( ));
        child->commit( incarnation );
    }
}

template< class C >
inline void Object::syncChildren( const std::vector< C* >& children )
{
    for( typename std::vector< C* >::const_iterator i = children.begin();
         i != children.end(); ++i )
    {
        C* child = *i;
        LBASSERT( child->isMaster( )); // slaves are synced using version
        child->sync();
    }
}

template< class P, class C >
inline void Object::releaseChildren( const std::vector< C* >& children )
{
    for( size_t i = children.size(); i > 0; --i )
    {
        C* child = children[ i - 1 ];

        if( child->isAttached( ))
        {
            getLocalNode()->releaseObject( child );
            if( !isMaster( ))
            {
                static_cast< P* >( this )->_removeChild( child );
                static_cast< P* >( this )->release( child );
            }
        }
        else
        {
            LBASSERT( isMaster( ));
        }
    }
}
}
}
#endif // EQFABRIC_OBJECT_H
