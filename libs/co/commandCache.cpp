
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

#include "commandCache.h"

#include "command.h"
#include "node.h"

#define COMPACT
//#define PROFILE
// 31300 hits, 35 misses, 297640 lookups, 126976b allocated in 31 packets
// 31300 hits, 35 misses, 49228 lookups, 135168b allocated in 34 packets

namespace co
{
namespace
{
// minimum number of free packets, at least 2
static const int32_t _minFree[2] = { 200, 20 };
static const uint32_t _freeShift = 1; // 'size >> shift' packets can be free


#ifdef PROFILE
static base::a_int32_t _hits;
static base::a_int32_t _misses;
static base::a_int32_t _lookups;
static base::a_int32_t _allocs;
static base::a_int32_t _frees;
#endif
}

CommandCache::CommandCache()
{
    for( size_t i = 0; i < CACHE_ALL; ++i )
    {
        _cache[i].push_back( new Command( _free[i] ));
        _position[i] = _cache[i].begin();
        _free[i] = 1;
        _maxFree[i] = _minFree[i];
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
    }
}

void CommandCache::flush()
{
    for( size_t i = 0; i < CACHE_ALL; ++i )
    {
        Data& cache = _cache[i];
        EQASSERTINFO( size_t( _free[i] ) == cache.size(),
                      _free[i] << " != " << cache.size() );

        for( DataCIter j = cache.begin(); j != cache.end(); ++j )
        {
            Command* command = *j;
            EQASSERT( command->isFree( ));
            delete command;
        }

        cache.clear();
        cache.push_back( new Command( _free[i] ));
        _free[i] = 1;
        _maxFree[i] = _minFree[i];
        _position[i] = _cache[i].begin();
    }
}

void CommandCache::_compact( const Cache which )
{
#ifdef COMPACT
    EQ_TS_THREAD( _thread );

    base::a_int32_t& current = _free[ which ];
    int32_t& maxFree = _maxFree[ which ];
    if( current <= maxFree )
        return;

    const int32_t target = maxFree >> 1;
    EQASSERT( target > 0 );
    Data& cache = _cache[ which ];
    for( Data::iterator i = cache.begin(); i != cache.end(); )
    {
        const Command* cmd = *i;
        if( cmd->isFree( ))
        {
            EQASSERT( current > 0 );
            i = cache.erase( i );
            delete cmd;

#  ifdef PROFILE
            ++_frees;
#  endif
            if( --current <= target )
                break;
        }
        else
            ++i;
    }

    const int32_t num = int32_t( cache.size() >> _freeShift );
    maxFree = EQ_MAX( _minFree[ which ] , num );
    _position[ which ] = cache.begin();
#endif // COMPACT
}

Command& CommandCache::_newCommand( const Cache which )
{
    EQ_TS_THREAD( _thread );
    _compact( which );

    Data& cache = _cache[ which ];
    const uint32_t cacheSize = uint32_t( cache.size( ));
    base::a_int32_t& freeCounter = _free[ which ];
    EQASSERTINFO( size_t( freeCounter ) <= cacheSize,
                  freeCounter << " > " << cacheSize );

    if( freeCounter > 0 )
    {
        EQASSERT( cacheSize > 0 );

        const DataCIter end = _position[ which ];
        DataCIter& i = _position[ which ];

        for( ++i; i != end; ++i )
        {
            if( i == cache.end( ))
                i = cache.begin();
#ifdef PROFILE
            ++_lookups;
#endif
            Command* command = *i;
            if( command->isFree( ))
            {
#ifdef PROFILE
                const long hits = ++_hits;
                if( (hits%1000) == 0 )
                {
                    for( size_t j = 0; j < CACHE_ALL; ++j )
                    {
                        size_t size = 0;
                        const Data& cmds = _cache[ j ];
                        for( DataCIter k = cmds.begin(); k != cmds.end(); ++k )
                            size += (*k)->getAllocationSize();

                        EQINFO << _hits << "/" << _hits + _misses << " hits, "
                               << _lookups << " lookups, " << _free[j] << " of "
                               << cmds.size() << " packets free (min "
                               << _minFree[ j ] << " max " << _maxFree[ j ]
                               << "), " << _allocs << " allocs, " << _frees
                               << " frees, " << size / 1024 << "KB" << std::endl;
                    }
                }
#endif
                return *command;
            }
        }
    }
#ifdef PROFILE
    ++_misses;
#endif
    
    const uint32_t add = (cacheSize >> 3) + 1;
    for( size_t j = 0; j < add; ++j )
        cache.push_back( new Command( freeCounter ));

    freeCounter += add;
    const int32_t num = int32_t( cache.size() >> _freeShift );
    _maxFree[ which ] = EQ_MAX( _minFree[ which ], num );
    _position[ which ] = cache.begin();

#ifdef PROFILE
    _allocs += add;
#endif

    return *( cache.back( ));
}

Command& CommandCache::alloc( NodePtr node, LocalNodePtr localNode, 
                              const uint64_t size )
{
    EQ_TS_THREAD( _thread );
    EQASSERTINFO( size < EQ_BIT48,
                  "Out-of-sync network stream: packet size " << size << "?" );

    const Cache which = (size > Packet::minSize) ? CACHE_BIG : CACHE_SMALL;
    Command& command = _newCommand( which );

    command.alloc_( node, localNode, size );
    return command;
}

Command& CommandCache::clone( Command& from )
{
    EQ_TS_THREAD( _thread );

    const Cache which = (from->size>Packet::minSize) ? CACHE_BIG : CACHE_SMALL;
    Command& command = _newCommand( which );
    
    command.clone_( from );
    return command;
}

std::ostream& operator << ( std::ostream& os, const CommandCache& cache )
{
    const CommandCache::Data& commands =
        cache._cache[ CommandCache::CACHE_SMALL ];
    os << base::disableFlush << "Cache has "
       << commands.size() - cache._free[ CommandCache::CACHE_SMALL ]
       << " used small packets:" << std::endl
       << base::indent << base::disableHeader;

    for( CommandCache::DataCIter i = commands.begin(); i != commands.end(); ++i)
    {
        const Command* command = *i;
        if( !command->isFree( ))
            os << *command << std::endl;
    }
    os << base::enableHeader << base::exdent << base::enableFlush ;
    return os;
}

}
