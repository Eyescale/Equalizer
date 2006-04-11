
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "object.h"

#include "packets.h"
#include "session.h"

#include <eq/base/log.h>
#include <iostream>

using namespace eqNet;
using namespace std;

Object::Object( const uint32_t typeID, 
                const uint32_t nCommands )
        : Base( nCommands ),
          _session( NULL ),
          _id( EQ_INVALID_ID ),
          _typeID( typeID ),
          _version(0)
{
    EQASSERT( nCommands >= CMD_OBJECT_CUSTOM );

    registerCommand( CMD_OBJECT_SYNC, this,
                     reinterpret_cast<CommandFcn>(
                         &eqNet::Object::_cmdSync ));
    registerCommand( REQ_OBJECT_SYNC, this, 
                     reinterpret_cast<CommandFcn>(
                         &eqNet::Object::_reqSync ));
}

Object::~Object()
{
}

uint32_t Object::commit()
{
    if( !isMaster( ))
        return 0;

    uint64_t deltaSize;
    const void* delta = pack( &deltaSize );
    if( !delta )
        return _version;

    ++_version;

    EQASSERT( _version != 0 );

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
