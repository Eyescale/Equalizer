
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "barrier.h"

#include "command.h"
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

    registerCommand( CMD_BARRIER_ENTER, 
                     CommandFunc<Barrier>( this, &Barrier::_cmdEnter ));
    registerCommand( CMD_BARRIER_ENTER_REPLY, 
                     CommandFunc<Barrier>( this, &Barrier::_cmdEnterReply ));
}

Barrier::Barrier( eqBase::RefPtr<Node> master, const uint32_t height )
        : _master( master )
{
    _data.master = master->getNodeID();
    _data.height = height;
    _construct();

    EQASSERT( _data.master != NodeID::ZERO );
    EQINFO << "New barrier of height " << _data.height << endl;
}

Barrier::Barrier()
{
    _construct();
    EQINFO << "Barrier instanciated" << endl;
}

Barrier::~Barrier()
{
}

void Barrier::enter()
{
    EQASSERT( _data.height > 0 );
    EQASSERT( _data.master != NodeID::ZERO );

    if( _data.height == 1 ) // trivial ;)
        return;

    if( !_master )
    {
        Session*     session   = getSession();
        RefPtr<Node> localNode = session->getLocalNode();
        _master = localNode->connect( _data.master, session->getServer( ));
    }

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

CommandResult Barrier::_cmdEnter( Command& command )
{
    CHECK_THREAD( _thread );
    EQASSERTINFO( !_master || _master == getSession()->getLocalNode( ),
                  _master );

    const BarrierEnterPacket* packet = command.getPacket<BarrierEnterPacket>();

    EQLOG( LOG_BARRIER ) << "handle barrier enter " << packet << " barrier v"
                         << getVersion() << endl;
    if( packet->version > getVersion( ))
        return COMMAND_REDISPATCH;
    
    EQASSERT( packet->version == getVersion( ));

    EQLOG( LOG_BARRIER ) << "enter barrier, has " << _enteredNodes.size()
                         << " of " << _data.height << endl;
    _enteredNodes.push_back( command.getNode( ));

    if( _enteredNodes.size() < _data.height )
        return COMMAND_DISCARD;

    EQASSERT( _enteredNodes.size() == _data.height );
    EQLOG( LOG_BARRIER ) << "Barrier reached" << endl;

    BarrierEnterReplyPacket reply;
    reply.sessionID  = getSession()->getID();
    reply.objectID   = getID();

    stde::usort( _enteredNodes );

    for( vector< RefPtr<Node> >::iterator iter = _enteredNodes.begin();
         iter != _enteredNodes.end(); ++iter )
    {
        RefPtr<Node>& node = *iter;
        if( node->isLocal( )) // OPT
            ++_leaveNotify;
        else
            node->send( reply );
    }

    _enteredNodes.clear();
    return COMMAND_DISCARD;
}

CommandResult Barrier::_cmdEnterReply( Command& command )
{
    CHECK_THREAD( _thread );
    ++_leaveNotify;
    return COMMAND_HANDLED;
}
