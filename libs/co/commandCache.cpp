
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
#include <lunchbox/atomic.h>

#define COMPACT
//#define PROFILE
// 31300 hits, 35 misses, 297640 lookups, 126976b allocated in 31 packets
// 31300 hits, 35 misses, 49228 lookups, 135168b allocated in 34 packets

namespace co
{
namespace
{
enum Cache
{
    CACHE_SMALL,
    CACHE_BIG,
    CACHE_ALL
};

typedef std::vector< Command* > Data;
typedef Data::const_iterator DataCIter;

// minimum number of free packets, at least 2
static const int32_t _minFree[ CACHE_ALL ] = { 200, 20 };
static const uint32_t _freeShift = 1; // 'size >> shift' packets can be free

#ifdef PROFILE
static base::a_int32_t _hits;
static base::a_int32_t _misses;
static base::a_int32_t _lookups;
static base::a_int32_t _allocs;
static base::a_int32_t _frees;
#endif
}
namespace detail
{
class CommandCache
{
public:
    CommandCache()
        {
            for( size_t i = 0; i < CACHE_ALL; ++i )
            {
                cache[i].push_back( new Command( free[i] ));
                position[i] = cache[i].begin();
                free[i] = 1;
                maxFree[i] = _minFree[i];
            }
        }

    ~CommandCache()
        {
            for( size_t i = 0; i < CACHE_ALL; ++i )
            {
                EQASSERT( cache[i].size() == 1 );
                EQASSERT( cache[i].front()->isFree( ));

                delete cache[i].front();
                cache[i].clear();
            }
        }

    void flush()
        {
            for( size_t i = 0; i < CACHE_ALL; ++i )
            {
                Data& cache_ = cache[i];
                EQASSERTINFO( size_t( free[i] ) == cache_.size(),
                              free[i] << " != " << cache_.size() );

                for( DataCIter j = cache_.begin(); j != cache_.end(); ++j )
                {
                    Command* command = *j;
                    EQASSERT( command->isFree( ));
                    delete command;
                }

                cache_.clear();
                cache_.push_back( new Command( free[i] ));
                free[i] = 1;
                maxFree[i] = _minFree[i];
                position[i] = cache_.begin();
            }
        }

    Command& newCommand( const Cache which )
        {
            _compact( CACHE_SMALL );
            _compact( CACHE_BIG );

            Data& cache_ = cache[ which ];
            const uint32_t cacheSize = uint32_t( cache_.size( ));
            base::a_int32_t& freeCounter = free[ which ];
            EQASSERTINFO( size_t( freeCounter ) <= cacheSize,
                          freeCounter << " > " << cacheSize );

            if( freeCounter > 0 )
            {
                EQASSERT( cacheSize > 0 );

                const DataCIter end = position[ which ];
                DataCIter& i = position[ which ];

                for( ++i; i != end; ++i )
                {
                    if( i == cache_.end( ))
                        i = cache_.begin();
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
                                const Data& cmds = cache_[ j ];
                                for( DataCIter k = cmds.begin();
                                     k != cmds.end(); ++k )
                                {
                                    size += (*k)->getAllocationSize();
                                }
                                EQINFO << _hits << "/" << _hits + _misses
                                       << " hits, " << _lookups << " lookups, "
                                       << _free[j] << " of " << cmds.size()
                                       << " packets free (min " << _minFree[ j ]
                                       << " max " << _maxFree[ j ] << "), "
                                       << _allocs << " allocs, " << _frees
                                       << " frees, " << size / 1024 << "KB"
                                       << std::endl;
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
                cache_.push_back( new Command( freeCounter ));

            freeCounter += add;
            const int32_t num = int32_t( cache_.size() >> _freeShift );
            maxFree[ which ] = EQ_MAX( _minFree[ which ], num );
            position[ which ] = cache_.begin();

#ifdef PROFILE
            _allocs += add;
#endif

            return *( cache_.back( ));
        }

    /** The caches. */
    Data cache[ CACHE_ALL ];

    /** Last lookup position in each cache. */
    DataCIter position[ CACHE_ALL ];

    /** The current number of free items in each cache */
    base::a_int32_t free[ CACHE_ALL ];

    /** The maximum number of free items in each cache */
    int32_t maxFree[ CACHE_ALL ];

private:
    void _compact( const Cache which )
        {
#ifdef COMPACT
            base::a_int32_t& currentFree = free[ which ];
            int32_t& maxFree_ = maxFree[ which ];
            if( currentFree <= maxFree_ )
                return;

            const int32_t target = maxFree_ >> 1;
            EQASSERT( target > 0 );
            Data& cache_ = cache[ which ];
            for( Data::iterator i = cache_.begin(); i != cache_.end(); )
            {
                const Command* cmd = *i;
                if( cmd->isFree( ))
                {
                    EQASSERT( currentFree > 0 );
                    i = cache_.erase( i );
                    delete cmd;

#  ifdef PROFILE
                    ++_frees;
#  endif
                    if( --currentFree <= target )
                        break;
                }
                else
                    ++i;
            }

            const int32_t num = int32_t( cache_.size() >> _freeShift );
            maxFree_ = EQ_MAX( _minFree[ which ] , num );
            position[ which ] = cache_.begin();
#endif // COMPACT
        }
};
}

CommandCache::CommandCache()
        : _impl( new detail::CommandCache )
{}

CommandCache::~CommandCache()
{
    flush();
}

void CommandCache::flush()
{
    _impl->flush();
}

Command& CommandCache::alloc( NodePtr node, LocalNodePtr localNode, 
                              const uint64_t size )
{
    EQ_TS_THREAD( _thread );
    EQASSERTINFO( size < EQ_BIT48,
                  "Out-of-sync network stream: packet size " << size << "?" );

    const Cache which = (size > Packet::minSize) ? CACHE_BIG : CACHE_SMALL;
    Command& command = _impl->newCommand( which );

    command.alloc_( node, localNode, size );
    return command;
}

Command& CommandCache::clone( Command& from )
{
    EQ_TS_THREAD( _thread );

    const Cache which = (from->size>Packet::minSize) ? CACHE_BIG : CACHE_SMALL;
    Command& command = _impl->newCommand( which );
    
    command.clone_( from );
    return command;
}

std::ostream& operator << ( std::ostream& os, const CommandCache& cache )
{
    const Data& commands = cache._impl->cache[ CACHE_SMALL ];
    os << base::disableFlush << "Cache has "
       << commands.size() - cache._impl->free[ CACHE_SMALL ]
       << " used small packets:" << std::endl
       << base::indent << base::disableHeader;

    for( DataCIter i = commands.begin(); i != commands.end(); ++i )
    {
        const Command* command = *i;
        if( !command->isFree( ))
            os << *command << std::endl;
    }
    return os << base::enableHeader << base::exdent << base::enableFlush ;
}

}
