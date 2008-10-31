
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "barrier.h"

#include "command.h"
#include "connection.h"
#include "dataIStream.h"
#include "dataOStream.h"
#include "log.h"
#include "packets.h"
#include "session.h"

using namespace eq::base;
using namespace std;

namespace eq
{
namespace net
{
Barrier::Barrier( NodePtr master, const uint32_t height )
        : _master( master )
{
    _data.master = master->getNodeID();
    _data.height = height;

    EQASSERT( _data.master != NodeID::ZERO );
    EQINFO << "New barrier of height " << _data.height << endl;
}

Barrier::Barrier()
{
    EQINFO << "Barrier instanciated" << endl;
}

Barrier::~Barrier()
{
}

//---------------------------------------------------------------------------
// Serialization
//---------------------------------------------------------------------------
void Barrier::getInstanceData( DataOStream& os )
{
    os.writeOnce( &_data, sizeof( _data )); 
}

void Barrier::applyInstanceData( DataIStream& is )
{
    EQASSERT( is.getRemainingBufferSize() == sizeof( _data )); 

    memcpy( &_data, is.getRemainingBuffer(), sizeof( _data ));
    is.advanceBuffer( sizeof( _data ));

    EQASSERT( is.nRemainingBuffers() == 0 );
    EQASSERT( is.getRemainingBufferSize() == 0 );
}

void Barrier::pack( DataOStream& os )
{
    os.writeOnce( &_data.height, sizeof( _data.height )); 
}

void Barrier::unpack( DataIStream& is )
{
    EQASSERT( is.getRemainingBufferSize() == sizeof( _data.height )); 

    memcpy( &_data.height, is.getRemainingBuffer(), sizeof( _data.height ));
    is.advanceBuffer( sizeof( _data.height ));

    EQASSERT( is.nRemainingBuffers() == 0 );
    EQASSERT( is.getRemainingBufferSize() == 0 );
}
//---------------------------------------------------------------------------

void Barrier::attachToSession( const uint32_t id, const uint32_t instanceID, 
                               Session* session )
{
    Object::attachToSession( id, instanceID, session );

    CommandQueue* queue = session->getCommandThreadQueue();

    registerCommand( CMD_BARRIER_ENTER, 
                     CommandFunc<Barrier>( this, &Barrier::_cmdEnter ),
                     queue );
    registerCommand( CMD_BARRIER_ENTER_REPLY, 
                     CommandFunc<Barrier>( this, &Barrier::_cmdEnterReply ),
                     queue );
}

void Barrier::enter()
{
    EQASSERT( _data.height > 0 );
    EQASSERT( _data.master != NodeID::ZERO );

    if( _data.height == 1 ) // trivial ;)
        return;

    if( !_master )
    {
        Session* session   = getSession();
        NodePtr  localNode = session->getLocalNode();
        _master = localNode->connect( _data.master, session->getServer( ));
    }

    EQASSERT( _master.isValid( ));
    EQASSERT( _master->isConnected( ));
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

    const uint32_t version = packet->version;
    NodeVector&    nodes   = _enteredNodes[ packet->version ];

    EQLOG( LOG_BARRIER ) << "enter barrier v" << version 
                         << ", has " << nodes.size() << " of " << _data.height
                         << endl;

    nodes.push_back( command.getNode( ));

    // If we got early entry requests for this barrier, just note their
    // appearance. This requires that another request for the later version
    // arrives once the barrier reaches this version. The only case when this is
    // not the case is when no contributor to the current version contributes to
    // the later version, in which case deadlocks might happen because the later
    // version never leaves the barrier. We simply assume this is not the case.
    if( version > getVersion( ))
        return COMMAND_DISCARD;
    
    EQASSERT( version == getVersion( ));

    if( nodes.size() < _data.height )
        return COMMAND_DISCARD;

    EQASSERT( nodes.size() == _data.height );
    EQLOG( LOG_BARRIER ) << "Barrier reached" << endl;

    BarrierEnterReplyPacket reply;
    reply.sessionID  = getSession()->getID();
    reply.objectID   = getID();

    stde::usort( nodes );

    for( NodeVector::iterator i = nodes.begin(); i != nodes.end(); ++i )
    {
        RefPtr< Node >& node = *i;
        if( node->isLocal( )) // OPT
        {
            EQLOG( LOG_BARRIER ) << "Unlock local user(s)" << endl;
            ++_leaveNotify;
        }
        else
        {
            EQLOG( LOG_BARRIER ) << "Unlock " << node << endl;
            node->send( reply );
        }
    }

    // delete node vector for version
    map< uint32_t, NodeVector >::iterator it = _enteredNodes.find( version );
    EQASSERT( it != _enteredNodes.end( ));
    _enteredNodes.erase( it );

    return COMMAND_DISCARD;
}

CommandResult Barrier::_cmdEnterReply( Command& command )
{
    CHECK_THREAD( _thread );
    EQLOG( LOG_BARRIER ) << "Got ok, unlock local user(s)" << endl;
    ++_leaveNotify;
    return COMMAND_HANDLED;
}
}
}
