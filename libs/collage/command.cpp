
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


namespace co
{

Command::Command() 
        : _packet( 0 )
        , _data( 0 )
        , _dataSize( 0 )
        , _refCountMaster( 0 )
        , _func( 0, 0 )
{}

Command::~Command() 
{
    EQASSERTINFO( _refCount == 0, _refCount << ", " << *this );
    _free(); 
}

void Command::retain()
{
    ++_refCount; 
    if( _refCountMaster )
    {
        ++( *_refCountMaster );
        EQASSERT( *_refCountMaster >= _refCount );
    }
}

void Command::release() 
{
    if( _refCountMaster ) // do it before self - otherwise race!
    {
        EQASSERT( *_refCountMaster != 0 );
        EQASSERT( *_refCountMaster >= _refCount );
        --( *_refCountMaster );
    }

    EQASSERT( _refCount != 0 );
    --_refCount;
}

size_t Command::_alloc( NodePtr node, LocalNodePtr localNode,
                        const uint64_t size )
{
    EQ_TS_THREAD( _writeThread );
    EQASSERT( _refCount == 0 );
    EQASSERTINFO( !_func.isValid(), *this );

    size_t allocated = 0;
    if( !_data )
    {
        _dataSize = EQ_MAX( Packet::minSize, size );
        _data = static_cast< Packet* >( malloc( _dataSize ));
        allocated = _dataSize;
    }
    else if( size > _dataSize )
    {
        allocated =  size - _dataSize;
        _dataSize = EQ_MAX( Packet::minSize, size );
        free( _data );
        _data = static_cast< Packet* >( malloc( _dataSize ));
    }

    _node = node;
    _localNode = localNode;
    _refCountMaster = 0;
    _func.clear();
    _packet = _data;
    _packet->size = size;

    return allocated;
}

void Command::_clone( Command& from )
{
    EQ_TS_THREAD( _writeThread );
    EQASSERT( _refCount == 0 );
    EQASSERT( !_func.isValid( ));

    _node = from._node;
    _localNode = from._localNode;
    _packet = from._packet;

    _refCountMaster = &from._refCount;
}

void Command::_free()
{
    EQ_TS_THREAD( _writeThread );
    EQASSERT( _refCount == 0 );
    EQASSERT( !_func.isValid( ));

    if( _data )
        free( _data );

    _data = 0;
    _dataSize = 0;
    _packet = 0;
    _node = 0;
    _localNode = 0;
    _refCountMaster = 0;
}        

bool Command::invoke()
{
    EQASSERT( _func.isValid( ));
    Dispatcher::Func func = _func;
    _func.clear();
    return func( *this );
}

std::ostream& operator << ( std::ostream& os, const Command& command )
{
    if( command.isValid( ))
    {
        os << co::base::disableFlush << "command< ";
        const Packet* packet = command.getPacket() ;
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

        os << ", " << command.getNode() << ", r" << command._refCount << " >"
           << co::base::enableFlush;
    }
    else
        os << "command< empty >";

    if( command._func.isValid( ))
        os << ' ' << command._func << std::endl;
    return os;
}
}
