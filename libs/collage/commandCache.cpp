
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

#include "commandCache.h"

#include "command.h"
#include "node.h"

using namespace std;

#define COMPACT
//#define PROFILE
// 31300 hits, 35 misses, 297640 lookups, 126976b allocated in 31 packets
// 31300 hits, 35 misses, 49228 lookups, 135168b allocated in 34 packets

namespace co
{
CommandCache::CommandCache()
{
    for( size_t i = 0; i < CACHE_ALL; ++i )
    {
        _cache[ i ].push_back( new Command );
        _size[ i ] = 0;
        _position[ i ] = 0;
    }
}

CommandCache::~CommandCache()
{
    flush();

    for( size_t i = 0; i < CACHE_ALL; ++i )
    {
        EQASSERT( _cache[i].size() == 1 );
        EQASSERT( _cache[i].front()->isFree( ));

        delete _cache[i].front();
        _cache[i].clear();
        _size[i] = 0;
    }
}

void CommandCache::flush()
{
    for( size_t i = 0; i < CACHE_ALL; ++i )
    {
        Commands& cache = _cache[ i ];
        for( Commands::const_iterator j = cache.begin(); j != cache.end(); ++j )
        {
            Command* command = *j;
            _size[ i ] -= command->getAllocationSize();
            delete command;
        }
        cache.clear();

        EQASSERT( _size[ i ] == 0 );
        _size[ i ] = 0;
        _position[ i ] = 0;
        cache.push_back( new Command );
    }
}

#ifdef PROFILE
namespace
{
static co::base::a_int32_t _hits;
static co::base::a_int32_t _misses;
static co::base::a_int32_t _lookups;
static co::base::a_int32_t _allocs;
static co::base::a_int32_t _frees;
}
#endif

void CommandCache::_compact( const Cache which )
{
#ifdef COMPACT
    EQ_TS_THREAD( _thread );

    size_t& size = _size[ which ];
    if( size < EQ_64MB || ( _position[ which ] & 0xf ) != 0 )
        return;

    Commands& cache = _cache[ which ];
    for( Commands::iterator i = cache.begin(); i != cache.end(); )
    {
        const Command* cmd = *i;
        if( cmd->isFree( ))
        {            
            const size_t cmdSize = cmd->getAllocationSize();
            size -= cmdSize;
            i = cache.erase( i );
            delete cmd;
#  ifdef PROFILE
            ++_frees;
#  endif
            if( size <= EQ_48MB )
                break;
        }
        else
            ++i;
    }
#endif // COMPACT
}

Command& CommandCache::_newCommand( const Cache which )
{
    EQ_TS_THREAD( _thread );

    Commands& cache = _cache[ which ];
    const size_t cacheSize = cache.size();        
    EQASSERT( cacheSize > 0 );

    size_t& i = _position[ which ];
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
                size_t mem = 0;
                size_t packets = 0;

                for( size_t j = 0; j < CACHE_ALL; ++j )
                {                    
                    packets += _cache[ j ].size();
                    mem += _size[ j ];
                }
                
                EQINFO << _hits << " hits, " << _misses << " misses, "
                       << _lookups << " lookups, " << mem << "b allocated in "
                       << packets << " packets, " << _allocs << " allocated, " 
                       << _frees << " freed" << std::endl;
            }
#endif
            return *command;
        }
    }

#ifdef PROFILE
    ++_misses;
#endif
    
    const size_t add = (cacheSize >> 3) + 1;
    for( size_t j = 0; j < add; ++j )
        cache.push_back( new Command );
#ifdef PROFILE
    _allocs += add;
#endif

    return *( cache.back( ));
}

Command& CommandCache::alloc( NodePtr node, LocalNodePtr localNode, 
                              const uint64_t size )
{
    EQ_TS_THREAD( _thread );

    const Cache which = (size > Packet::minSize) ? CACHE_BIG : CACHE_SMALL;

    _compact( which );
    Command& command = _newCommand( which );
    
    _size[ which ] += command._alloc( node, localNode, size );
    return command;
}

Command& CommandCache::clone( Command& from )
{
    EQ_TS_THREAD( _thread );

    const Cache which = (from->size > Packet::minSize) ? CACHE_BIG :
                                                         CACHE_SMALL;
    Command& command = _newCommand( which );
    
    command._clone( from );
    return command;
}

}
