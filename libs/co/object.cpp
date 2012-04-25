
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

#include "object.h"

#include "command.h"
#include "cpuCompressor.h"
#include "dataIStream.h"
#include "dataOStream.h"
#include "deltaMasterCM.h"
#include "fullMasterCM.h"
#include "log.h"
#include "nodePackets.h"
#include "nullCM.h"
#include "objectCM.h"
#include "staticMasterCM.h"
#include "staticSlaveCM.h"
#include "types.h"
#include "unbufferedMasterCM.h"
#include "versionedSlaveCM.h"

#include <lunchbox/scopedMutex.h>
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
    LBASSERTINFO( !isAttached(),
                  "Object " << _id << " is still attached to node " <<
                  _localNode->getNodeID());
    
    if( _localNode.isValid() )
        _localNode->releaseObject( this );
    _localNode = 0;

    if( _cm != ObjectCM::ZERO )
        delete _cm;
    _cm = 0;
}

typedef CommandFunc<Object> CmdFunc;

void Object::attach( const UUID& id, const uint32_t instanceID )
{
    LBASSERT( !isAttached() );
    LBASSERT( _localNode );
    LBASSERT( instanceID <= EQ_INSTANCE_MAX );

    _id         = id;
    _instanceID = instanceID;
    LBLOG( LOG_OBJECTS )
        << _id << '.' << _instanceID << ": " << lunchbox::className( this )
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
    const Nodes slaves = _cm->getSlaveNodes();
    if( slaves.empty( ))
        return;

    LBWARN << slaves.size() << " slaves subscribed during deregisterObject of "
           << lunchbox::className( this ) << " id " << _id << std::endl;

    NodeUnmapObjectPacket packet;
    packet.objectID = _id;

    for( NodesCIter i = slaves.begin(); i != slaves.end(); ++i )
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
        LBVERB
            << "Overriding existing object change manager, obj "
            << lunchbox::className( this ) << ", old cm " << lunchbox::className( _cm )
            << ", new cm " << lunchbox::className( cm ) << std::endl;
        delete _cm;
    }

    _cm = cm;
    cm->init();
    LBLOG( LOG_OBJECTS ) << "set new change manager " << lunchbox::className( cm )
                         << " for " << lunchbox::className( this ) 
                         << std::endl;
}

void Object::setID( const UUID& identifier )
{
    LBASSERT( !isAttached( ));
    LBASSERT( identifier.isGenerated( ));
    _id = identifier;
}

bool Object::send( NodePtr node, ObjectPacket& packet )
{
    LBASSERT( isAttached( ));
    packet.objectID = _id;
    return node->send( packet );
}

bool Object::send( NodePtr node, ObjectPacket& packet, const std::string& string)
{
    LBASSERT( isAttached() );
    packet.objectID  = _id;
    return node->send( packet, string );
}

bool Object::send( NodePtr node, ObjectPacket& packet, const void* data,
                   const uint64_t size )
{
    LBASSERT( isAttached() );
    packet.objectID  = _id;
    return node->send( packet, data, size );
}

void Object::push( const uint128_t& groupID, const uint128_t& typeID,
                   const Nodes& nodes )
{
    _cm->push( groupID, typeID, nodes );
}

uint128_t Object::commit( const uint32_t incarnation )
{
    return _cm->commit( incarnation );
}


void Object::setupChangeManager( const Object::ChangeType type, 
                                 const bool master, LocalNodePtr localNode,
                                 const uint32_t masterInstanceID )
{
    _localNode = localNode;

    switch( type )
    {
        case Object::NONE:
            LBASSERT( !_localNode );
            _setChangeManager( ObjectCM::ZERO );
            break;

        case Object::STATIC:
            LBASSERT( _localNode );
            if( master )
                _setChangeManager( new StaticMasterCM( this ));
            else
                _setChangeManager( new StaticSlaveCM( this ));
            break;

        case Object::INSTANCE:
            LBASSERT( _localNode );
            if( master )
                _setChangeManager( new FullMasterCM( this ));
            else
                _setChangeManager( new VersionedSlaveCM( this, 
                                                         masterInstanceID ));
            break;

        case Object::DELTA:
            LBASSERT( _localNode );
            if( master )
                _setChangeManager( new DeltaMasterCM( this ));
            else
                _setChangeManager( new VersionedSlaveCM( this,
                                                         masterInstanceID ));
            break;

        case Object::UNBUFFERED:
            LBASSERT( _localNode );
            if( master )
                _setChangeManager( new UnbufferedMasterCM( this ));
            else
                _setChangeManager( new VersionedSlaveCM( this,
                                                         masterInstanceID ));
            break;

        default: LBUNIMPLEMENTED;
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

bool Object::isBuffered() const
{
    return _cm->isBuffered();
}

bool Object::isMaster() const
{
    return _cm->isMaster();
}

void Object::addSlave( Command& command )
{
    _cm->addSlave( command );
}

void Object::removeSlave( NodePtr node, const uint32_t instanceID )
{
    _cm->removeSlave( node, instanceID );
}

void Object::removeSlaves( NodePtr node )
{
    _cm->removeSlaves( node );
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

void Object::notifyNewHeadVersion( const uint128_t& version )
{ 
    LBASSERTINFO( getVersion() == VERSION_NONE || 
                  version < getVersion() + 100, 
                  lunchbox::className( this ));
}

uint32_t Object::chooseCompressor() const
{
    return CPUCompressor::chooseCompressor( EQ_COMPRESSOR_DATATYPE_BYTE );
}

uint32_t Object::getMasterInstanceID() const
{
    return _cm->getMasterInstanceID();
}

NodePtr Object::getMasterNode()
{
    return _cm->getMasterNode();
}

std::ostream& operator << ( std::ostream& os, const Object& object )
{
    os << lunchbox::className( &object ) << " " << object.getID() << "."
       << object.getInstanceID() << " v" << object.getVersion();
    return os;
}

}
