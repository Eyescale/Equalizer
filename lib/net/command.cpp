
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

#include "command.h"

#include "node.h"
#include "packets.h"

namespace eq
{
namespace net
{

Command::Command() 
  : _packet( 0 )
  , _packetAllocSize( 0 )
{
}

Command::~Command() 
{
    EQASSERTINFO( _refCount == 0, _refCount << ", " << *this );
    _free(); 
}

void Command::alloc( NodePtr node, NodePtr localNode, const uint64_t size )
{
    if( !_packet )
    {
        _packetAllocSize = EQ_MAX( Packet::minSize, size );
        _packet = static_cast<Packet*>( malloc( _packetAllocSize ));
    }
    else if( size > _packetAllocSize )
    {
        _packetAllocSize = EQ_MAX( Packet::minSize, size );
        free( _packet );
        _packet = static_cast<Packet*>( malloc( _packetAllocSize ));
    }

    _node         = node;
    _localNode    = localNode;
    _packet->size = size;
}

void Command::_free()
{
    if( _packet )
        free( _packet );

    _packet    = 0;
    _node      = 0;
    _localNode = 0;
    _packetAllocSize = 0;
}        

EQ_EXPORT std::ostream& operator << ( std::ostream& os, const Command& command )
{
    if( command.isValid( ))
    {
        os << "command< ";
        const Packet* packet = command.getPacket() ;
        switch( packet->type )
        {
            case PACKETTYPE_EQNET_SESSION:
                os << static_cast< const SessionPacket* >( packet );
                break;

            case PACKETTYPE_EQNET_OBJECT:
                os << static_cast< const ObjectPacket* >( packet );
                break;

            default:
                os << packet;
        }

        os << ", " << command.getNode() << " >";
    }
    else
        os << "command< empty >";
    
    return os;
}
}
}
