
/* Copyright (c) 2006-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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
#include "global.h"
#include "log.h"
#include "barrierPackets.h"
#include "exception.h"

#include <lunchbox/monitor.h>
#include <lunchbox/stdExt.h>

namespace co
{
namespace
{
struct Request
{
    Request() 
            : time( 0 ), timeout( LB_TIMEOUT_INDEFINITE ), incarnation( 0 ) {}
    uint64_t time;
    uint32_t timeout;
    uint32_t incarnation;
    Nodes nodes;
};

typedef stde::hash_map< uint128_t, Request > RequestMap;
typedef RequestMap::iterator RequestMapIter;
}

namespace detail
{
class Barrier
{
public:
    Barrier() : height( 0 ) {}
    Barrier( NodePtr m, const uint32_t h )
            : masterID( m->getNodeID( ))
            , height( h )
            , master( m )
        {
            EQASSERT( masterID != NodeID::ZERO );
        }

    /** The master barrier node. */
    NodeID   masterID;

    /** The height of the barrier, only set on the master. */
    uint32_t height;

    /** The local, connected instantiation of the master node. */
    NodePtr master;

    /** Slave nodes which have entered the barrier, index per version. */
    RequestMap enteredNodes;

    /** The monitor used for barrier leave notification. */
    lunchbox::Monitor< uint32_t > leaveNotify;
};
}

typedef CommandFunc<Barrier> CmdFunc;

Barrier::Barrier()
        : _impl( new detail::Barrier )
{
    EQINFO << "Barrier instantiated" << std::endl;
}

Barrier::Barrier( NodePtr master, const uint32_t height )
        : _impl( new detail::Barrier( master, height ))
{
    EQINFO << "New barrier of height " << height << std::endl;
}

Barrier::~Barrier()
{
    delete _impl;
}

//---------------------------------------------------------------------------
// Serialization
//---------------------------------------------------------------------------
void Barrier::getInstanceData( DataOStream& os )
{
    os << _impl->height << _impl->masterID;
    _impl->leaveNotify = 0;
}

void Barrier::applyInstanceData( DataIStream& is )
{
    is >> _impl->height >> _impl->masterID;
    _impl->leaveNotify = 0;
}

void Barrier::pack( DataOStream& os )
{
    os << _impl->height;
    _impl->leaveNotify = 0;
}

void Barrier::unpack( DataIStream& is )
{
    is >> _impl->height;
    _impl->leaveNotify = 0;
}

//---------------------------------------------------------------------------
void Barrier::setHeight( const uint32_t height )
{
    _impl->height = height;
}

void Barrier::increase()
{
    ++_impl->height;
}

uint32_t Barrier::getHeight() const
{
    return _impl->height;
}

void Barrier::attach( const UUID& id, const uint32_t instanceID )
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
    EQASSERT( _impl->height > 0 );
    EQASSERT( _impl->masterID != NodeID::ZERO );

    if( _impl->height == 1 ) // trivial ;)
        return;

    if( !_impl->master )
    {
        LocalNodePtr localNode = getLocalNode();
        _impl->master = localNode->connect( _impl->masterID );
    }

    EQASSERT( _impl->master );
    EQASSERT( _impl->master->isConnected( ));
    if( !_impl->master || !_impl->master->isConnected( ))
    {
        EQWARN << "Can't connect barrier master node " << _impl->masterID
               << std::endl;
        return;
    }

    EQLOG( LOG_BARRIER ) << "enter barrier " << getID() << " v" << getVersion()
                         << ", height " << _impl->height << std::endl;

    const uint32_t leaveVal = _impl->leaveNotify.get() + 1;

    BarrierEnterPacket packet;
    packet.version = getVersion();
    packet.incarnation = _impl->leaveNotify.get();
    packet.timeout = timeout;
    send( _impl->master, packet );

    if( timeout == LB_TIMEOUT_INDEFINITE )
        _impl->leaveNotify.waitEQ( leaveVal );
    else if( !_impl->leaveNotify.timedWaitEQ( leaveVal, timeout ))
        throw Exception( Exception::TIMEOUT_BARRIER );

    EQLOG( LOG_BARRIER ) << "left barrier " << getID() << " v" << getVersion()
                         << ", height " << _impl->height << std::endl;
}

bool Barrier::_cmdEnter( Command& command )
{
    LB_TS_THREAD( _thread );
    EQASSERTINFO( !_impl->master || _impl->master == getLocalNode(),
                  _impl->master );

    BarrierEnterPacket* packet = command.getModifiable< BarrierEnterPacket >();
    if( packet->handled )
        return true;
    packet->handled = true;

    EQLOG( LOG_BARRIER ) << "handle barrier enter " << packet << " barrier v"
                         << getVersion() << std::endl;

    const uint128_t version = packet->version;
    const uint64_t incarnation = packet->incarnation;
    Request& request = _impl->enteredNodes[ version ];
 
    EQLOG( LOG_BARRIER ) << "enter barrier v" << version 
                         << ", has " << request.nodes.size() << " of " 
                         << _impl->height << std::endl;

    request.time = getLocalNode()->getTime64();
    
    // It's the first call to enter barrier
    if( request.nodes.empty() )
    {
        request.incarnation = incarnation;
        request.timeout = packet->timeout;
    }
    else if( request.timeout != LB_TIMEOUT_INDEFINITE )
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
    if( request.timeout != LB_TIMEOUT_INDEFINITE )
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
    if( packet->timeout != LB_TIMEOUT_INDEFINITE && version < getVersion( ))
    {
        EQASSERT( incarnation == 0 );
        _sendNotify( version, command.getNode( ) );
        return true;
    }

    EQASSERT( version == getVersion( ));

    Nodes& nodes = request.nodes;
    if( nodes.size() < _impl->height )
        return true;

    EQASSERT( nodes.size() == _impl->height );
    EQLOG( LOG_BARRIER ) << "Barrier reached" << std::endl;

    stde::usort( nodes );

    for( NodesIter i = nodes.begin(); i != nodes.end(); ++i )
        _sendNotify( version, *i );

    // delete node vector for version
    RequestMapIter i = _impl->enteredNodes.find( version );
    EQASSERT( i != _impl->enteredNodes.end( ));
    _impl->enteredNodes.erase( i );
    return true;
}

void Barrier::_sendNotify( const uint128_t& version, NodePtr node )
{
    LB_TS_THREAD( _thread );
    EQASSERTINFO( !_impl->master || _impl->master == getLocalNode(),
                  _impl->master );

    if( node->isLocal( )) // OPT
    {
        EQLOG( LOG_BARRIER ) << "Unlock local user(s)" << std::endl;
        // the case where we receive a different version of the barrier meant
        // that previosly we have detect a timeout true negative
        if( version == getVersion() )
            ++_impl->leaveNotify;
    }
    else
    {
        EQLOG( LOG_BARRIER ) << "Unlock " << node << std::endl;
        BarrierEnterReplyPacket reply( getID(), version );
        node->send( reply );
    }
}

void Barrier::_cleanup( const uint64_t time )
{
    LB_TS_THREAD( _thread );
    EQASSERTINFO( !_impl->master || _impl->master == getLocalNode(),
                  _impl->master );

    if( _impl->enteredNodes.size() < 2 )
        return;

    for( RequestMapIter i = _impl->enteredNodes.begin();
         i != _impl->enteredNodes.end(); ++i )
    {
        Request& cleanNodes = i->second;
        
        if( cleanNodes.timeout == LB_TIMEOUT_INDEFINITE )
            continue;

        const uint32_t timeout = cleanNodes.timeout != LB_TIMEOUT_DEFAULT ? 
                        cleanNodes.timeout :
                        Global::getIAttribute( Global::IATTR_TIMEOUT_DEFAULT );
               
        if( time > cleanNodes.time + timeout )
        {
            _impl->enteredNodes.erase( i );
            return;
        }
    }
}

bool Barrier::_cmdEnterReply( Command& command )
{
    LB_TS_THREAD( _thread );
    EQLOG( LOG_BARRIER ) << "Got ok, unlock local user(s)" << std::endl;
    const BarrierEnterReplyPacket* reply =
        command.get< BarrierEnterReplyPacket >();
    
    if( reply->version == getVersion( ))
        ++_impl->leaveNotify;
    
    return true;
}

}
