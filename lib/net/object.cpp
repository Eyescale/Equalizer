
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
#include "nullCM.h"
#include "objectCM.h"
#include "session.h"
#include "staticMasterCM.h"
#include "staticSlaveCM.h"
#include "unbufferedMasterCM.h"

#include <eq/base/scopedMutex.h>
#include <iostream>

namespace eq
{
namespace net
{
Object::Object()
        : _session          ( 0 )
        , _id               ( EQ_ID_INVALID )
        , _instanceID       ( EQ_ID_INVALID )
        , _cm               ( ObjectCM::ZERO )
{
}

Object::Object( const Object& object )
        : Dispatcher( object )
        , _session          ( 0 )
        , _id               ( EQ_ID_INVALID )
        , _instanceID       ( EQ_ID_INVALID )
        , _cm               ( ObjectCM::ZERO )
{
}


Object::~Object()
{
    EQASSERTINFO( !_session,
                  "Object " << _id << " is still registered in session " <<
                  _session->getID() << " in destructor" );
    if( _session )
        _session->releaseObject( this );
    if( _cm != ObjectCM::ZERO )
        delete _cm;
    _cm = 0;
}

typedef CommandFunc<Object> CmdFunc;

void Object::attachToSession( const uint32_t id, const uint32_t instanceID, 
                              Session* session )
{
    EQASSERT( id <= EQ_ID_MAX );
    EQASSERT( instanceID <= EQ_ID_MAX );
    EQASSERT( session );

    _id         = id;
    _instanceID = instanceID;
    _session    = session;

    CommandQueue* queue = session->getCommandThreadQueue();

    registerCommand( CMD_OBJECT_INSTANCE,
                     CmdFunc( this, &Object::_cmdForward ), queue );
    registerCommand( CMD_OBJECT_DELTA, 
                     CmdFunc( this, &Object::_cmdForward ), queue );
    registerCommand( CMD_OBJECT_SLAVE_DELTA,
                     CmdFunc( this, &Object::_cmdForward ), queue );
    registerCommand( CMD_OBJECT_COMMIT, 
                     CmdFunc( this, &Object::_cmdForward ), queue );

    EQLOG( LOG_OBJECTS ) << _id << '.' << _instanceID << ": " 
                         << base::className( this )
                         << (isMaster() ? " master" : " slave") << std::endl;
}

void Object::detachFromSession()
{
    _id         = EQ_ID_INVALID;
    _instanceID = EQ_ID_INVALID;
    _session    = 0;
}

bool Object::dispatchCommand( Command& command )
{
    EQASSERT( isAttached( ));
    command.setDispatchID( _instanceID );
    return Dispatcher::dispatchCommand( command );
}

void Object::_setChangeManager( ObjectCM* cm )
{
    if( _cm != ObjectCM::ZERO )
    {
        EQVERB
            << "Overriding existing object change manager, obj "
            << base::className( this ) << ", old cm " << base::className( _cm )
            << ", new cm " << base::className( cm ) << std::endl;
        delete _cm;
    }

    _cm = cm;
    cm->init();
    EQLOG( LOG_OBJECTS ) << "set new change manager " << base::className( cm )
                         << " for " << base::className( this ) << std::endl;
}

const Nodes* Object::_getSlaveNodes() const
{
    return _cm->getSlaveNodes();
}

NodePtr Object::getLocalNode()
{ 
    return _session ? _session->getLocalNode() : 0; 
}

bool Object::send( NodePtr node, ObjectPacket& packet )
{
    EQASSERT( _session );
    EQASSERT( isAttached( ));
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return node->send( packet );
}

bool Object::send( NodePtr node, ObjectPacket& packet, 
                   const std::string& string )
{
    EQASSERT( _session ); EQASSERT( _id <= EQ_ID_MAX );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return node->send( packet, string );
}

bool Object::send( NodePtr node, ObjectPacket& packet, 
                   const void* data, const uint64_t size )
{
    EQASSERT( _session ); EQASSERT( _id <= EQ_ID_MAX );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return node->send( packet, data, size );
}

uint32_t Object::commit()
{
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
                _setChangeManager( new VersionedSlaveCM( this, 
                                                         masterInstanceID ));
            break;
        case Object::DELTA:
            if( master )
                _setChangeManager( new DeltaMasterCM( this ));
            else
                _setChangeManager( new VersionedSlaveCM( this,
                                                         masterInstanceID ));
            break;
        case Object::UNBUFFERED:
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

bool Object::isMaster() const
{
    return _cm->isMaster();
}

uint32_t Object::commitNB()
{
    if( !isDirty( ))
    {
        _cm->increaseCommitCount();
        return EQ_ID_INVALID;
    }
    return _cm->commitNB();
}

uint32_t Object::commitSync( const uint32_t commitID ) 
{
    if( commitID == EQ_ID_INVALID )
        return getVersion();

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

bool Object::_cmdForward( Command& command )
{
    return _cm->invokeCommand( command );
}

std::ostream& operator << ( std::ostream& os, const Object& object )
{
    os << base::className( &object ) << " " << object.getID() << "."
       << object.getInstanceID() << " v" << object.getVersion();
    return os;
}

}
}
