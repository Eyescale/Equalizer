
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "versionedObject.h"

#include "packets.h"
#include "session.h"

#include <eq/base/log.h>
#include <iostream>

using namespace eqNet;
using namespace std;

VersionedObject::VersionedObject( const uint32_t mobjectType, 
                                  const uint32_t nCommands )
        : Mobject( mobjectType ),
          Base( nCommands ),
          _version(0)
{
    EQASSERT( nCommands >= CMD_VERSIONED_OBJECT_CUSTOM );

    registerCommand( CMD_VERSIONED_OBJECT_SYNC, this,
                     reinterpret_cast<CommandFcn>(
                         &eqNet::VersionedObject::_cmdSync ));
    registerCommand( REQ_VERSIONED_OBJECT_SYNC, this, 
                     reinterpret_cast<CommandFcn>(
                         &eqNet::VersionedObject::_reqSync ));
}

VersionedObject::~VersionedObject()
{
}

uint32_t VersionedObject::commit()
{
    if( !isMaster( ))
        return 0;

    ++_version;

    EQASSERT( _version != 0 );

    vector< eqBase::RefPtr<Node> >& slaves = getSlaves();

    VersionedObjectSyncPacket packet( getSession()->getID(), getID( ));

    packet.version = _version;
    const void* delta = pack( &packet.deltaSize );
    
    // OPT: packet is re-assembled for each slave!
    for( vector< eqBase::RefPtr<Node> >::iterator iter = slaves.begin();
         iter != slaves.end(); ++iter )
        
        (*iter)->send( packet, delta, packet.deltaSize );

    return _version;
}

bool VersionedObject::sync( const uint32_t version, const float timeout )
{
    if( _version == version )
        return true;

    if( isMaster() || version < _version )
        return false;


    while( _version < version )
    {
        eqNet::Node*   node;
        eqNet::Packet* packet;
        _syncQueue.pop( &node, &packet );

        EQASSERT( packet->command == REQ_VERSIONED_OBJECT_SYNC );
        _reqSync( node, packet ); // XXX shortcut around handleCommand()
    }

    return true;
}

void VersionedObject::sync()
{
    if( isMaster( ))
        return;

    eqNet::Node*   node;
    eqNet::Packet* packet;

    while( _syncQueue.tryPop( &node, &packet ))
    {
        EQASSERT( packet->command == REQ_VERSIONED_OBJECT_SYNC );
        _reqSync( node, packet ); // XXX shortcut around handleCommand()
    }
}

uint32_t VersionedObject::getHeadVersion()
{
    if( isMaster( ))
        return _version;

    eqNet::Node*               node;
    VersionedObjectSyncPacket* packet;

    while( !_syncQueue.back( &node, (Packet**)&packet ))
        return _version;
    
    EQASSERT( packet->command == REQ_VERSIONED_OBJECT_SYNC );
    return packet->version;
}

//---------------------------------------------------------------------------
// command handlers
//---------------------------------------------------------------------------
void VersionedObject::_reqSync( Node* node, const Packet* pkg )
{
    VersionedObjectSyncPacket* packet = (VersionedObjectSyncPacket*)pkg;
    
    unpack( packet->delta, packet->deltaSize );
    _version = packet->version;
    //return eqNet::COMMAND_HANDLED;
}
