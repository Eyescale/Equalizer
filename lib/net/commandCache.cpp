
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

//#define PROFILE
// 31300 hits, 35 misses, 297640 lookups, 126976b allocated in 31 packets
// 31300 hits, 35 misses, 49228 lookups, 135168b allocated in 34 packets

namespace eq
{
namespace net
{
CommandCache::CommandCache()
        : _smallPos( 0 )
        , _bigPos( 0 )
{
    _small.push_back( new Command );
    _big.push_back( new Command );
}

CommandCache::~CommandCache()
{
    flush();
    EQASSERT( _small.size() == 1 );
    EQASSERT( _big.size() == 1 );

    delete _small.front();
    delete _big.front();
}

void CommandCache::flush()
{
    for( CommandVector::const_iterator i=_small.begin(); i!=_small.end(); ++i )
        delete *i;
    _small.clear();
    _smallPos = 0;

    for( CommandVector::const_iterator i = _big.begin(); i != _big.end(); ++i )
        delete *i;
    _big.clear();
    _bigPos = 0;

    _small.push_back( new Command );
    _big.push_back( new Command );
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

    const bool isBig = (size > Packet::minSize);
    CommandVector& cache = isBig ? _big : _small;

    EQASSERT( !cache.empty( ));

    size_t& i = isBig ? _bigPos : _smallPos;
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
                uint64_t mem( _small.size() * Packet::minSize );
                size_t free( 0 );

                for( CommandVector::const_iterator j = _small.begin(); 
                     j != _small.end(); ++j )
                {
                    const Command* cmd = *j;
                    if( cmd->isFree( ))
                        ++free;
                }
                for( CommandVector::const_iterator j = _big.begin(); 
                     j != _big.end(); ++j )
                {
                    const Command* cmd = *j;
                    mem += cmd->_packetAllocSize;
                    if( cmd->isFree( ))
                        ++free;
                }
                
                EQINFO << _hits << " hits, " << _misses << " misses, "
                       << _lookups << " lookups, " << mem << "b allocated in "
                       << _small.size() +_big.size() << " packets (" << free
                       << " free)" << std::endl;
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
