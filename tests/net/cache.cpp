
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <test.h>

#include <eq/net/command.h>
#include <eq/net/init.h>
#include <eq/net/instanceCache.h>
#include <eq/net/objectVersion.h>

#include <eq/base/rng.h>
#include <eq/base/thread.h>

#include "net/objectPackets.h" // private header

// Tests the functionality of the network packet cache

#define N_READER 1
#define PACKET_SIZE 4096
#define RUNTIME 5000

eq::base::Clock _clock;

class Reader : public eq::base::Thread
{
public:
    Reader( eq::net::InstanceCache& cache )
            : Thread(),
              _cache( cache )
        {}
    virtual ~Reader(){}

protected:
    virtual void run()
        {
            size_t hits = 0;
            size_t ops = 0;
            while( _clock.getTime64() < RUNTIME )
            {
                const uint32_t key = _rng.get< uint16_t >();
                if( _cache[ key ] != eq::net::InstanceCache::Data::NONE )
                {
                    ++hits;
                    _cache.release( key );
                }
                ++ops;
            }
            const uint64_t time = _clock.getTime64();
            EQINFO << hits << " read hits in " << ops << " operations, "
                   << ops / time << " ops/ms" << std::endl;
        }

private:
    eq::net::InstanceCache& _cache;
    eq::base::RNG _rng;
};

int main( int argc, char **argv )
{
    eq::net::init( argc, argv );
    eq::net::CommandCache commandCache;

    eq::net::Command& command = commandCache.alloc( 0, 0, PACKET_SIZE );
    eq::net::ObjectInstancePacket* packet = 
        command.getPacket< eq::net::ObjectInstancePacket >();
    *packet = eq::net::ObjectInstancePacket();
    packet->last = true;
    packet->dataSize = PACKET_SIZE;
    packet->version = 1;
    packet->sequence = 0;

    Reader** readers = static_cast< Reader** >( 
        alloca( N_READER * sizeof( Reader* )));

    eq::net::InstanceCache cache;
    eq::base::RNG rng;

    size_t hits = 0;
    size_t ops = 0;

    for( uint32_t key = 0; key < 65536; ++key ) // Fill cache
        if( !cache.add( eq::net::ObjectVersion( key, 1 ), 1, command ))
            break;

    _clock.reset();
    for( size_t i = 0; i < N_READER; ++i )
    {
        readers[ i ] = new Reader( cache );
        readers[ i ]->start();
    }

    while( _clock.getTime64() < RUNTIME )
    {
        const eq::net::ObjectVersion key( rng.get< uint16_t >(), 1 );
        if( cache[ key.identifier ] != eq::net::InstanceCache::Data::NONE )
        {
            TEST( cache.release( key.identifier ));
            ++ops;
            if( cache.erase( key.identifier ))
            {
                TEST( cache.add( key, 1, command ));
                ops += 2;
                hits += 2;
            }
        }
        else if( cache.add( key, 1, command ))
            ++hits;
        ++ops;
    }

    const uint64_t time = _clock.getTime64();
    EQINFO << hits << " write hits in " << ops << " operations, "
           << ops / time << " ops/ms" << std::endl;

    for( size_t i = 0; i < N_READER; ++i )
    {
        readers[ i ]->join();
        delete readers[ i ];
    }

    EQINFO << cache << std::endl;

    for( uint32_t key = 0; key < 65536; ++key )
    {
        if( cache[ key ] != eq::net::InstanceCache::Data::NONE )
        {
            TEST( cache.release( key ));
            TEST( cache.erase( key ));
        }
    }

    for( uint32_t key = 0; key < 65536; ++key )
    {
        TEST( cache[ key ] == eq::net::InstanceCache::Data::NONE );
    }

    EQINFO << cache << std::endl;

    TEST( cache.getSize() == 0 );
    TEST( command.isFree( ));
    return EXIT_SUCCESS;
}
