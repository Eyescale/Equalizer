
/* Copyright (c) 2006-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

namespace co
{

Command::Command( lunchbox::a_int32_t& freeCounter ) 
        : _packet( 0 )
        , _data( 0 )
        , _dataSize( 0 )
        , _freeCount( freeCounter )
        , _func( 0, 0 )
{}

Command::~Command() 
{
    free(); 
}

void Command::deleteReferenced( const Referenced* object ) const
{
    Command* command = const_cast< Command* >( this );
    // DON'T 'command->_master = 0;', command is already reusable and _master
    // may be set any time. alloc or clone_ will free old master.
    ++command->_freeCount;
}

size_t Command::alloc_( NodePtr node, LocalNodePtr localNode,
                        const uint64_t size )
{
    LB_TS_THREAD( _writeThread );
    LBASSERT( getRefCount() == 1 ); // caller CommandCache
    LBASSERTINFO( !_func.isValid(), *this );
    LBASSERT( _freeCount > 0 );

    --_freeCount;

    size_t allocated = 0;
    if( !_data )
    {
        _dataSize = LB_MAX( Packet::minSize, size );
        _data = static_cast< Packet* >( malloc( _dataSize ));
        allocated = _dataSize;
    }
    else if( size > _dataSize )
    {
        allocated =  size - _dataSize;
        _dataSize = LB_MAX( Packet::minSize, size );
        ::free( _data );
        _data = static_cast< Packet* >( malloc( _dataSize ));
    }

    _node = node;
    _localNode = localNode;
    _func.clear();
    _packet = _data;
    _packet->size = size;
    _master = 0;

    return allocated;
}

void Command::clone_( CommandPtr from )
{
    LB_TS_THREAD( _writeThread );
    LBASSERT( getRefCount() == 1 ); // caller CommandCache
    LBASSERT( from->getRefCount() > 1 ); // caller CommandCache, self
    LBASSERTINFO( !_func.isValid(), *this );
    LBASSERT( _freeCount > 0 );

    --_freeCount;

    _node = from->_node;
    _localNode = from->_localNode;
    _packet = from->_packet;
    _master = from;
}

void Command::free()
{
    LB_TS_THREAD( _writeThread );
    LBASSERT( !_func.isValid( ));

    if( _data )
        ::free( _data );

    _data = 0;
    _dataSize = 0;
    _packet = 0;
    _node = 0;
    _localNode = 0;
    _master = 0;
}        

bool Command::operator()()
{
    LBASSERT( _func.isValid( ));
    Dispatcher::Func func = _func;
    _func.clear();
    return func( this );
}

std::ostream& operator << ( std::ostream& os, const Command& command )
{
    if( command.isValid( ))
    {
        os << lunchbox::disableFlush << "command< ";
        const Packet* packet = command.get< Packet >() ;
        switch( packet->type )
        {
            case PACKETTYPE_CO_NODE:
                os << static_cast< const NodePacket* >( packet );
                break;

            case PACKETTYPE_CO_OBJECT:
                os << static_cast< const ObjectPacket* >( packet );
                break;

            default:
                os << packet;
        }

        os << ", " << command.getNode() << " >" << lunchbox::enableFlush;
    }
    else
        os << "command< empty >";

    if( command._func.isValid( ))
        os << ' ' << command._func << std::endl;
    return os;
}
}
