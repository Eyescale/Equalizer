
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "object.h"

#include "packets.h"
#include "session.h"

#include <eq/base/log.h>
#include <iostream>

using namespace eqNet;
using namespace eqBase;
using namespace std;

void Object::_construct()
{
    _session      = NULL;
    _id           = EQ_INVALID_ID;
    _version      = 0;
    _commitCount  = 0;
    _nVersions    = 0;
    _countCommits = false;

    registerCommand( CMD_OBJECT_SYNC, this, reinterpret_cast<CommandFcn>(
                         &eqNet::Object::_cmdSync ));
    registerCommand( REQ_OBJECT_SYNC, this, reinterpret_cast<CommandFcn>(
                         &eqNet::Object::_reqSync ));
}

Object::Object( const uint32_t typeID, 
                const uint32_t nCommands )
        : Base( nCommands ),
          _typeID( typeID )
{
    EQASSERT( nCommands >= CMD_OBJECT_CUSTOM );
    _construct();
}

Object::Object( const Object& from )
        : Base( from.getNCommands( )),
          _typeID( from._typeID )
{
    _construct();
}

Object::~Object()
{
    for( list<ChangeData>::iterator iter = _instanceData.begin(); 
         iter != _instanceData.end(); ++iter )
    {
        if( (*iter).data )
            free( (*iter).data );
    }
    for( list<ChangeData>::iterator iter = _changeData.begin(); 
         iter != _changeData.end(); ++iter )
    {
        if( (*iter).data )
            free( (*iter).data );
    }
}

void Object::instanciateOnNode( RefPtr<Node> node )
{
    EQASSERT( _session );
    EQASSERT( _id != EQ_INVALID_ID );

    addSlave( node );

    SessionInstanciateObjectPacket packet( _session->getID( ));
    packet.objectID       = _id;
    packet.objectType     = _typeID;
    packet.objectDataSize = 0;
    packet.isMaster       = !_master;

    if( _typeID < TYPE_VERSIONED )
    {
        const void* data = getInstanceData( &packet.objectDataSize );
        node->send( packet, data, packet.objectDataSize );
        releaseInstanceData( data );
    }
    else
    {
        if( _version == 0 )
            commit();

        ChangeData& data = _instanceData.front();
        packet.objectDataSize = data.size;
        node->send( packet, data.data, packet.objectDataSize );
    }
}

uint32_t Object::commit()
{
    if( !isMaster( ) || _typeID < TYPE_VERSIONED )
        return 0;

    // build initial version
    if( _version == 0 )
    {
        EQASSERT( _instanceData.empty( ));
        EQASSERT( _changeData.empty( ));

        // compute base version
        ChangeData  data = { 0 };
        const void* ptr  = getInstanceData( &data.size );
        
        if( ptr )
        {
            data.data    = malloc( data.size );
            data.maxSize = data.size;
            memcpy( data.data, ptr, data.size );
        }
        _instanceData.push_front( data );
        releaseInstanceData( ptr );
        
        ++_version;
        ++_commitCount;
        return _version;
    }

    EQASSERT( _instanceData.size() == _changeData.size() + 1 );

    uint64_t deltaSize;
    const void* delta = pack( &deltaSize );

    ++_commitCount;

    ChangeData instanceData = { 0 };
    ChangeData changeData   = { 0 };

    if( _countCommits ) // auto-obsoletion based on # of commits
    {
        ChangeData& lastInstanceData = _instanceData.back();
        if( lastInstanceData.commitCount < (_commitCount - _nVersions) &&
            _instanceData.size() > 1 )
        {
            instanceData = lastInstanceData;
            changeData   = _changeData.back();

            _instanceData.pop_back();
            _changeData.pop_back();
        }
    }

    if( !delta )
        return _version;

    ++_version;
    EQASSERT( _version );
    
    // auto-obsoletion based on # of versions
    if( !_countCommits )
    {
        while( _instanceData.size() > _nVersions )
        {
            EQASSERT( !_instanceData.empty( ));
            instanceData = _instanceData.back();
            _instanceData.pop_back();

            if( _nVersions > 0 )
            {
                EQASSERT( !_changeData.empty( ));
                changeData = _changeData.back();
                _changeData.pop_back();
            }
        }
    }

    // save delta and instance data
    if( _nVersions > 0 )
    {
        if( deltaSize > changeData.maxSize )
        {
            if( changeData.data )
                free( changeData.data );
            
            changeData.data    = malloc( deltaSize );
            changeData.maxSize = deltaSize;
        }
        memcpy( changeData.data, delta, deltaSize );
        changeData.size        = deltaSize;
        changeData.commitCount = _commitCount;
        _changeData.push_front( changeData );
    }

    const void* ptr = getInstanceData( &instanceData.size );
    if( instanceData.size > instanceData.maxSize )
    {
        if( instanceData.data )
            free( instanceData.data );
        instanceData.data    = malloc( instanceData.size );
        instanceData.maxSize = instanceData.size;
    }
    memcpy( instanceData.data, ptr, instanceData.size );
    instanceData.commitCount = _commitCount;
    _instanceData.push_front( instanceData );

    // send delta to subscribed slaves
    vector< eqBase::RefPtr<Node> >& slaves = getSlaves();
    ObjectSyncPacket       packet( _session->getID(), getID( ));

    packet.version   = _version;
    packet.deltaSize = deltaSize;

    // OPT: packet is re-assembled for each slave!
    for( vector< eqBase::RefPtr<Node> >::iterator iter = slaves.begin();
         iter != slaves.end(); ++iter )
        
        (*iter)->send( packet, delta, deltaSize );

    releasePackData( delta );
    EQVERB << "Committed version " << _version << ", id " << getID() << endl;
    return _version;
}

bool Object::sync( const uint32_t version, const float timeout )
{
    EQVERB << "Sync to version " << version << ", id " << getID() << endl;
    if( _version == version )
        return true;

    if( isMaster( ) || _version > version )
        return false;

    while( _version < version )
    {
        eqNet::Node*   node;
        eqNet::Packet* packet;
        _syncQueue.pop( &node, &packet );

         // OPT shortcut around handleCommand()
        EQASSERT( packet->command == REQ_OBJECT_SYNC );
        _reqSync( node, packet );
    }

    EQVERB << "Sync'ed to version " << version << ", id " << getID() << endl;
    return true;
}

void Object::sync()
{
    if( isMaster( ))
        return;

    eqNet::Node*   node;
    eqNet::Packet* packet;

    while( _syncQueue.tryPop( &node, &packet ))
    {
        EQASSERT( packet->command == REQ_OBJECT_SYNC );
        _reqSync( node, packet ); // XXX shortcut around handleCommand()
    }
    EQVERB << "Sync'ed to head version " << _version << ", id " << getID() 
           << endl;
}

uint32_t Object::getHeadVersion()
{
    if( isMaster( ))
        return _version;

    eqNet::Node*               node;
    ObjectSyncPacket* packet;

    if( !_syncQueue.back( &node, (Packet**)&packet ))
        return _version;
    
    EQASSERT( packet->command == REQ_OBJECT_SYNC );
    return packet->version;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
void Object::_reqSync( Node* node, const Packet* pkg )
{
    ObjectSyncPacket* packet = (ObjectSyncPacket*)pkg;
    EQVERB << "req sync " << packet << endl;
    
    unpack( packet->delta, packet->deltaSize );
    _version = packet->version;
    //return eqNet::COMMAND_HANDLED;
}
