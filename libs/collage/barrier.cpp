
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com> 
 *                    2011, Cedric Stalder <cedric.stalder@gmail.com>
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
#include "exception.h"

#include <co/base/global.h>

namespace co
{
typedef CommandFunc<Barrier> CmdFunc;

Barrier::Barrier( NodePtr master, const uint32_t height )
        : _masterID( master->getNodeID( ))
        , _height( height )
        , _master( master )
{
    EQASSERT( _masterID != NodeID::ZERO );
    EQINFO << "New barrier of height " << _height << std::endl;
}

Barrier::Barrier()
{
    EQINFO << "Barrier instantiated" << std::endl;
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
    _leaveNotify = 0;
}

void Barrier::applyInstanceData( DataIStream& is )
{
    is >> _height >> _masterID;
    _leaveNotify = 0;
}

void Barrier::pack( DataOStream& os )
{
    os << _height;
    _leaveNotify = 0;
}

void Barrier::unpack( DataIStream& is )
{
    is >> _height;
    _leaveNotify = 0;
}

//---------------------------------------------------------------------------
void Barrier::attach( const base::UUID& id, const uint32_t instanceID )
{
    Object::attach( id, instanceID );

    CommandQueue* queue = getLocalNode()->getCommandThreadQueue();

    registerCommand( CMD_BARRIER_ENTER,
                     CmdFunc( this, &Barrier::_cmdEnter ), queue );
    registerCommand( CMD_BARRIER_ENTER_REPLY, 
                     CmdFunc( this, &Barrier::_cmdEnterReply ), queue );
}

void Barrier::enter( const uint32_t timeout )
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

    EQASSERT( _master );
    EQASSERT( _master->isConnected( ));
    if( !_master || !_master->isConnected( ))
    {
        EQWARN << "Can't connect barrier master node " << _masterID <<std::endl;
        return;
    }

    EQLOG( LOG_BARRIER ) << "enter barrier " << getID() << " v" << getVersion()
                         << ", height " << _height << std::endl;

    const uint32_t leaveVal = _leaveNotify.get() + 1;

    BarrierEnterPacket packet;
    packet.version = getVersion();
    packet.incarnation = _leaveNotify.get();
    packet.timeout = timeout;
    send( _master, packet );

    if( timeout == EQ_TIMEOUT_INDEFINITE )
        _leaveNotify.waitEQ( leaveVal );
    else if( !_leaveNotify.timedWaitEQ( leaveVal, timeout ))
        throw Exception( Exception::TIMEOUT_BARRIER );

    EQLOG( LOG_BARRIER ) << "left barrier " << getID() << " v" << getVersion()
                         << ", height " << _height << std::endl;
}

bool Barrier::_cmdEnter( Command& command )
{
    EQ_TS_THREAD( _thread );
    EQASSERTINFO( !_master || _master == getLocalNode(), _master );

    BarrierEnterPacket* packet = command.get< BarrierEnterPacket >();
    if( packet->handled )
        return true;
    packet->handled = true;

    EQLOG( LOG_BARRIER ) << "handle barrier enter " << packet << " barrier v"
                         << getVersion() << std::endl;

    const uint128_t version = packet->version;
    const uint64_t incarnation = packet->incarnation;
    Request& request = _enteredNodes[ version ];
 
    EQLOG( LOG_BARRIER ) << "enter barrier v" << version 
                         << ", has " << request.nodes.size() << " of " 
                         << _height << std::endl;

    request.time = getLocalNode()->getTime64();
    
    // It's the first call to enter barrier
    if( request.nodes.empty() )
    {
        request.incarnation = incarnation;
        request.timeout = packet->timeout;
    }
    else if( request.timeout != EQ_TIMEOUT_INDEFINITE )
    {
        // the incarnation belongs to an older barrier
        if( request.incarnation < incarnation )
        {
            // send directly the reply packet to unblock the caller
            _sendNotify( version, command.getNode( ));
            return true;
        }
        // the previous enter had a timeout, start a newsynchronization
        else if( request.incarnation != incarnation )
        {
            request.nodes.clear();
            request.incarnation = incarnation;
            request.timeout = packet->timeout;
        }
    }
    request.nodes.push_back( command.getNode( ));

    // clean older data which was not removed during older synchronization
    if( request.timeout != EQ_TIMEOUT_INDEFINITE )
        _cleanup( request.time );

    // If we got early entry requests for this barrier, just note their
    // appearance. This requires that another request for the later version
    // arrives once the barrier reaches this version. The only case when this is
    // not the case is when no contributor to the current version contributes to
    // the later version, in which case deadlocks might happen because the later
    // version never leaves the barrier. We simply assume this is not the case.
    if( version > getVersion( ))
        return true;
    
    // if it's an older version a timeout has been handled
    // for performance, send directly the order to unblock the caller.
    if( packet->timeout != EQ_TIMEOUT_INDEFINITE && version < getVersion( ))
    {
        EQASSERT( incarnation == 0 );
        _sendNotify( version, command.getNode( ) );
        return true;
    }

    EQASSERT( version == getVersion( ));

    Nodes& nodes = request.nodes;
    if( nodes.size() < _height )
        return true;

    EQASSERT( nodes.size() == _height );
    EQLOG( LOG_BARRIER ) << "Barrier reached" << std::endl;

    stde::usort( nodes );

    for( Nodes::iterator i = nodes.begin(); i != nodes.end(); ++i )
        _sendNotify( version, *i );

    // delete node vector for version
    std::map< uint128_t, Request >::iterator i = _enteredNodes.find( version );
    EQASSERT( i != _enteredNodes.end( ));
    _enteredNodes.erase( i );
    return true;
}

void Barrier::_sendNotify( const uint128_t& version, NodePtr node )
{
    EQ_TS_THREAD( _thread );
    EQASSERTINFO( !_master || _master == getLocalNode(), _master );

    if( node->isLocal( )) // OPT
    {
        EQLOG( LOG_BARRIER ) << "Unlock local user(s)" << std::endl;
        // the case where we receive a different version of the barrier meant
        // that previosly we have detect a timeout true negative
        if( version == getVersion() )
            ++_leaveNotify;
    }
    else
    {
        EQLOG( LOG_BARRIER ) << "Unlock " << node << std::endl;
        BarrierEnterReplyPacket reply( getID(), version );
        node->send( reply );
    }
}

void Barrier::_cleanup( const uint64_t time)
{
    EQ_TS_THREAD( _thread );
    EQASSERTINFO( !_master || _master == getLocalNode(), _master );

    if( _enteredNodes.size() < 2 )
        return;

    for( std::map< uint128_t, Request >::iterator i = 
                        _enteredNodes.begin(); i != _enteredNodes.end(); i++ )
    {
        Request& cleanNodes = i->second;
        
        if( cleanNodes.timeout == EQ_TIMEOUT_INDEFINITE )
            continue;

        const uint32_t timeout = cleanNodes.timeout != EQ_TIMEOUT_DEFAULT ? 
                        cleanNodes.timeout :
                        base::Global::getIAttribute( 
                            base::Global::IATTR_TIMEOUT_DEFAULT );
               
        if( time > cleanNodes.time + timeout )
        {
            _enteredNodes.erase( i );
            return;
        }
    }
}

bool Barrier::_cmdEnterReply( Command& command )
{
    EQ_TS_THREAD( _thread );
    EQLOG( LOG_BARRIER ) << "Got ok, unlock local user(s)" << std::endl;
    BarrierEnterReplyPacket* reply = command.get< BarrierEnterReplyPacket >();
    
    if( reply->version == getVersion( ))
        ++_leaveNotify;
    
    return true;
}
}
