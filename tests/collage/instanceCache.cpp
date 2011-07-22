
/* Copyright (c) 2009-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef NDEBUG
#  define EQ_TEST_RUNTIME 600000 // 10min
#endif

#include <test.h>

#include <co/command.h>
#include <co/commandCache.h>
#include <co/init.h>
#include <co/instanceCache.h>
#include <co/objectVersion.h>

#include <co/base/rng.h>
#include <co/base/thread.h>

#include <co/objectPackets.h> // private header

// Tests the functionality of the instance cache

#define N_READER 1
#define RUNTIME 5000
#ifdef NDEBUG
#  define PACKET_SIZE 4096
#else
#  define PACKET_SIZE 2048
#endif

co::base::Clock _clock;

class Reader : public co::base::Thread
{
public:
    Reader( co::InstanceCache& cache )
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
                const co::base::UUID key( _rng.get< uint16_t >(), 0 );
                if( _cache[ key ] != co::InstanceCache::Data::NONE )
                {
                    ++hits;
                    _cache.release( key, 1 );
                }
                ++ops;
            }
            const uint64_t time = _clock.getTime64();
            std::cout << hits << " read hits in " << ops << " operations, "
                      << ops / time << " ops/ms" << std::endl;
        }

private:
    co::InstanceCache& _cache;
    co::base::RNG _rng;
};

int main( int argc, char **argv )
{
    co::init( argc, argv );
    co::CommandCache commandCache;
    co::LocalNodePtr node = new co::LocalNode;

    co::Command& command = commandCache.alloc( node, node, PACKET_SIZE );
    co::ObjectInstancePacket* packet = 
        command.get< co::ObjectInstancePacket >();
    *packet = co::ObjectInstancePacket();
    packet->last = true;
    packet->dataSize = PACKET_SIZE;
    packet->version = 1;
    packet->sequence = 0;

    Reader** readers = static_cast< Reader** >( 
        alloca( N_READER * sizeof( Reader* )));

    co::InstanceCache cache;
    co::base::RNG rng;

    size_t hits = 0;
    size_t ops = 0;

    for( co::base::UUID key; key.low() < 65536; ++key ) // Fill cache
        if( !cache.add( co::ObjectVersion( key, 1 ), 1, command ))
            break;

    _clock.reset();
    for( size_t i = 0; i < N_READER; ++i )
    {
        readers[ i ] = new Reader( cache );
        readers[ i ]->start();
    }

    while( _clock.getTime64() < RUNTIME )
    {
        const co::base::UUID id( 0, rng.get< uint16_t >( ));
        const co::ObjectVersion key( id, 1 );
        if( cache[ key.identifier ] != co::InstanceCache::Data::NONE )
        {
            TEST( cache.release( key.identifier, 1 ));
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
    std::cout << hits << " write hits in " << ops << " operations, "
              << ops / time << " ops/ms" << std::endl;

    for( size_t i = 0; i < N_READER; ++i )
    {
        readers[ i ]->join();
        delete readers[ i ];
    }

    std::cout << cache << std::endl;

    for( co::base::UUID key; key.low() < 65536; ++key ) // Fill cache
    {
        if( cache[ key ] != co::InstanceCache::Data::NONE )
        {
            TEST( cache.release( key, 1 ));
            TEST( cache.erase( key ));
        }
    }

    for( co::base::UUID key; key.low() < 65536; ++key ) // Fill cache
    {
        TEST( cache[ key ] == co::InstanceCache::Data::NONE );
    }

    std::cout << cache << std::endl;

    TESTINFO( cache.getSize() == 0, cache.getSize( ));
    TEST( command.isFree( ));

    TEST( co::exit( ));
    return EXIT_SUCCESS;
}
