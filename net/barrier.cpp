
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "barrier.h"

#include "packets.h"
#include "session.h"

using namespace eqNet;

Barrier::Barrier( const uint32_t height )
        : Base( CMD_BARRIER_ALL ),
          _height( height ),
          _master( true )
{
    ASSERT( height > 1 );
    _lock.set();
    registerCommand( CMD_BARRIER_ENTER, this, reinterpret_cast<CommandFcn>(
                         &eqNet::Barrier::_cmdEnter ));
}

Barrier::Barrier( const char* instanceInfo )
        : Base( CMD_BARRIER_ALL ),
          _height(0), // irrelevant on slaves
          _master( false )
{
    _lock.set();
    registerCommand( CMD_BARRIER_ENTER_REPLY, this, 
                reinterpret_cast<CommandFcn>(&eqNet::Barrier::_cmdEnterReply ));
}


void Barrier::getInstanceInfo( uint32_t* typeID, std::string& data )
{
    *typeID = MOBJECT_EQNET_BARRIER;
    data = ""; // no data needed for slaves
}

void Barrier::enter()
{
    ASSERT( _session );

    if( _master )
    {
        _lock.set();
        ASSERT( _slaves.size() == _height-1 );

        BarrierEnterReplyPacket packet( _session->getID(), _id );
        for( uint32_t i=0; i<_height-1; ++i )
            _slaves[i]->send( packet );

        _slaves.clear();
        return;
    }

    eqBase::RefPtr<Node> master = _session->getIDMaster( _id );
    ASSERT( master );

    BarrierEnterPacket packet( _session->getID(), _id );
    
    master->send( packet );
    _lock.set();
}

CommandResult Barrier::_cmdEnter( Node* node, const Packet* pkg )
{
    _slaves.push_back( node );

    if( _slaves.size() == _height-1 )
        _lock.unset();
    
    return COMMAND_HANDLED;
}

CommandResult Barrier::_cmdEnterReply( Node* node, const Packet* pkg )
{
    _lock.unset();
    return COMMAND_HANDLED;
}
