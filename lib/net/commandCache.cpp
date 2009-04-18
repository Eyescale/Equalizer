
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

namespace eq
{
namespace net
{
CommandCache::CommandCache()
{}

CommandCache::~CommandCache()
{
    flush();
}

void CommandCache::flush()
{
    for( CommandVector::const_iterator i=_small.begin(); i!=_small.end(); ++i )
        delete *i;
    _small.clear();

    for( CommandVector::const_iterator i = _big.begin(); i != _big.end(); ++i )
        delete *i;
    _big.clear();
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

    CommandVector& cache = (size > Packet::minSize) ? _big : _small;
    for( CommandVector::const_iterator i = cache.begin(); 
         i != cache.end(); ++i )
    {
#ifdef PROFILE
        ++_lookups;
#endif

        Command* command = *i;
        if( command->isFree( ))
        {
#ifdef PROFILE
            const long hits = ++_hits;
            if( (hits%100) == 0 )
            {
                uint64_t mem = _small.size() * Packet::minSize;
                
                for( CommandVector::const_iterator j = _big.begin(); 
                     j != _big.end(); ++j )
                {
                    mem += (*j)->_packetAllocSize;
                }

                EQINFO << _hits << " hits, " << _misses << " misses, "
                       << _lookups << " lookups, " << mem << "b allocated in "
                       << _small.size() + _big.size() << " packets" <<std::endl;
            }
#endif
            command->alloc( node, localNode, size );
            return *command;
        }
    }

#ifdef PROFILE
    ++_misses;
#endif

    Command* command = new Command;
    cache.push_back( command );
    command->alloc( node, localNode, size );
    return *command;
}

}
}
