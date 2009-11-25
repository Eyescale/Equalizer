 
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "idPool.h"

#include "debug.h"
#include "scopedMutex.h"

#include <sstream>

using namespace std;

#define COMPRESS_INTERVAL 10000

namespace eq
{
namespace base
{
IDPool::IDPool( const uint32_t initialCapacity )
        : _compressCounter(0)
{
    if( initialCapacity )
    {
        Block* block = new Block();
        block->start = 1;
        block->range = initialCapacity;
        
        _freeIDs.push_front( block );
    }
}

IDPool::~IDPool()
{
    while( !_freeIDs.empty( ))
    {
        Block* block = _freeIDs.front();
        _freeIDs.pop_front();
        delete block;
    }
    while( !_blockCache.empty( ))
    {
        Block* block = _blockCache.front();
        _blockCache.pop_front();
        delete block;
    }
}

uint32_t IDPool::genIDs( const uint32_t range )
{
    ScopedMutex mutex( _lock );

    const uint32_t id = _genIDs( range );
    if( id )
        return id;

    _compressIDs();
    return _genIDs( range );
}

uint32_t IDPool::_genIDs( const uint32_t range )
{
    for( list<Block*>::iterator i = _freeIDs.begin(); i != _freeIDs.end(); ++i )
    {
        Block* block = *i;

        if( range <= block->range )
        {
            uint32_t start = block->start;

            block->range -= range;
            block->start += range;

            if( block->range == 0 )
            {
                _freeIDs.erase( i );
                _blockCache.push_front( block );
            }

            return start;
        }
    }
    return EQ_ID_INVALID;
}

void IDPool::freeIDs( const uint32_t start, const uint32_t range )
{
    ScopedMutex mutex( _lock );

    Block* block;

    if( _blockCache.empty( ))
        block = new Block;
    else
    {
        block = _blockCache.front();
        _blockCache.pop_front();
    }

    block->start = start;
    block->range = range;
    _freeIDs.push_front( block );

    ++_compressCounter;
    if( ( _compressCounter % COMPRESS_INTERVAL) == 0 )
        _compressIDs();
}

void IDPool::_compressIDs()
{
    list<Block*> blocks;
    bool         compress = true;
    
    while( compress )
    {
        blocks.clear();
        compress = false;

        while( !_freeIDs.empty( ))
        {
            Block* block = _freeIDs.front();
            _freeIDs.pop_front();

            for( list<Block*>::iterator i = _freeIDs.begin();  
                 i != _freeIDs.end(); /*nop*/ )
            {
                Block* block2 = *i;
                list<Block*>::iterator currentIter = i;
                ++i;
                
                if(( block->start + block->range ) == block2->start )
                {
                    block->range += block2->range;
                    _freeIDs.erase( currentIter );

                    _blockCache.push_front( block2 );
                    compress = true;
                }
                else if( block->start == ( block2->start + block2->range ))
                {
                    block->start  = block2->start;
                    block->range += block2->range;
                    _freeIDs.erase( currentIter );

                    _blockCache.push_front( block2 );
                    compress = true;
                }
             }
            
            blocks.push_front( block );
        }

        _freeIDs.swap( blocks );
    }
}
}
}

