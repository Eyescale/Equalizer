
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "commandCache.h"

#include "command.h"
#include "node.h"
#include "packets.h"

using namespace std;

#define COMPACT
//#define PROFILE
// 31300 hits, 35 misses, 297640 lookups, 126976b allocated in 31 packets
// 31300 hits, 35 misses, 49228 lookups, 135168b allocated in 34 packets

namespace eq
{
namespace net
{
CommandCache::CommandCache()
{
    for( size_t i = 0; i < CACHE_ALL; ++i )
    {
        _caches[ i ].push_back( new Command );
        _positions[ i ] = 0;
    }
}

CommandCache::~CommandCache()
{
    flush();

    for( size_t i = 0; i < CACHE_ALL; ++i )
    {
        EQASSERT( _caches[i].size() == 1 );
        delete _caches[i].front();
        _caches[i].clear();
    }
}

void CommandCache::flush()
{
    for( size_t i = 0; i < CACHE_ALL; ++i )
    {
        CommandVector& cache = _caches[ i ];
        for( CommandVector::const_iterator j = cache.begin(); 
             j != cache.end(); ++j )
        {
            delete *j;
        }
        cache.clear();
        _positions[ i ] = 0;
        cache.push_back( new Command );
    }
}

#ifdef PROFILE
namespace
{
static base::mtLong _hits;
static base::mtLong _misses;
static base::mtLong _lookups;
}
#endif

Command& CommandCache::alloc( NodePtr node, NodePtr localNode, 
                              const uint64_t size )
{
    CHECK_THREAD( _thread );

    const Cache which = (size > Packet::minSize) ? CACHE_BIG : CACHE_SMALL;
    CommandVector& cache = _caches[ which ];
    size_t& i = _positions[ which ];

    EQASSERT( !cache.empty( ));

#ifdef COMPACT
    const unsigned highWater = (which == CACHE_BIG) ? 10 : 1000;
    if( cache.size() > highWater && (i%10) == 0 )
    {
        int64_t keepFree( 30 * 1024 * 1024 );
        keepFree = EQ_MAX( keepFree, 10 );
#  ifdef PROFILE
        size_t freed( 0 );
#  endif

        for( i = 0; i < cache.size(); ++i )
        {
            const Command* cmd = cache[i];
            if( !cmd->isFree( ))
                continue;
            
            if( keepFree > 0 )
            {
                keepFree -= cmd->getAllocationSize();
                continue;
            }

            cache.erase( cache.begin() + i );
            delete cmd;
#  ifdef PROFILE
            ++freed;
#  endif
        }
#  ifdef PROFILE
        EQINFO << "Freed " << freed << " packets, remaining " << cache.size()
               << std::endl;
#  endif
    }
#endif

    const size_t cacheSize = cache.size();        
    i = i % cacheSize;

    const size_t end = i + cacheSize;
    
    for( ++i; i <= end; ++i )
    {
#ifdef PROFILE
        ++_lookups;
#endif
        
        Command* command = cache[ i%cacheSize ];
        if( command->isFree( ))
        {
#ifdef PROFILE
            const long hits = ++_hits;
            if( (hits%1000) == 0 )
            {
                size_t mem( 0 );
                size_t free( 0 );
                size_t packets( 0 );

                for( size_t j = 0; j < CACHE_ALL; ++j )
                {
                    CommandVector& cache1 = _caches[ j ];
                    packets += cache1.size();

                    for( CommandVector::const_iterator k = cache1.begin(); 
                         k != cache1.end(); ++k )
                    {
                        const Command* cmd = *k;
                        mem += cmd->_packetAllocSize;
                        if( cmd->isFree( ))
                            ++free;
                    }
                }
                
                EQINFO << _hits << " hits, " << _misses << " misses, "
                       << _lookups << " lookups, " << mem << "b allocated in "
                       << packets << " packets (" << free << " free)"
                       << std::endl;
            }
#endif
            command->alloc( node, localNode, size );
            return *command;
        }
    }

#ifdef PROFILE
    ++_misses;
#endif
    
    const size_t add = (cacheSize >> 3) + 1;
    for( size_t j = 0; j < add; ++j )
        cache.push_back( new Command );

    Command* command = cache.back();
    command->alloc( node, localNode, size );
    return *command;
}

}
}
