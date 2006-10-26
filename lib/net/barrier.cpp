
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "barrier.h"

#include "connection.h"
#include "log.h"
#include "packets.h"
#include "session.h"

using namespace eqNet;
using namespace eqBase;
using namespace std;

void Barrier::_construct()
{
    setInstanceData( &_data, sizeof( _data ));
    setDeltaData( &_data.height, sizeof( _data.height ));

    registerCommand( CMD_BARRIER_ENTER, this, reinterpret_cast<CommandFcn>(
                         &eqNet::Barrier::_cmdEnter ));
    registerCommand( CMD_BARRIER_ENTER_REPLY, this, 
                reinterpret_cast<CommandFcn>(&eqNet::Barrier::_cmdEnterReply ));
}

Barrier::Barrier( eqBase::RefPtr<Node> master, const uint32_t height )
        : Object( TYPE_BARRIER, CMD_BARRIER_ALL ),
          _master( master )
{
    _data.master = master->getNodeID();
    _data.height = height;
    _construct();

    EQASSERT( _data.master != NodeID::ZERO );
    EQINFO << "New barrier of height " << _data.height << endl;
}

Barrier::Barrier( const void* instanceData )
        : Object( TYPE_BARRIER, CMD_BARRIER_ALL ),
          _data( *(Data*)instanceData ) 
{
    _construct();
    EQINFO << "Barrier of height " << _data.height << " instanciated" << endl;
}

void Barrier::init( const void* data, const uint64_t dataSize )
{
    EQASSERT( _data.master != NodeID::ZERO );
    _master = eqNet::Node::getLocalNode()->connect( _data.master, 
                                                    getSession()->getServer( ));
}

void Barrier::enter()
{
    EQASSERT( _data.height > 1 );
    EQASSERT( _master.isValid( ));
    EQLOG( LOG_BARRIER ) << "enter barrier " << getID() << " v" << getVersion()
                         << ", height " << _data.height << endl;
    EQASSERT( getSession( ));

    const uint32_t leaveVal = _leaveNotify.get() + 1;

    BarrierEnterPacket packet;
    packet.version = getVersion();
    send( _master, packet );
    
    _leaveNotify.waitEQ( leaveVal );
    EQLOG( LOG_BARRIER ) << "left barrier " << getID() << " v" << getVersion()
                         << ", height " << _data.height << endl;
}

CommandResult Barrier::_cmdEnter( Node* node, const Packet* pkg )
{
    CHECK_THREAD( _thread );
    EQASSERT( _master == eqNet::Node::getLocalNode( ));

    BarrierEnterPacket* packet = (BarrierEnterPacket*)pkg;
    EQLOG( LOG_BARRIER ) << "handle barrier enter " << packet << " barrier v"
                         << getVersion() << endl;
    if( packet->version > getVersion( ))
        return COMMAND_REDISPATCH;
    
    EQASSERT( packet->version == getVersion( ));

    EQLOG( LOG_BARRIER ) << "enter barrier, has " << _enteredNodes.size()
                         << " of " << _data.height << endl;
    _enteredNodes.push_back( node );

    if( _enteredNodes.size() < _data.height )
        return COMMAND_DISCARD;

    EQASSERT( _enteredNodes.size() == _data.height );
    EQLOG( LOG_BARRIER ) << "Barrier reached" << endl;

    BarrierEnterReplyPacket reply;
    reply.sessionID  = getSession()->getID();
    reply.objectID   = getID();

    stde::usort( _enteredNodes );

    for( vector<Node*>::const_iterator iter = _enteredNodes.begin();
         iter != _enteredNodes.end(); ++iter )
    {
        Node* node = *iter;
        if( node->isLocal( )) // OPT
            ++_leaveNotify;
        else
            node->send( reply );
    }

    _enteredNodes.clear();
    return COMMAND_DISCARD;
}

CommandResult Barrier::_cmdEnterReply( Node* node, const Packet* pkg )
{
    CHECK_THREAD( _thread );
    ++_leaveNotify;
    return COMMAND_HANDLED;
}
