
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include "fullSlaveCM.h"
#include "log.h"
#include "nullCM.h"
#include "objectCM.h"
#include "packets.h"
#include "session.h"
#include "staticMasterCM.h"
#include "staticSlaveCM.h"
#include "unbufferedMasterCM.h"

#include <eq/base/scopedMutex.h>
#include <iostream>

using namespace eq::base;
using namespace std;

namespace eq
{
namespace net
{
Object::Object()
        : _session          ( 0 )
        , _id               ( EQ_ID_INVALID )
        , _instanceID       ( EQ_ID_INVALID )
        , _cm               ( ObjectCM::ZERO )
        , _threadSafe       ( false )
{
}

Object::Object( const Object& object )
        : Dispatcher( object )
        , _session          ( 0 )
        , _id               ( EQ_ID_INVALID )
        , _instanceID       ( EQ_ID_INVALID )
        , _cm               ( ObjectCM::ZERO )
        , _threadSafe       ( object._threadSafe )
{
}


Object::~Object()
{
    if( _session ) // Still registered
        EQERROR << "Object " << _id << " is still registered in session "
                << _session->getID() << " in destructor" << endl;

    if( _cm != ObjectCM::ZERO )
        delete _cm;
    _cm = 0;
}

void Object::attachToSession( const uint32_t id, const uint32_t instanceID, 
                              Session* session )
{
    EQASSERT( id != EQ_ID_INVALID );
    EQASSERT( instanceID != EQ_ID_INVALID );
    EQASSERT( session );

    _id         = id;
    _instanceID = instanceID;
    _session    = session;

    CommandQueue* queue = session->getCommandThreadQueue();

    registerCommand( CMD_OBJECT_INSTANCE_DATA,
                     CommandFunc<Object>( this, &Object::_cmdForward ), queue );
    registerCommand( CMD_OBJECT_INSTANCE,
                     CommandFunc<Object>( this, &Object::_cmdForward ), queue );
    registerCommand( CMD_OBJECT_DELTA_DATA, 
                     CommandFunc<Object>( this, &Object::_cmdForward ), queue );
    registerCommand( CMD_OBJECT_DELTA, 
                     CommandFunc<Object>( this, &Object::_cmdForward ), queue );
    registerCommand( CMD_OBJECT_COMMIT, 
                     CommandFunc<Object>( this, &Object::_cmdForward ), queue );
    registerCommand( CMD_OBJECT_NEW_MASTER, 
                     CommandFunc<Object>( this, &Object::_cmdNewMaster ),queue);
    registerCommand( CMD_OBJECT_VERSION, 
                     CommandFunc<Object>( this, &Object::_cmdForward ), queue );

    EQINFO << _id << '.' << _instanceID << ": " << typeid( *this ).name()
           << (isMaster() ? " master" : " slave") << std::endl;
}

void Object::detachFromSession()
{
    // Slave objects keep their cm to be able to sync queued versions
    if( isMaster( )) 
        _setChangeManager( ObjectCM::ZERO );

    _id         = EQ_ID_INVALID;
    _instanceID = EQ_ID_INVALID;
    _session    = 0;
}

void Object::_setChangeManager( ObjectCM* cm )
{
    if( _cm != ObjectCM::ZERO )
    {
        EQVERB << "Overriding existing object change manager, obj "
               << typeid( *this ).name() << ", old cm " 
               << typeid( *_cm ).name() << ", new cm "
               << typeid( *cm ).name() << endl;
        delete _cm;
    }

    if( _threadSafe )
        cm->makeThreadSafe();

    _cm = cm;
    EQLOG( LOG_OBJECTS ) << "set new change manager " << typeid( *cm ).name()
                         << " for " << typeid( *this ).name() << endl;
}

void Object::makeThreadSafe()
{
    EQASSERT( _id == EQ_ID_INVALID );
    if( _threadSafe )
        return;

    _threadSafe = true;
    _cm->makeThreadSafe();
}

NodePtr Object::getLocalNode()
{ 
    return _session ? _session->getLocalNode() : 0; 
}

bool Object::send( NodePtr node, ObjectPacket& packet )
{
    EQASSERT( _session ); EQASSERT( _id != EQ_ID_INVALID );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return node->send( packet );
}

bool Object::send( NodePtr node, ObjectPacket& packet, 
                   const std::string& string )
{
    EQASSERT( _session ); EQASSERT( _id != EQ_ID_INVALID );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return node->send( packet, string );
}

bool Object::send( NodePtr node, ObjectPacket& packet, 
                   const void* data, const uint64_t size )
{
    EQASSERT( _session ); EQASSERT( _id != EQ_ID_INVALID );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return node->send( packet, data, size );
}

void Object::becomeMaster()
{
    EQASSERT( _session );
    EQASSERT( !isMaster( ));
    EQASSERT( _id != EQ_ID_INVALID );

    // save location of master instance
    Session* session = _session;
    const NodeID& masterNodeID = session->getIDMaster( _id );
    EQASSERT( masterNodeID != NodeID::ZERO );

    NodePtr localNode = session->getLocalNode();
    NodePtr master    = localNode->getNode( masterNodeID );
    EQASSERT( master.isValid( ));

    const uint32_t masterID = _id;
    const uint32_t masterInstanceID = getMasterInstanceID();
    EQASSERT( masterInstanceID != EQ_ID_INVALID );

    // remap as master
    session->unmapObject( this );
    sync();
    session->registerObject( this );

    EQINFO << "became master " << masterID << '.' << masterInstanceID << " to "
           << _id << '.' << _instanceID << std::endl;

    // tell old master
    ObjectNewMasterPacket packet;
    packet.sessionID  = session->getID();
    packet.objectID   = masterID;
    packet.instanceID = masterInstanceID;
    packet.newMasterID = _id;
    packet.newMasterInstanceID = _instanceID;
    packet.changeType = getChangeType();

    master->send( packet );

    // subscribe slave (old master)
    _cm->addOldMaster( master, masterInstanceID );
}

uint32_t Object::commit()
{
    if( !isDirty( ))
        return getVersion();

    const uint32_t requestID = commitNB();
    return commitSync( requestID );
}

void Object::setupChangeManager( const Object::ChangeType type, 
                                 const bool master, 
                                 const uint32_t masterInstanceID )
{
    switch( type )
    {
        case Object::STATIC:
            if( master )
                _setChangeManager( new StaticMasterCM( this ));
            else
                _setChangeManager( new StaticSlaveCM( this ));
            break;
        case Object::INSTANCE:
            if( master )
                _setChangeManager( new FullMasterCM( this ));
            else
                _setChangeManager( new FullSlaveCM( this, masterInstanceID ));
            break;
        case Object::DELTA:
            if( master )
                _setChangeManager( new DeltaMasterCM( this ));
            else
                _setChangeManager( new FullSlaveCM( this, masterInstanceID ));
            break;
        case Object::UNBUFFERED:
            if( master )
                _setChangeManager( new UnbufferedMasterCM( this ));
            else
                _setChangeManager( new FullSlaveCM( this, masterInstanceID ));
            break;

        default: EQUNIMPLEMENTED;
    }
}

//---------------------------------------------------------------------------
// ChangeManager forwarders
//---------------------------------------------------------------------------

bool Object::isMaster() const
{
    return _cm->isMaster();
}

uint32_t Object::commitNB()
{
    return _cm->commitNB();
}

uint32_t Object::commitSync( const uint32_t commitID ) 
{
    return _cm->commitSync( commitID );
}

void Object::obsolete( const uint32_t version )
{
    _cm->obsolete( version );
}

void Object::setAutoObsolete( const uint32_t count, const uint32_t flags )
{
    _cm->setAutoObsolete( count, flags );
}

uint32_t Object::getAutoObsoleteCount() const 
{
    return _cm->getAutoObsoleteCount();
}

uint32_t Object::sync( const uint32_t version )
{
    return _cm->sync( version );
}

uint32_t Object::getHeadVersion() const
{
    return _cm->getHeadVersion();
}

uint32_t Object::getVersion() const
{
    return _cm->getVersion();
}

uint32_t Object::getOldestVersion() const
{
    return _cm->getOldestVersion();
}

uint32_t Object::getMasterInstanceID() const
{
    return _cm->getMasterInstanceID();
}

void Object::addSlave( NodePtr node, const uint32_t instanceID, 
                       const uint32_t version )
{
    _cm->addSlave( node, instanceID, version );
}

void Object::removeSlave( NodePtr node )
{
    _cm->removeSlave( node );
}

CommandResult Object::_cmdForward( Command& command )
{
    return _cm->invokeCommand( command );
}

CommandResult Object::_cmdNewMaster( Command& command )
{
    ObjectNewMasterPacket* packet = command.getPacket<ObjectNewMasterPacket>();
    EQVERB << "become slave " << _id << '.' << _instanceID << " to "
           << packet->newMasterID << '.' << packet->newMasterInstanceID
           << std::endl;
    EQASSERT( isMaster( ));

    const uint32_t instanceID = _instanceID; // save - reset during deregister
    Session* session = getSession();
    session->deregisterObject( this );

    setupChangeManager( static_cast< Object::ChangeType >( packet->changeType ),
                        false, packet->newMasterInstanceID );
    session->attachObject( this, packet->newMasterID, instanceID );

    return COMMAND_HANDLED;
}

}
}
