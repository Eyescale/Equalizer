
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
          _height( height ),
          _waitForLeave( false )
{
    EQASSERT( height > 1 );
    _enterLock.set();
    _leaveLock.set();

    registerCommand( CMD_BARRIER_ENTER, this, reinterpret_cast<CommandFcn>(
                         &eqNet::Barrier::_cmdEnter ));
    registerCommand( CMD_BARRIER_ENTER_REPLY, this, 
                reinterpret_cast<CommandFcn>(&eqNet::Barrier::_cmdEnterReply ));
    EQINFO << "New barrier of height " << _height << endl;
}

Barrier::Barrier( const void* instanceData )
        : Mobject( MOBJECT_EQNET_BARRIER ),
          Base( CMD_BARRIER_ALL ),
          _waitForLeave( false )
{
    _height = *(uint32_t*)instanceData;
    EQASSERT( _height > 1 );
    _enterLock.set();
    _leaveLock.set();

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
        _enterLock.set();
        EQASSERT( _slaves.size() == _height-1 );

        BarrierEnterReplyPacket packet( getSession()->getID(), getID( ));
        for( uint32_t i=0; i<_height-1; ++i )
            _slaves[i]->send( packet );

        _slaves.clear();
        _leaveLock.unset();
        return;
    }

    eqBase::RefPtr<Node> master = getSession()->getIDMaster( getID( ));
    EQASSERT( master.isValid( ));

    BarrierEnterPacket packet( getSession()->getID(), getID( ));
    
    master->send( packet );
    _enterLock.set();
}

CommandResult Barrier::_cmdEnter( Node* node, const Packet* pkg )
{
    EQASSERT( isMaster( ));

    if( _waitForLeave )
    {
        _waitForLeave = false;
         // blocks until unlocks have been send to ensure thread-safety for the
         // _slaves vector. Otherwise we might re-enter before everybody is
         // unlocked.
         // Not perfect performance-wise, since receiver thread is
         // blocked. Sending the leave packets should not take long, though.
        _leaveLock.set();
    }

    _slaves.push_back( node );

    if( _slaves.size() == _height-1 )
    {
        _enterLock.unset();
        _waitForLeave = true; // sync before next enter
    }

    return COMMAND_HANDLED;
}

CommandResult Barrier::_cmdEnterReply( Node* node, const Packet* pkg )
{
    EQASSERT( !isMaster( ));
    _enterLock.unset();
    return COMMAND_HANDLED;
}
