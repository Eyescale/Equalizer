
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "object.h"

#include "command.h"
#include "deltaMasterCM.h"
#include "deltaSlaveCM.h"
#include "fullMasterCM.h"
#include "fullSlaveCM.h"
#include "log.h"
#include "nullCM.h"
#include "packets.h"
#include "session.h"
#include "staticMasterCM.h"
#include "staticSlaveCM.h"

#include <eq/base/scopedMutex.h>
#include <iostream>

using namespace eqNet;
using namespace eqBase;
using namespace std;

void Object::_construct()
{
    _cm            = ObjectCM::ZERO;
    _session       = 0;
    _id            = EQ_ID_INVALID;
    _instanceID    = EQ_ID_INVALID;
    _instanceData     = 0;
    _instanceDataSize = 0;
    _deltaData     = 0;
    _deltaDataSize = 0;

    registerCommand( CMD_OBJECT_INSTANCE_DATA,
                     CommandFunc<Object>( this, &Object::_cmdForward ));
    registerCommand( CMD_OBJECT_DELTA_DATA, 
                     CommandFunc<Object>( this, &Object::_cmdForward ));
    registerCommand( CMD_OBJECT_COMMIT, 
                     CommandFunc<Object>( this, &Object::_cmdForward ));
}

Object::Object()
{
    _construct();
}

Object::Object( const Object& from )
{
    EQASSERT( from._id == EQ_ID_INVALID );

    _construct();
    _instanceData     = from._instanceData;
    _instanceDataSize = from._instanceDataSize;
    _deltaData        = from._deltaData;
    _deltaDataSize    = from._deltaDataSize;
}

Object::~Object()
{
    if( _session ) // Still registered
        EQERROR << "Object is still registered in session " << _session->getID()
                << " in destructor" << endl;

    if( _cm != ObjectCM::ZERO )
        delete _cm;
    _cm = 0;
}

void Object::_setChangeManager( ObjectCM* cm )
{
    if( _cm != ObjectCM::ZERO )
    {
        EQASSERTINFO( cm == ObjectCM::ZERO, 
                      "Overriding existing object change manager, obj "
                      << typeid( *this ).name() << ", old cm " 
                      << typeid( *_cm ).name( ));
        delete _cm;
    }

    if( _threadSafe )
        cm->makeThreadSafe();

    _cm = cm;
}

void Object::makeThreadSafe()
{
    EQASSERT( _id == EQ_ID_INVALID );
    if( _threadSafe ) return;

    _threadSafe = true;
    _cm->makeThreadSafe();
}

RefPtr<Node> Object::getLocalNode()
{ 
    return _session ? _session->getLocalNode() : 0; 
}

bool Object::send( eqBase::RefPtr<Node> node, ObjectPacket& packet )
{
    EQASSERT( _session ); EQASSERT( _id != EQ_ID_INVALID );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return node->send( packet );
}

bool Object::send( eqBase::RefPtr<Node> node, ObjectPacket& packet, 
                   const std::string& string )
{
    EQASSERT( _session ); EQASSERT( _id != EQ_ID_INVALID );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return node->send( packet, string );
}

bool Object::send( eqBase::RefPtr<Node> node, ObjectPacket& packet, 
                   const void* data, const uint64_t size )
{
    EQASSERT( _session ); EQASSERT( _id != EQ_ID_INVALID );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return node->send( packet, data, size );
}

bool Object::send( NodeVector& nodes, ObjectPacket& packet )
{
    EQASSERT( _session ); EQASSERT( _id != EQ_ID_INVALID );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return Connection::send( nodes, packet );
}
bool Object::send( NodeVector nodes, ObjectPacket& packet, const void* data,
                   const uint64_t size )
{
    EQASSERT( _session ); EQASSERT( _id != EQ_ID_INVALID );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;
    return Connection::send( nodes, packet, data, size );
}

uint32_t Object::commit()
{
    const uint32_t requestID = commitNB();
    return commitSync( requestID );
}

ObjectCM::Type Object::getChangeManagerType() const
{
    if( isStatic( ))
        return ObjectCM::STATIC;

    if ( _instanceData && 
         _instanceData == _deltaData && _instanceDataSize == _deltaDataSize )

        return ObjectCM::FULL;

    return ObjectCM::DELTA;
}

void Object::setupChangeManager( const ObjectCM::Type type, const bool master )
{
    switch( type )
    {
        case ObjectCM::STATIC:
            if( master )
                _setChangeManager( new StaticMasterCM( this ));
            else
                _setChangeManager( new StaticSlaveCM( this ));
            break;
        case ObjectCM::FULL:
            if( master )
                _setChangeManager( new FullMasterCM( this ));
            else
                _setChangeManager( new FullSlaveCM( this ));
            break;
        case ObjectCM::DELTA:
            if( master )
                _setChangeManager( new DeltaMasterCM( this ));
            else
                _setChangeManager( new DeltaSlaveCM( this ));
            break;

        default: EQUNIMPLEMENTED;
    }
}

void Object::setInstanceData( void* data, const uint64_t size )
{
    _instanceData     = data;
    _instanceDataSize = size;

    if( _deltaData )
        return;

    _deltaData     = data;
    _deltaDataSize = size;
}
