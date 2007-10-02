
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "fullMasterCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "node.h"
#include "object.h"
#include "packets.h"

using namespace eqNet;
using namespace eqBase;
using namespace std;

FullMasterCM::FullMasterCM( Object* object )
        : _object( object ),
          _version( Object::VERSION_NONE ),
          _commitCount( 0 ),
          _nVersions( 0 ),
          _obsoleteFlags( Object::AUTO_OBSOLETE_COUNT_VERSIONS )
{
    registerCommand( CMD_OBJECT_COMMIT, 
                  CommandFunc<FullMasterCM>( this, &FullMasterCM::_cmdCommit ));
    // sync commands are send to any instance, even the master gets the command
    registerCommand( CMD_OBJECT_DELTA_DATA,
                 CommandFunc<FullMasterCM>( this, &FullMasterCM::_cmdDiscard ));
    registerCommand( CMD_OBJECT_DELTA,
                 CommandFunc<FullMasterCM>( this, &FullMasterCM::_cmdDiscard ));
}

FullMasterCM::~FullMasterCM()
{
    if( !_slaves.empty( ))
        EQWARN << _slaves.size() 
               << " slave nodes subscribed during deregisterObject" << endl;


    for( std::deque< DeltaData* >::const_iterator i = _deltaDatas.begin();
         i != _deltaDatas.end(); ++i )

        delete *i;

    _deltaDatas.clear();

    for(std::vector<DeltaData*>::const_iterator i=_deltaDataCache.begin();
         i != _deltaDataCache.end(); ++i )

        delete *i;
    
    _deltaDataCache.clear();
}

uint32_t FullMasterCM::commitNB()
{
    EQASSERTINFO( !_object->isStatic( ), 
                  "Object type " << typeid(*this).name( ));

    ObjectCommitPacket packet;
    packet.instanceID = _object->_instanceID;
    packet.requestID  = _requestHandler.registerRequest();

    _object->send( _object->getLocalNode(), packet );
    return packet.requestID;
}

uint32_t FullMasterCM::commitSync( const uint32_t commitID )
{
    uint32_t version = Object::VERSION_NONE;
    _requestHandler.waitRequest( commitID, version );
    return version;
}

uint32_t FullMasterCM::_commitInitial()
{
    CHECK_THREAD( _thread );
    EQASSERT( _slaves.empty( ));
    EQASSERT( _version == Object::VERSION_NONE );
    EQASSERT( _deltaDatas.empty( ));

    DeltaData* data = _newDeltaData();
    data->os.setVersion( 1 );

    data->os.enable();
    _object->getInstanceData( data->os );
    data->os.disable();
        
    _deltaDatas.push_front( data );
    ++_version;
    ++_commitCount;
    return _version;
}

// Obsoletes old changes based on number of commits or number of versions,
// depending on the obsolete flags.
void FullMasterCM::_obsolete()
{
    if( _obsoleteFlags & Object::AUTO_OBSOLETE_COUNT_COMMITS )
    {
        DeltaData* lastDeltaData = _deltaDatas.back();
        if( lastDeltaData->commitCount < (_commitCount - _nVersions) &&
            _deltaDatas.size() > 1 )
        {
            _deltaDataCache.push_back( lastDeltaData );
            _deltaDatas.pop_back();
        }
        _checkConsistency();
        return;
    }
    // else count versions
    while( _deltaDatas.size() > (_nVersions+1) )
    {
        _deltaDataCache.push_back( _deltaDatas.back( ));
        _deltaDatas.pop_back();
        _checkConsistency();
    }
}

void FullMasterCM::addSlave( RefPtr<Node> node, const uint32_t instanceID )
{
    if( _version == Object::VERSION_NONE )
        _commitInitial();
    _checkConsistency();

    // add to subscribers
    ++_slavesCount[ node->getNodeID() ];
    _slaves.push_back( node );
    stde::usort( _slaves );

    EQLOG( LOG_OBJECTS ) << "Object id " << _object->_id << " v" << _version
                         << ", instanciate on " << node->getNodeID() << endl;

    EQASSERT( !_object->isStatic( ))

    deque< DeltaData* >::reverse_iterator i = _deltaDatas.rbegin();
    const DeltaData* data = *i;
         
    // first packet has to be an instance packet to be applied immediately
    const Buffer&        buffer    = data->os.getSaveBuffer();
    ObjectInstancePacket instPacket;
    instPacket.instanceID = instanceID;
    instPacket.dataSize   = buffer.size;
    instPacket.version    = data->os.getVersion();

    _object->send( node, instPacket, buffer.data, buffer.size );

    // versions oldest-1..newest are delta packets
    for( ++i; i != _deltaDatas.rend(); ++i )
    {
        DeltaData* data = *i;
        data->os.setInstanceID( instanceID );
        data->os.resend( node );
        EQASSERT( ++instPacket.version == data->os.getVersion( ));
    }
    EQASSERT( instPacket.version == _version );
}

void FullMasterCM::removeSlave( RefPtr<Node> node )
{
    _checkConsistency();

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

void FullMasterCM::_checkConsistency() const
{
#ifndef NDEBUG
    EQASSERT( _object->_id != EQ_ID_INVALID );
    EQASSERT( !_object->isStatic( ));

    if( _version == Object::VERSION_NONE )
        return;

    if( !( _obsoleteFlags & Object::AUTO_OBSOLETE_COUNT_COMMITS ))
    {   // count versions
        if( _version <= _nVersions )
            EQASSERT( _deltaDatas.size() == _version );
    }
#endif
}

//---------------------------------------------------------------------------
// cache handling
//---------------------------------------------------------------------------
FullMasterCM::DeltaData* FullMasterCM::_newDeltaData()
{
    DeltaData* deltaData;

    if( _deltaDataCache.empty( ))
        deltaData = new DeltaData( _object );
    else
    {
        deltaData = _deltaDataCache.back();
        _deltaDataCache.pop_back();
    }

    deltaData->os.enableSave();
    deltaData->os.enableBuffering();
    deltaData->os.setInstanceID( EQ_ID_ANY );
    return deltaData;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
CommandResult FullMasterCM::_cmdCommit( Command& command )
{
    const ObjectCommitPacket* packet = command.getPacket<ObjectCommitPacket>();
    EQLOG( LOG_OBJECTS ) << "commit v" << _version << " " << command << endl;

    if( _version == Object::VERSION_NONE )
    {
        EQASSERT( _slaves.empty( ));

        _commitInitial();
        _checkConsistency();

        _requestHandler.serveRequest( packet->requestID, _version );
        return COMMAND_HANDLED;
    }

    ++_commitCount;

    DeltaData* deltaData = _newDeltaData();

    deltaData->commitCount = _commitCount;
    deltaData->os.setVersion( _version + 1 );

    deltaData->os.enable( _slaves );
    _object->getInstanceData( deltaData->os );
    deltaData->os.disable();

    if( deltaData->os.hasSentData( ))
    {
        ++_version;
        EQASSERT( _version );
    
        _deltaDatas.push_front( deltaData );
        EQLOG( LOG_OBJECTS ) << "Committed v" << _version << ", id " 
                             << _object->getID() << endl;
    }
    else
        _deltaDataCache.push_back( deltaData );

    _obsolete();
    _checkConsistency();
    _requestHandler.serveRequest( packet->requestID, _version );
    return COMMAND_HANDLED;
}
