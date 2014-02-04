
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

#include "object.h"

#include "task.h"

#include <co/iCommand.h>
#include <co/dataIStream.h>
#include <co/dataOStream.h>
#include <co/types.h>

namespace eq
{
namespace fabric
{
namespace detail
{
struct BackupData
{
    /** The application-defined name of the object. */
    std::string name;

    /** The user data parameters if no _userData object is set. */
    co::ObjectVersion userData;
};

class Object
{
public:
    Object()
        : userData( 0 )
        , tasks( TASK_NONE )
        , serial( CO_INSTANCE_INVALID )
    {}

    BackupData data; //!< Active data
    BackupData backup; //!< Backed up version (from .eqc)

    co::Object* userData; //!< User data object
    uint32_t tasks; //!< Worst-case set of tasks
    uint32_t serial; //!< Server-unique serial number

    /** The identifiers of removed children since the last slave commit. */
    std::vector< uint128_t > removedChildren;
};
}

Object::Object()
    : impl_( new detail::Object )
{}

Object::Object( const Object& from )
    : co::Serializable( from )
    , impl_( new detail::Object( *from.impl_ ))
{}

Object::~Object()
{
    LBASSERTINFO( !isAttached(), "Object " << getID() << " is still attached" );
    LBASSERTINFO( !impl_->userData,
                  "Unset user data before destruction to allow clean release" )

    co::LocalNodePtr node = getLocalNode();
    if( node.isValid() )
        node->releaseObject( this );
    delete impl_;
}

bool Object::isDirty() const
{
    if( impl_->userData && impl_->userData->isAttached( ))
    {
        if( impl_->userData->isMaster( ))
            impl_->userData->sync(); // apply slave object commits
        return Serializable::isDirty() || impl_->userData->isDirty();
    }

    // else
    return Serializable::isDirty();
}

uint128_t Object::commit( const uint32_t incarnation )
{
    if( !impl_->userData )
        return Serializable::commit( incarnation );

    if( !impl_->userData->isAttached() && hasMasterUserData( ))
    {
        getLocalNode()->registerObject( impl_->userData );
        impl_->userData->setAutoObsolete( getUserDataLatency() + 1 );
        impl_->data.userData = impl_->userData;
        setDirty( DIRTY_USERDATA );
    }

    if( impl_->userData->isDirty() && impl_->userData->isAttached( ))
    {
        const uint128_t& version = impl_->userData->commit( incarnation );
        LBASSERT( version != co::VERSION_NONE );
//        LBINFO << "Committed " << impl_->userData->getID() << " v" << version
//               << " of " << lunchbox::className( impl_->userData ) << " @"
//               << (void*)impl_->userData << lunchbox::backtrace << std::endl;

        LBASSERT( !impl_->userData->isDirty( ));
        LBASSERT( impl_->data.userData.identifier != impl_->userData->getID() ||
                  impl_->data.userData.version <= version );

        if( impl_->userData->isMaster() && impl_->data.userData != impl_->userData )
        {
            LBASSERTINFO( impl_->data.userData.identifier !=
                          impl_->userData->getID() ||
                          impl_->data.userData.version <
                          impl_->userData->getVersion(),
                          impl_->data.userData << " >= " <<
                          co::ObjectVersion( impl_->userData ));

            impl_->data.userData.identifier = impl_->userData->getID();
            impl_->data.userData.version = version;
            setDirty( DIRTY_USERDATA );
        }
    }

    return Serializable::commit( incarnation );
}

void Object::notifyDetach()
{
    Serializable::notifyDetach();
    if( !impl_->userData )
        return;

    LBASSERT( !impl_->userData->isAttached() ||
              impl_->userData->isMaster() == hasMasterUserData( ));

    if( impl_->userData->isMaster( ))
        impl_->data.userData = co::ObjectVersion( 0 );

    getLocalNode()->releaseObject( impl_->userData );
}

void Object::backup()
{
    LBASSERT( !impl_->userData );
    impl_->backup = impl_->data;
}

void Object::restore()
{
    LBASSERT( !impl_->userData );
    impl_->data = impl_->backup;
    setDirty( DIRTY_NAME | DIRTY_USERDATA );
}

void Object::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    if( dirtyBits & DIRTY_NAME )
        os << impl_->data.name;
    if( dirtyBits & DIRTY_USERDATA )
        os << impl_->data.userData;
    if( dirtyBits & DIRTY_TASKS )
        os << impl_->tasks;
    if( dirtyBits & DIRTY_REMOVED )
    {
        LBASSERT( !isMaster() ||
                  ( impl_->removedChildren.empty() && dirtyBits == DIRTY_ALL ))
        os << impl_->removedChildren;
        impl_->removedChildren.clear();
    }
    if( (dirtyBits & DIRTY_SERIAL) && isMaster( ))
    {
        impl_->serial = getInstanceID();
        os << impl_->serial;
    }
}

void Object::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
{
    if( dirtyBits & DIRTY_NAME )
        is >> impl_->data.name;
    if( dirtyBits & DIRTY_USERDATA )
    {
        is >> impl_->data.userData;
        // map&sync below to allow early exits
    }
    if( dirtyBits & DIRTY_TASKS )
        is >> impl_->tasks;
    if( dirtyBits & DIRTY_REMOVED )
    {
        std::vector< uint128_t > removed;
        is >> removed;
        if( !removed.empty( ))
        {
            LBASSERT( isMaster( ));
            std::vector< uint128_t >::const_iterator i = removed.begin();
            for( ; i != removed.end(); ++i )
            {
                removeChild( *i );
            }
        }
    }
    if( (dirtyBits & DIRTY_SERIAL) && !isMaster( ))
        is >> impl_->serial;

    if( isMaster( )) // redistribute changes
        setDirty( dirtyBits & getRedistributableBits( ));

    // Update user data state
    if( !(dirtyBits & DIRTY_USERDATA) || !impl_->userData )
        return;

    LBASSERTINFO( impl_->data.userData.identifier != impl_->userData->getID() ||
                  impl_->data.userData.version >= impl_->userData->getVersion() ||
                  impl_->userData->isMaster(),
                  "Incompatible version, new " << impl_->data.userData << " old " <<
                  co::ObjectVersion( impl_->userData ));

    if( impl_->data.userData.identifier == 0 )
    {
        if( impl_->userData->isAttached() && !impl_->userData->isMaster( ))
        {
            LBASSERT( !hasMasterUserData( ));
            getLocalNode()->unmapObject( impl_->userData );
        }
        return;
    }

    if( !impl_->userData->isAttached() && impl_->data.userData.identifier != 0 )
    {
        LBASSERT( !hasMasterUserData( ));
        //LBINFO << "Map " << impl_->data.userData << lunchbox::backtrace
        //       << std::endl;
        if( !getLocalNode()->mapObject( impl_->userData, impl_->data.userData ))
        {
            LBWARN << "Mapping of " << lunchbox::className( impl_->userData )
                   << " user data failed" << std::endl;
            LBUNREACHABLE;
            return;
        }
    }

    if( !impl_->userData->isMaster() && impl_->userData->isAttached( ))
    {
        LBASSERTINFO( impl_->userData->getID() == impl_->data.userData.identifier,
                      impl_->userData->getID() << " != " << impl_->data.userData.identifier );
#if 0
        if( impl_->userData->getVersion() < impl_->data.userData.version )
            LBINFO << "Sync " << impl_->data.userData << lunchbox::backtrace
                   << std::endl;
#endif
        impl_->userData->sync( impl_->data.userData.version );
    }
}

void Object::setName( const std::string& name )
{
    if( impl_->data.name == name )
        return;
    impl_->data.name = name;
    setDirty( DIRTY_NAME );
}

void Object::setUserData( co::Object* userData )
{
    LBASSERT( !userData || !userData->isAttached() );

    if( impl_->userData == userData )
        return;

    if( impl_->userData && impl_->userData->isAttached( ))
    {
        LBASSERT( impl_->userData->isMaster() == hasMasterUserData( ));
        impl_->userData->getLocalNode()->releaseObject( impl_->userData );
    }

    impl_->userData = userData;

    if( hasMasterUserData( ))
        setDirty( DIRTY_USERDATA );
    else if( impl_->data.userData.identifier != 0 )
    {
        co::LocalNodePtr node = getLocalNode();
        if( node.isValid() )
            node->mapObject( impl_->userData, impl_->data.userData );
    }
}

co::Object* Object::getUserData()
{
    return impl_->userData;
}

const co::Object* Object::getUserData() const
{
    return impl_->userData;
}

uint32_t Object::getTasks() const
{
    return impl_->tasks;
}

uint32_t Object::getSerial() const
{
    return impl_->serial;
}

const std::string& Object::getName() const
{
    return impl_->data.name;
}

void Object::setTasks( const uint32_t tasks )
{
    if( impl_->tasks == tasks )
        return;
    impl_->tasks = tasks;
    setDirty( DIRTY_TASKS );
}

void Object::postRemove( Object* child )
{
    LBASSERT( child );
    if( !child->isAttached( ))
        return;

    LBASSERT( !child->isMaster( ));
    LBASSERT( !isMaster( ));

    impl_->removedChildren.push_back( child->getID( ));
    setDirty( DIRTY_REMOVED );

    co::LocalNodePtr localNode = child->getLocalNode();
    localNode->releaseObject( child );
}

bool Object::_cmdSync( co::ICommand& )
{
    LBASSERT( isMaster( ));
    sync( co::VERSION_HEAD );
    return true;
}

}
}
