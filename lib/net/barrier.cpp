
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "barrier.h"

#include "packets.h"
#include "session.h"

using namespace eqNet;
using namespace std;

void Barrier::_construct()
{
    _waitForLeave  = false;
    _masterEntered = false;

    _masterNotify.set();
    _slaveNotify.set();
    _leaveNotify.set();

    registerCommand( CMD_BARRIER_ENTER, this, reinterpret_cast<CommandFcn>(
                         &eqNet::Barrier::_cmdEnter ));
    registerCommand( CMD_BARRIER_ENTER_REPLY, this, 
                reinterpret_cast<CommandFcn>(&eqNet::Barrier::_cmdEnterReply ));

#ifdef CHECK_THREADSAFETY
    _threadID = 0;
#endif
}

Barrier::Barrier( const uint32_t height )
        : Object( TYPE_BARRIER, CMD_BARRIER_ALL ),
          _height( height )
{
    EQASSERT( height > 1 );
    _construct();

    EQINFO << "New barrier of height " << _height << endl;
}

Barrier::Barrier( const void* instanceData )
        : Object( TYPE_BARRIER, CMD_BARRIER_ALL ),
          _height( *(uint32_t*)instanceData )
 
{
    EQASSERT( _height > 1 );
    _construct();

    EQINFO << "Barrier of height " << _height << " instanciated" << endl;
}


const void* Barrier::getInstanceData( uint64_t* size )
{
    *size   = sizeof( _height );
    return &_height;
}

void Barrier::enter()
{
    EQVERB << "enter barrier" << endl;
    EQASSERT( getSession( ));

    if( isMaster( ))
    {
        _mutex.set();
        const bool masterEntered = _masterEntered;
        if( !_masterEntered )
            _masterEntered = true;
        _mutex.unset();

        if( !masterEntered )
        {
            EQVERB << "master entry" << endl;
            _masterNotify.set();
            _mutex.set();
            _masterEntered = false;

            EQASSERT( _slaves.size() == _height-1 );

            BarrierEnterReplyPacket packet( getSession()->getID(), getID( ));
            for( uint32_t i=0; i<_height-1; ++i )
                _slaves[i]->send( packet );
            
            _slaves.clear();
            _leaveNotify.unset();
            _mutex.unset();
            EQVERB << "master left" << endl;
            return;
        }
        EQVERB << "secondary master entry" << endl;
    }
    else
        EQVERB << "slave entry" << endl;

    // slave or secondary enter on master instance
    eqBase::RefPtr<Node> master = getSession()->getIDMaster( getID( ));
    EQASSERT( master.isValid( ));

    BarrierEnterPacket packet( getSession()->getID(), getID( ));
    
    master->send( packet );
    _slaveNotify.set();
    EQVERB << "slave left" << endl;
}

CommandResult Barrier::_cmdEnter( Node* node, const Packet* pkg )
{
    CHECK_THREAD( _threadID );
    EQASSERT( isMaster( ));

    if( _waitForLeave )
    {
        _waitForLeave = false;
         // blocks until leaves have been send to ensure thread-safety for the
         // _slaves vector. Otherwise we might re-enter before everybody is
         // unlocked.
         // Not perfect performance-wise, since receiver thread is
         // blocked until the leave packets have been sent.
        _leaveNotify.set();
    }

    _slaves.push_back( node );

    if( _slaves.size() == _height-1 )
    {
        _masterNotify.unset();
        _waitForLeave = true; // sync before next enter
    }

    return COMMAND_HANDLED;
}

CommandResult Barrier::_cmdEnterReply( Node* node, const Packet* pkg )
{
    CHECK_THREAD( _threadID );
    _slaveNotify.unset();
    return COMMAND_HANDLED;
}
