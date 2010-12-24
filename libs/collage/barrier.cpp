
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "barrier.h"

#include "command.h"
#include "connection.h"
#include "dataIStream.h"
#include "dataOStream.h"
#include "log.h"
#include "barrierPackets.h"

using namespace co::base;
using namespace std;

namespace co
{
typedef CommandFunc<Barrier> CmdFunc;

Barrier::Barrier( NodePtr master, const uint32_t height )
        : _masterID( master->getNodeID( ))
        , _height( height )
        , _master( master )
{
    EQASSERT( _masterID != NodeID::ZERO );
    EQINFO << "New barrier of height " << _height << endl;
}

Barrier::Barrier()
{
    EQINFO << "Barrier instantiated" << endl;
}

Barrier::~Barrier()
{
}

//---------------------------------------------------------------------------
// Serialization
//---------------------------------------------------------------------------
void Barrier::getInstanceData( DataOStream& os )
{
    os << _height << _masterID;
}

void Barrier::applyInstanceData( DataIStream& is )
{
    is >> _height >> _masterID;
}

void Barrier::pack( DataOStream& os )
{
    os << _height;
}

void Barrier::unpack( DataIStream& is )
{
    is >> _height;
}
//---------------------------------------------------------------------------

void Barrier::attach( const co::base::UUID& id, const uint32_t instanceID )
{
    Object::attach( id, instanceID );

    CommandQueue* queue = getLocalNode()->getCommandThreadQueue();

    registerCommand( CMD_BARRIER_ENTER,
                     CmdFunc( this, &Barrier::_cmdEnter ), queue );
    registerCommand( CMD_BARRIER_ENTER_REPLY, 
                     CmdFunc( this, &Barrier::_cmdEnterReply ), queue );
}

void Barrier::enter()
{
    EQASSERT( _height > 0 );
    EQASSERT( _masterID != NodeID::ZERO );

    if( _height == 1 ) // trivial ;)
        return;

    if( !_master )
    {
        LocalNodePtr localNode = getLocalNode();
        _master = localNode->connect( _masterID );
    }

    EQASSERT( _master.isValid( ));
    EQASSERT( _master->isConnected( ));
    EQLOG( LOG_BARRIER ) << "enter barrier " << getID() << " v" << getVersion()
                         << ", height " << _height << endl;

    const uint32_t leaveVal = _leaveNotify.get() + 1;

    BarrierEnterPacket packet;
    packet.version = getVersion();
    send( _master, packet );
    
    _leaveNotify.waitEQ( leaveVal );
    EQLOG( LOG_BARRIER ) << "left barrier " << getID() << " v" << getVersion()
                         << ", height " << _height << endl;
}

bool Barrier::_cmdEnter( Command& command )
{
    EQ_TS_THREAD( _thread );
    EQASSERTINFO( !_master || _master == getLocalNode(), _master );

    BarrierEnterPacket* packet = command.getPacket< BarrierEnterPacket >();
    if( packet->handled )
        return true;
    packet->handled = true;

    EQLOG( LOG_BARRIER ) << "handle barrier enter " << packet << " barrier v"
                         << getVersion() << endl;

    const uint128_t version = packet->version;
    Nodes& nodes = _enteredNodes[ packet->version ];

    EQLOG( LOG_BARRIER ) << "enter barrier v" << version 
                         << ", has " << nodes.size() << " of " << _height
                         << endl;

    nodes.push_back( command.getNode( ));

    // If we got early entry requests for this barrier, just note their
    // appearance. This requires that another request for the later version
    // arrives once the barrier reaches this version. The only case when this is
    // not the case is when no contributor to the current version contributes to
    // the later version, in which case deadlocks might happen because the later
    // version never leaves the barrier. We simply assume this is not the case.
    if( version > getVersion( ))
        return true;
    
    EQASSERT( version == getVersion( ));

    if( nodes.size() < _height )
        return true;

    EQASSERT( nodes.size() == _height );
    EQLOG( LOG_BARRIER ) << "Barrier reached" << endl;

    BarrierEnterReplyPacket reply;
    reply.objectID   = getID();

    stde::usort( nodes );

    for( Nodes::iterator i = nodes.begin(); i != nodes.end(); ++i )
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
    map< uint128_t, Nodes >::iterator i = _enteredNodes.find( version );
    EQASSERT( i != _enteredNodes.end( ));
    _enteredNodes.erase( i );
    return true;
}

bool Barrier::_cmdEnterReply( Command& )
{
    EQ_TS_THREAD( _thread );
    EQLOG( LOG_BARRIER ) << "Got ok, unlock local user(s)" << endl;
    ++_leaveNotify;
    return true;
}

}
