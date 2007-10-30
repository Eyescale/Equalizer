
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "deltaMasterCM.h"

#include "command.h"
#include "commands.h"
#include "log.h"
#include "node.h"
#include "object.h"
#include "objectDeltaDataOStream.h"
#include "packets.h"

using namespace eqBase;
using namespace std;

namespace eqNet
{
DeltaMasterCM::DeltaMasterCM( Object* object )
        : _object( object ),
          _version( Object::VERSION_NONE ),
          _commitCount( 0 ),
          _nVersions( 0 ),
          _obsoleteFlags( Object::AUTO_OBSOLETE_COUNT_VERSIONS )
{
    registerCommand( CMD_OBJECT_COMMIT, 
                CommandFunc<DeltaMasterCM>( this, &DeltaMasterCM::_cmdCommit ));
    // sync commands are send to any instance, even the master gets the command
	registerCommand( CMD_OBJECT_DELTA_DATA, 
		CommandFunc<DeltaMasterCM>( this, &DeltaMasterCM::_cmdDiscard ));
	registerCommand( CMD_OBJECT_DELTA, 
		CommandFunc<DeltaMasterCM>( this, &DeltaMasterCM::_cmdDiscard ));
}

DeltaMasterCM::~DeltaMasterCM()
{
    if( !_slaves.empty( ))
        EQWARN << _slaves.size() 
               << " slave nodes subscribed during deregisterObject" << endl;


    for( std::deque< InstanceData* >::const_iterator i = _instanceDatas.begin();
         i != _instanceDatas.end(); ++i )

        delete *i;

    _instanceDatas.clear();

    for( std::deque< DeltaData* >::const_iterator i = _deltaDatas.begin();
         i != _deltaDatas.end(); ++i )

        delete *i;

    _deltaDatas.clear();

    for(std::vector<InstanceData*>::const_iterator i=_instanceDataCache.begin();
         i != _instanceDataCache.end(); ++i )

        delete *i;
    
    _instanceDataCache.clear();

    for( std::vector<DeltaData*>::const_iterator i =_deltaDataCache.begin();
         i != _deltaDataCache.end(); ++i )

        delete *i;
    
    _deltaDataCache.clear();
}

uint32_t DeltaMasterCM::commitNB()
{
    EQASSERTINFO( !_object->isStatic( ), 
                  "Object type " << typeid(*this).name( ));

    ObjectCommitPacket packet;
    packet.instanceID = _object->_instanceID;
    packet.requestID  = _requestHandler.registerRequest();

    _object->send( _object->getLocalNode(), packet );
    return packet.requestID;
}

uint32_t DeltaMasterCM::commitSync( const uint32_t commitID )
{
    uint32_t version = Object::VERSION_NONE;
    _requestHandler.waitRequest( commitID, version );
    return version;
}

uint32_t DeltaMasterCM::_commitInitial()
{
    CHECK_THREAD( _thread );
    EQASSERT( _slaves.empty( ));
    EQASSERT( _version == Object::VERSION_NONE );
    EQASSERT( _instanceDatas.empty( ));
    EQASSERT( _deltaDatas.empty( ));

    InstanceData* data = _newInstanceData();
    data->os.setVersion( 1 );
    data->os.enableSave();

    data->os.enable();
    _object->getInstanceData( data->os );
    data->os.disable();
        
    _instanceDatas.push_front( data );
    ++_version;
    ++_commitCount;
    return _version;
}

// Obsoletes old changes based on number of commits or number of versions,
// depending on the obsolete flags.
void DeltaMasterCM::_obsolete()
{
    if( _obsoleteFlags & Object::AUTO_OBSOLETE_COUNT_COMMITS )
    {
        InstanceData* lastInstanceData = _instanceDatas.back();
        if( lastInstanceData->commitCount < (_commitCount - _nVersions) &&
            _instanceDatas.size() > 1 )
        {
            EQASSERT( !_deltaDatas.empty( ));
            _instanceDataCache.push_back( lastInstanceData );
            _instanceDatas.pop_back();

            _deltaDataCache.push_back( _deltaDatas.back( ));        
            _deltaDatas.pop_back();
        }
        _checkConsistency();
        return;
    }
    // else count versions
    while( _instanceDatas.size() > (_nVersions+1) )
    {
        _instanceDataCache.push_back( _instanceDatas.back( ));
        _instanceDatas.pop_back();
        
        if( _nVersions > 0 )
        {
            EQASSERT( !_deltaDatas.empty( ));
            _deltaDataCache.push_back( _deltaDatas.back( ));        
            _deltaDatas.pop_back();
        }
        _checkConsistency();
    }
}

void DeltaMasterCM::addSlave( RefPtr<Node> node, const uint32_t instanceID )
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

    // send initial instance data
    InstanceData* data = _instanceDatas.back();
    EQASSERT( data );
    
    data->os.setInstanceID( instanceID );
    data->os.resend( node );

    // send all deltas
    for( deque< DeltaData* >::reverse_iterator i = _deltaDatas.rbegin();
         i != _deltaDatas.rend(); ++i )
    {
        DeltaData* data = *i;
        data->setInstanceID( instanceID );
        data->resend( node );
    }
}

void DeltaMasterCM::removeSlave( RefPtr<Node> node )
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

void DeltaMasterCM::_checkConsistency() const
{
#ifndef NDEBUG
    EQASSERT( _object->_id != EQ_ID_INVALID );
    EQASSERT( !_object->isStatic( ));
    if( _version == Object::VERSION_NONE )
        return;

    EQASSERT( _instanceDatas.size() == _deltaDatas.size() + 1 );

    if( !( _obsoleteFlags & Object::AUTO_OBSOLETE_COUNT_COMMITS ))
    {   // count versions
        if( _version > _nVersions )
        {
            EQASSERT( _deltaDatas.size() == _nVersions );
        }
        else
        {
            EQASSERT( _instanceDatas.size() == _version );
        }
    }

    uint32_t version = _version;
    for( deque< InstanceData* >::const_iterator i = _instanceDatas.begin();
         i != _instanceDatas.end(); ++i )
    {
        const InstanceData* data = *i;
        EQASSERT( data->os.getVersion() == version );
        EQASSERT( data->os.getVersion() > 0 );
        --version;
    }

    version = _version;
    for( deque< DeltaData* >::const_iterator i = _deltaDatas.begin();
         i != _deltaDatas.end(); ++i )
    {
        const DeltaData* data = *i;
        EQASSERT( data->getVersion() == version );
        EQASSERT( data->getVersion() > 0 );
        --version;
    }
#endif
}

//---------------------------------------------------------------------------
// cache handling
//---------------------------------------------------------------------------
DeltaMasterCM::InstanceData* DeltaMasterCM::_newInstanceData()
{
    InstanceData* instanceData;

    if( _instanceDataCache.empty( ))
        instanceData = new InstanceData( _object );
    else
    {
        instanceData = _instanceDataCache.back();
        _instanceDataCache.pop_back();
    }

    instanceData->os.disableSave();
    instanceData->os.enableBuffering();
    instanceData->os.setInstanceID( EQ_ID_ANY );
    return instanceData;
}

DeltaMasterCM::DeltaData* DeltaMasterCM::_newDeltaData()
{
    DeltaData* deltaData;

    if( _deltaDataCache.empty( ))
        deltaData = new DeltaData( _object );
    else
    {
        deltaData = _deltaDataCache.back();
        _deltaDataCache.pop_back();
    }

    deltaData->disableSave();
    deltaData->enableBuffering();
    deltaData->setInstanceID( EQ_ID_ANY );
    return deltaData;
}


//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
CommandResult DeltaMasterCM::_cmdCommit( Command& command )
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

    EQASSERT( _instanceDatas.size() == _deltaDatas.size() + 1 );

    ++_commitCount;

    DeltaData* deltaData = _newDeltaData();
    const bool  saveDelta  = ( _nVersions > 0 );
    if( saveDelta ) deltaData->enableSave();

    deltaData->setVersion( _version + 1 );
    deltaData->enable( _slaves );
    _object->pack( *deltaData );
    deltaData->disable();

    if( !deltaData->hasSentData( ))
    {
        _deltaDataCache.push_back( deltaData );
        _obsolete();
        _checkConsistency();

        _requestHandler.serveRequest( packet->requestID, _version );
        return COMMAND_HANDLED;
    }

    ++_version;
    EQASSERT( _version );
    
    if( saveDelta )
        _deltaDatas.push_front( deltaData );
    else
        _deltaDataCache.push_back( deltaData );

    // save instance data
    InstanceData* instanceData = _newInstanceData();
    instanceData->os.enableSave();
    instanceData->os.setVersion( _version );

    instanceData->os.enable();
    _object->getInstanceData( instanceData->os );
    instanceData->os.disable();

    instanceData->commitCount = _commitCount;
    _instanceDatas.push_front( instanceData );
    
    _obsolete();
    _checkConsistency();

    EQLOG( LOG_OBJECTS ) << "Committed v" << _version << ", id " 
                         << _object->getID() << endl;
    _requestHandler.serveRequest( packet->requestID, _version );
    return COMMAND_HANDLED;
}
}
