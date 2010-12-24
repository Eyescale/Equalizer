
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

#include "object.h"

#include "command.h"
#include "dataIStream.h"
#include "dataOStream.h"
#include "deltaMasterCM.h"
#include "fullMasterCM.h"
#include "versionedSlaveCM.h"
#include "log.h"
#include "nodePackets.h"
#include "nullCM.h"
#include "objectCM.h"
#include "staticMasterCM.h"
#include "staticSlaveCM.h"
#include "types.h"
#include "unbufferedMasterCM.h"

#include <co/base/scopedMutex.h>
#include <iostream>

namespace co
{
Object::Object()
        : _id               ( true )
        , _instanceID       ( EQ_INSTANCE_INVALID )
        , _cm               ( ObjectCM::ZERO )
{}

Object::Object( const Object& object )
        : Dispatcher( object )
        , _id               ( true )
        , _localNode        ( 0 )
        , _instanceID       ( EQ_INSTANCE_INVALID )
        , _cm               ( ObjectCM::ZERO )
{}

Object::~Object()
{
    EQASSERTINFO( !_localNode,
                  "Object " << _id << " is still registered in localNode " <<
                  _localNode->getNodeID() << " in destructor" );
    
    if( _localNode.isValid() )
        _localNode->releaseObject( this );
    _localNode = 0;

    if( _cm != ObjectCM::ZERO )
        delete _cm;
    _cm = 0;
}

typedef CommandFunc<Object> CmdFunc;

void Object::attach( const co::base::UUID& id, const uint32_t instanceID )
{
    EQASSERT( !isAttached() );
    EQASSERT( _localNode );
    EQASSERT( instanceID <= EQ_INSTANCE_MAX );

    _id         = id;
    _instanceID = instanceID;
    EQLOG( LOG_OBJECTS )
        << _id << '.' << _instanceID << ": " << co::base::className( this )
        << (isMaster() ? " master" : " slave") << std::endl;
}

void Object::detach()
{
    _instanceID = EQ_INSTANCE_INVALID;
    _localNode = 0;
}

void Object::notifyDetach()
{
    if( !isMaster( ))
        return;

    // unmap slaves
    const Nodes* slaves = _cm->getSlaveNodes();
    if( !slaves || slaves->empty( ))
        return;

    EQWARN << slaves->size() << " slaves subscribed during deregisterObject of "
           << co::base::className( this ) << " id " << _id << std::endl;

    NodeUnmapObjectPacket packet;
    packet.objectID = _id;

    for( Nodes::const_iterator i = slaves->begin(); i != slaves->end(); ++i )
    {
        NodePtr node = *i;
        node->send( packet );
    }
}

void Object::transfer( Object* from )
{
    _id           = from->_id;
    _instanceID   = from->getInstanceID();
    _cm           = from->_cm; 
    _localNode    = from->_localNode;
    _cm->setObject( this );

    from->_cm = ObjectCM::ZERO;
    from->_localNode = 0;
    from->_instanceID = EQ_INSTANCE_INVALID;
}

void Object::_setChangeManager( ObjectCM* cm )
{
    if( _cm != ObjectCM::ZERO )
    {
        EQVERB
            << "Overriding existing object change manager, obj "
            << co::base::className( this ) << ", old cm " 
            << co::base::className( _cm ) << ", new cm " 
            << co::base::className( cm ) << std::endl;
        delete _cm;
    }

    _cm = cm;
    cm->init();
    EQLOG( LOG_OBJECTS ) << "set new change manager " << co::base::className( cm )
                         << " for " << co::base::className( this ) 
                         << std::endl;
}

void Object::setID( const co::base::UUID& identifier )
{
    EQASSERT( !isAttached( ));
    EQASSERT( identifier.isGenerated( ));
    _id = identifier;
}

bool Object::send( NodePtr node, ObjectPacket& packet )
{
    EQASSERT( isAttached( ));
    packet.objectID  = _id;
    return node->send( packet );
}

bool Object::send( NodePtr node, ObjectPacket& packet, 
                   const std::string& string )
{
    EQASSERT( isAttached() );
    packet.objectID  = _id;
    return node->send( packet, string );
}

bool Object::send( NodePtr node, ObjectPacket& packet, 
                   const void* data, const uint64_t size )
{
    EQASSERT( isAttached() );
    packet.objectID  = _id;
    return node->send( packet, data, size );
}

uint128_t Object::commit()
{
    const uint32_t requestID = commitNB();
    return commitSync( requestID );
}

void Object::setupChangeManager( const Object::ChangeType type, 
                                 const bool master, LocalNodePtr localNode,
                                 const uint32_t masterInstanceID )
{
    _localNode = localNode;

    switch( type )
    {
        case Object::NONE:
            EQASSERT( !_localNode );
            _setChangeManager( ObjectCM::ZERO );
            break;

        case Object::STATIC:
            EQASSERT( _localNode );
            if( master )
                _setChangeManager( new StaticMasterCM( this ));
            else
                _setChangeManager( new StaticSlaveCM( this ));
            break;

        case Object::INSTANCE:
            EQASSERT( _localNode );
            if( master )
                _setChangeManager( new FullMasterCM( this ));
            else
                _setChangeManager( new VersionedSlaveCM( this, 
                                                         masterInstanceID ));
            break;

        case Object::DELTA:
            EQASSERT( _localNode );
            if( master )
                _setChangeManager( new DeltaMasterCM( this ));
            else
                _setChangeManager( new VersionedSlaveCM( this,
                                                         masterInstanceID ));
            break;

        case Object::UNBUFFERED:
            EQASSERT( _localNode );
            if( master )
                _setChangeManager( new UnbufferedMasterCM( this ));
            else
                _setChangeManager( new VersionedSlaveCM( this,
                                                         masterInstanceID ));
            break;

        default: EQUNIMPLEMENTED;
    }
}

//---------------------------------------------------------------------------
// ChangeManager forwarders
//---------------------------------------------------------------------------
void Object::applyMapData( const uint128_t& version )
{
    _cm->applyMapData( version );
}

void Object::sendInstanceData( Nodes& nodes )
{
    _cm->sendInstanceData( nodes );
}

bool Object::isMaster() const
{
    return _cm->isMaster();
}

uint128_t Object::addSlave( Command& command )
{
    return _cm->addSlave( command );
}

void Object::removeSlave( NodePtr node )
{
    _cm->removeSlave( node );
}

void Object::setMasterNode( NodePtr node )
{
    _cm->setMasterNode( node );
}

void Object::addInstanceDatas( const ObjectDataIStreamDeque& cache,
                               const uint128_t& version )
{
    _cm->addInstanceDatas( cache, version );
}

uint32_t Object::commitNB()
{
    if( !isDirty( ))
    {
        _cm->increaseCommitCount();
        return EQ_UNDEFINED_UINT32;
    }
    return _cm->commitNB();
}

uint128_t Object::commitSync( const uint32_t commitID ) 
{
    if( commitID == EQ_UNDEFINED_UINT32 )
        return isMaster() ? getVersion() : VERSION_NONE;

    return _cm->commitSync( commitID );
}

void Object::setAutoObsolete( const uint32_t count )
{
    _cm->setAutoObsolete( count );
}

uint32_t Object::getAutoObsolete() const 
{
    return _cm->getAutoObsolete();
}

uint128_t Object::sync( const uint128_t& version )
{
    if( version == VERSION_NONE )
        return getVersion();
    return _cm->sync( version );
}

uint128_t Object::getHeadVersion() const
{
    return _cm->getHeadVersion();
}

uint128_t Object::getVersion() const
{
    return _cm->getVersion();
}

uint128_t Object::getOldestVersion() const
{
    return _cm->getOldestVersion();
}

void Object::notifyNewHeadVersion( const uint128_t& version )
{ 
    EQASSERTINFO( getVersion() == VERSION_NONE || 
                  version < getVersion() + 100, 
                  co::base::className( this ));
}

uint32_t Object::getMasterInstanceID() const
{
    return _cm->getMasterInstanceID();
}

const NodeID& Object::getMasterNodeID() const
{
    return _cm->getMasterNodeID();
}

std::ostream& operator << ( std::ostream& os, const Object& object )
{
    os << co::base::className( &object ) << " " << object.getID() << "."
       << object.getInstanceID() << " v" << object.getVersion();
    return os;
}

}
