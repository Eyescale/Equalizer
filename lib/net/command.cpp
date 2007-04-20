
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "command.h"

#include "node.h"
#include "packets.h"

using namespace eqNet;
using namespace std;

Command::Command( const Command& from )
        : _packet( 0 )
{
    if( !from.isValid( ))
        return;

    _packet = static_cast<Packet*>( malloc( MAX( Packet::minSize,
                                                 from._packet->size )));
    memcpy( _packet, from._packet, from._packet->size );
    _node      = from._node;
    _localNode = from._localNode;
}

Command& Command::operator = ( Command& rhs )
{
    if( this == &rhs )
        return *this;

    release();

    if( !rhs._packet )   // empty holder
        return *this;

    // transfer packet avoiding copy
    _packet        = rhs._packet;
    _node          = rhs._node;
    _localNode     = rhs._localNode;
    rhs._packet    = 0;
    rhs._node      = 0;
    rhs._localNode = 0;

    return *this;
}

void Command::allocate( eqBase::RefPtr<Node> node, 
                        eqBase::RefPtr<Node> localNode, 
                        const uint64_t packetSize )
{
    if( _packet && packetSize > Packet::minSize && _packet->size < packetSize )
        release();

    if( !_packet )
        _packet = static_cast<Packet*>( malloc( MAX( Packet::minSize,
                                                     packetSize      )));

    _node         = node;
    _localNode    = localNode;
    _packet->size = packetSize;
}

void Command::release()
{
    free( _packet );
    _packet    = 0;
    _node      = 0;
    _localNode = 0;
}        

EQ_EXPORT std::ostream& eqNet::operator << ( std::ostream& os, 
                                             const Command& command )
{
    if( command.isValid( ))
        os << "command< " << command.getPacket() << ", " 
           << command.getNode() << " >";
    else
        os << "command< empty >";
    
    return os;
}
