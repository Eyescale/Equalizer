
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "command.h"

#include "node.h"
#include "packets.h"

using namespace eqNet;
using namespace eqBase;
using namespace std;

Command::Command( const Command& from )
        : _packet( 0 )
{
    if( !from.isValid( ))
        return;

    _packet = static_cast<Packet*>( malloc( EQ_MAX( Packet::minSize,
                                                    from._packet->size )));
    memcpy( _packet, from._packet, from._packet->size );
    _node      = from._node;
    _localNode = from._localNode;
}

void Command::swap( Command& rhs )
{
    if( this == &rhs )
        return;

    Packet* packet    = _packet;
    NodePtr node      = _node;
    NodePtr localNode = _localNode;

    _packet        = rhs._packet;
    _node          = rhs._node;
    _localNode     = rhs._localNode;

    rhs._packet    = packet;
    rhs._node      = node;
    rhs._localNode = localNode;
}

void Command::allocate( eqBase::RefPtr<Node> node, 
                        eqBase::RefPtr<Node> localNode, 
                        const uint64_t packetSize )
{
    if( _packet && packetSize > Packet::minSize && _packet->size < packetSize )
        _packet = static_cast<Packet*>( realloc( _packet, packetSize ));

    if( !_packet )
        _packet = static_cast<Packet*>( malloc( EQ_MAX( Packet::minSize,
                                                        packetSize      )));
    _node         = node;
    _localNode    = localNode;
    _packet->size = packetSize;
}

void Command::release()
{
    if( _packet )
        free( _packet );
    _packet    = 0;
    _node      = 0;
    _localNode = 0;
}        

EQ_EXPORT std::ostream& eqNet::operator << ( std::ostream& os, 
                                             const Command& command )
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
