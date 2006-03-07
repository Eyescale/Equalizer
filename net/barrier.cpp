
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "barrier.h"

#include "packets.h"
#include "session.h"

using namespace eqNet;
using namespace std;

Barrier::Barrier( const uint32_t height )
        : Mobject( MOBJECT_EQNET_BARRIER ),
          Base( CMD_BARRIER_ALL ),
          _height( height )
{
    EQASSERT( height > 1 );
    _lock.set();

    registerCommand( CMD_BARRIER_ENTER, this, reinterpret_cast<CommandFcn>(
                         &eqNet::Barrier::_cmdEnter ));
    registerCommand( CMD_BARRIER_ENTER_REPLY, this, 
                reinterpret_cast<CommandFcn>(&eqNet::Barrier::_cmdEnterReply ));
    EQINFO << "New barrier of height " << _height << endl;
}

Barrier::Barrier( const void* instanceData )
        : Mobject( MOBJECT_EQNET_BARRIER ),
          Base( CMD_BARRIER_ALL )
{
    _height = *(uint32_t*)instanceData;
    EQASSERT( _height > 1 );
    _lock.set();

    registerCommand( CMD_BARRIER_ENTER, this, reinterpret_cast<CommandFcn>(
                         &eqNet::Barrier::_cmdEnter ));
    registerCommand( CMD_BARRIER_ENTER_REPLY, this, 
                reinterpret_cast<CommandFcn>(&eqNet::Barrier::_cmdEnterReply ));
    EQINFO << "Barrier of height " << _height << " instanciated" << endl;
}


const void* Barrier::getInstanceData( uint64_t* size )
{
    *size   = sizeof( _height );
    return &_height;
}

void Barrier::enter()
{
    EQASSERT( getSession( ));

    if( isMaster( ))
    {
        _lock.set();
        EQASSERT( _slaves.size() == _height-1 );

        BarrierEnterReplyPacket packet( getSession()->getID(), getID( ));
        for( uint32_t i=0; i<_height-1; ++i )
            _slaves[i]->send( packet );

        _slaves.clear(); // XXX not thread-safe w/ _cmdEnter !
        return;
    }

    eqBase::RefPtr<Node> master = getSession()->getIDMaster( getID( ));
    EQASSERT( master.isValid( ));

    BarrierEnterPacket packet( getSession()->getID(), getID( ));
    
    master->send( packet );
    _lock.set();
}

CommandResult Barrier::_cmdEnter( Node* node, const Packet* pkg )
{
    EQASSERT( isMaster( ));
    _slaves.push_back( node );

    if( _slaves.size() == _height-1 )
        _lock.unset();
    
    return COMMAND_HANDLED;
}

CommandResult Barrier::_cmdEnterReply( Node* node, const Packet* pkg )
{
    EQASSERT( !isMaster( ));
    _lock.unset();
    return COMMAND_HANDLED;
}
