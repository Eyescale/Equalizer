
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "command.h"

#include "node.h"
#include "packets.h"

using namespace eq::base;
using namespace std;

namespace eq
{
namespace net
{

Command::Command( const Command& from )
        : _node( from._node )
        , _localNode( from._localNode )
        , _packet( 0 )
        , _packetAllocSize( 0 )
        , _dispatched( from._dispatched )
{
    if( !from.isValid( ))
        return;

    _packetAllocSize = EQ_MAX( Packet::minSize, from._packet->size );
    _packet          = static_cast<Packet*>( malloc( _packetAllocSize ));
    memcpy( _packet, from._packet, from._packet->size );
}

void Command::swap( Command& rhs )
{
    if( this == &rhs )
        return;

    Packet*        packet          = _packet;
    NodePtr        node            = _node;
    NodePtr        localNode       = _localNode;
    const uint64_t packetAllocSize = _packetAllocSize;

    _packet          = rhs._packet;
    _node            = rhs._node;
    _localNode       = rhs._localNode;
    _packetAllocSize = rhs._packetAllocSize;
    _dispatched      = false;

    rhs._packet          = packet;
    rhs._node            = node;
    rhs._localNode       = localNode;
    rhs._packetAllocSize = packetAllocSize;
    rhs._dispatched      = false;
}

void Command::allocate( NodePtr node, 
                        NodePtr localNode, 
                        const uint64_t       packetSize )
{
    if( !_packet )
    {
        _packetAllocSize = EQ_MAX( Packet::minSize, packetSize );
        _packet = static_cast<Packet*>( malloc( _packetAllocSize ));
    }
    else if( packetSize > _packetAllocSize )
    {
        _packetAllocSize = EQ_MAX( Packet::minSize, packetSize );
        _packet = static_cast<Packet*>( realloc( _packet, _packetAllocSize ));
    }

    _node         = node;
    _localNode    = localNode;
    _packet->size = packetSize;
    _dispatched   = false;
}

void Command::release()
{
    if( _packet )
        free( _packet );

    _packet    = 0;
    _node      = 0;
    _localNode = 0;
    _packetAllocSize = 0;
    _dispatched      = false;
}        

EQ_EXPORT std::ostream& operator << ( std::ostream& os, const Command& command )
{
    if( command.isValid( ))
    {
        os << disableFlush << "command< ";
        const Packet* packet = command.getPacket() ;
        switch( packet->datatype )
        {
            case DATATYPE_EQNET_SESSION:
                os << static_cast< const SessionPacket* >( packet );
                break;

            case DATATYPE_EQNET_OBJECT:
                os << static_cast< const ObjectPacket* >( packet );
                break;

            default:
                os << packet;
        }

        os << ", " << command.getNode() << " >" << enableFlush;
    }
    else
        os << "command< empty >";
    
    return os;
}
}
}
