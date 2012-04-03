
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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
        , _refCountMaster( 0 )
        , _freeCount( freeCounter )
        , _func( 0, 0 )
{}

Command::~Command() 
{
    EQASSERTINFO( _refCount == 0, _refCount << ", " << *this );
    _free(); 
}

void Command::retain()
{
    //LB_TS_THREAD( _writeThread );
    if( ++_refCount == 1 ) // first reference
    {
        EQASSERT( _refCount == 1 ); // ought to be single-threaded in recv
        EQASSERT( _freeCount > 0 );
        --_freeCount;
    }

    if( _refCountMaster )
    {
        if( ++( *_refCountMaster ) == 1 )
            --_freeCount;
        EQASSERT( *_refCountMaster >= _refCount );
    }
}

void Command::release() 
{
    if( _refCountMaster ) // do it before self - otherwise race!
    {
        EQASSERT( *_refCountMaster != 0 );
        if( --( *_refCountMaster ) == 0 ) // last reference
            ++_freeCount;
    }

    EQASSERT( _refCount != 0 );
    if( --_refCount == 0 ) // last reference
        ++_freeCount;
}

size_t Command::alloc_( NodePtr node, LocalNodePtr localNode,
                        const uint64_t size )
{
    LB_TS_THREAD( _writeThread );
    EQASSERT( _refCount == 0 );
    EQASSERTINFO( !_func.isValid(), *this );

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

void Command::clone_( Command& from )
{
    LB_TS_THREAD( _writeThread );
    EQASSERT( _refCount == 0 );
    EQASSERT( !_func.isValid( ));

    _node = from._node;
    _localNode = from._localNode;
    _packet = from._packet;

    _refCountMaster = &from._refCount;
}

void Command::_free()
{
    LB_TS_THREAD( _writeThread );
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

bool Command::operator()()
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

        os << ", " << command.getNode() << ", r" << command._refCount << " >"
           << lunchbox::enableFlush;
    }
    else
        os << "command< empty >";

    if( command._func.isValid( ))
        os << ' ' << command._func << std::endl;
    return os;
}
}
