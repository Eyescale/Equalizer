
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "unbufferedMasterCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "node.h"
#include "object.h"
#include "objectDeltaDataOStream.h"
#include "packets.h"
#include "session.h"

using namespace eq::base;
using namespace std;

namespace eq
{
namespace net
{
UnbufferedMasterCM::UnbufferedMasterCM( Object* object )
        : _object( object )
        , _version( Object::VERSION_NONE )
{
}

UnbufferedMasterCM::~UnbufferedMasterCM()
{
    if( !_slaves.empty( ))
        EQWARN << _slaves.size() 
               << " slave nodes subscribed during deregisterObject" << endl;
    _slaves.clear();
}

void UnbufferedMasterCM::notifyAttached()
{
    Session* session = _object->getSession();
    EQASSERT( session );
    CommandQueue* queue = session->getCommandThreadQueue();

    registerCommand( CMD_OBJECT_COMMIT, 
       CommandFunc<UnbufferedMasterCM>( this, &UnbufferedMasterCM::_cmdCommit ),
                     queue );
    // sync commands are send to any instance, even the master gets the command
	registerCommand( CMD_OBJECT_DELTA_DATA, 
	  CommandFunc<UnbufferedMasterCM>( this, &UnbufferedMasterCM::_cmdDiscard ),
                     queue );
	registerCommand( CMD_OBJECT_DELTA, 
      CommandFunc<UnbufferedMasterCM>( this, &UnbufferedMasterCM::_cmdDiscard ),
                     queue );
}

uint32_t UnbufferedMasterCM::commitNB()
{
    EQASSERTINFO( _object->getChangeType() == Object::DELTA_UNBUFFERED,
                  "Object type " << typeid(*this).name( ));

    ObjectCommitPacket packet;
    packet.instanceID = _object->_instanceID;
    packet.requestID  = _requestHandler.registerRequest();

    _object->send( _object->getLocalNode(), packet );
    return packet.requestID;
}

uint32_t UnbufferedMasterCM::commitSync( const uint32_t commitID )
{
    uint32_t version = Object::VERSION_NONE;
    _requestHandler.waitRequest( commitID, version );
    return version;
}

void UnbufferedMasterCM::addSlave( NodePtr node, const uint32_t instanceID)
{
    // add to subscribers
    ++_slavesCount[ node->getNodeID() ];
    _slaves.push_back( node );
    stde::usort( _slaves );

    EQLOG( LOG_OBJECTS ) << "Object id " << _object->_id << " v" << _version
                         << ", instantiate on " << node->getNodeID() << endl;

    // send instance data
    ObjectInstanceDataOStream os( _object );
    os.setVersion( _version );
    os.setInstanceID( instanceID );
    os.enable( node );
    
    _object->getInstanceData( os );

    os.disable();

    if( !os.hasSentData( )) // if no instance data, send packet to set version
    {
        os.enable( node );
        os.writeOnce( 0, 0 );
        os.disable();
    }
}

void UnbufferedMasterCM::removeSlave( NodePtr node )
{
    // remove from subscribers
    const NodeID& nodeID = node->getNodeID();
    EQASSERT( _slavesCount[ nodeID ] != 0 );

    --_slavesCount[ nodeID ];
    if( _slavesCount[ nodeID ] == 0 )
    {
        NodeVector::iterator i = find( _slaves.begin(), _slaves.end(), node );
        EQASSERT( i != _slaves.end( ));
        _slaves.erase( i );
        _slavesCount.erase( nodeID );
    }
}


//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
CommandResult UnbufferedMasterCM::_cmdCommit( Command& command )
{
    const ObjectCommitPacket* packet = command.getPacket<ObjectCommitPacket>();
    EQLOG( LOG_OBJECTS ) << "commit v" << _version << " " << command << endl;
    if( _slaves.empty( ))
    {
        _requestHandler.serveRequest( packet->requestID, _version );
        return COMMAND_HANDLED;
    }

    ObjectDeltaDataOStream os( _object );
    os.setVersion( _version + 1 );
    os.enable( _slaves );
    _object->pack( os );
    os.disable();

    if( os.hasSentData( ))
    {
        ++_version;
        EQASSERT( _version );
        EQLOG( LOG_OBJECTS ) << "Committed v" << _version << ", id " 
                             << _object->getID() << endl;
    }

    _requestHandler.serveRequest( packet->requestID, _version );
    return COMMAND_HANDLED;
}
}
}
