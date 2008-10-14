
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "object.h"

#include "command.h"
#include "dataIStream.h"
#include "dataOStream.h"
#include "deltaMasterCM.h"
#include "fullMasterCM.h"
#include "fullSlaveCM.h"
#include "log.h"
#include "nullCM.h"
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
#ifdef EQ_USE_DEPRECATED
        , _instanceData     ( 0 )
        , _instanceDataSize ( 0 )
        , _deltaData        ( 0 )
        , _deltaDataSize    ( 0 )
#endif
        , _cm               ( ObjectCM::ZERO )
        , _threadSafe       ( false )
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

    _cm->notifyAttached();

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
    if( _threadSafe ) return;

    _threadSafe = true;
    _cm->makeThreadSafe();
}

NodePtr Object::getLocalNode()
{ 
    return _session ? _session->getLocalNode() : 0; 
}

#ifdef EQ_USE_DEPRECATED
void Object::getInstanceData( DataOStream& ostream )
{
    if( !_instanceData || _instanceDataSize == 0 )
        return;

    ostream.writeOnce( _instanceData, _instanceDataSize ); 
}

void Object::applyInstanceData( DataIStream& is )
{
    const uint64_t size = is.getRemainingBufferSize();
    EQASSERT( size == _instanceDataSize ); 

	if( size == 0 )
		return;

	const void*    data = is.getRemainingBuffer();

	EQASSERTINFO( is.nRemainingBuffers() == 0, 
		"Master instance did not use default getInstanceData(), "
		<< "can't use default applyInstanceData()" );
	EQASSERT( data && size > 0 );
    EQASSERT( _instanceData );

    memcpy( _instanceData, data, size );
    is.advanceBuffer( size );
}

void Object::pack( DataOStream& ostream )
{
    if( !_deltaData || _deltaDataSize == 0 )
        return;

    ostream.writeOnce( _deltaData, _deltaDataSize ); 
}

void Object::unpack( DataIStream& is )
{
    EQASSERTINFO( is.nRemainingBuffers() == 1, 
                  "Master instance did not use default Object::pack(), "
                  << "can't use default Object::unpack()" );

    const uint64_t size = is.getRemainingBufferSize();
    const void*    data = is.getRemainingBuffer();

    EQASSERT( data && size > 0 );
    EQASSERT( size == _deltaDataSize ); 
    EQASSERT( _deltaData );

    memcpy( _deltaData, data, size );
    is.advanceBuffer( size );
}
#endif

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

bool Object::send( NodeVector nodes, ObjectPacket& packet, const void* data,
                   const uint64_t size )
{
    EQASSERT( _session ); EQASSERT( _id != EQ_ID_INVALID );
    packet.sessionID = _session->getID();
    packet.objectID  = _id;

    ConnectionVector connections;
    for( NodeVector::const_iterator i = nodes.begin(); i != nodes.end(); ++i )
        connections.push_back( (*i)->getConnection( ));

    return Connection::send( connections, packet, data, size );
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
                _setChangeManager( new FullSlaveCM( this, masterInstanceID ));
            break;
        case Object::DELTA:
            if( master )
                _setChangeManager( new DeltaMasterCM( this ));
            else
                _setChangeManager( new FullSlaveCM( this, masterInstanceID ));
            break;
        case Object::DELTA_UNBUFFERED:
            if( master )
                _setChangeManager( new UnbufferedMasterCM( this ));
            else
                _setChangeManager( new FullSlaveCM( this, masterInstanceID ));
            break;

        default: EQUNIMPLEMENTED;
    }
}

#ifdef EQ_USE_DEPRECATED
void Object::setInstanceData( void* data, const uint64_t size )
{
    _instanceData     = data;
    _instanceDataSize = size;

    if( _deltaData )
        return;

    _deltaData     = data;
    _deltaDataSize = size;
}
#endif
}
}
