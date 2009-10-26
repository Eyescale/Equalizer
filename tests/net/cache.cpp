
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#define EQ_INSTRUMENT_CACHE

#include <eq/net/instanceCache.h>
#include <eq/net/init.h>
#include "../../lib/net/instanceCache.cpp"

#include <eq/base/thread.h>
#include <eq/net/command.h>

// Tests the functionality of the network packet cache

#define N_READER 1
#define PACKET_SIZE 4096
#define RUNTIME 5000

eq::base::Clock _clock;

class Reader : public eq::base::Thread
{
public:
    Reader( eq::net::InstanceCache< uint16_t >& cache )
            : Thread(),
              _cache( cache )
        {}
    virtual ~Reader(){}

protected:
    virtual void* run()
        {
            size_t hits = 0;
            size_t ops = 0;
            while( _clock.getTime64() < RUNTIME )
            {
                const uint16_t key = _rng.get< uint16_t >();
                if( _cache.pin( key ))
                {
                    ++hits;
                    _cache.unpin( key );
                }
                ++ops;
            }
            const uint64_t time = _clock.getTime64();
            EQINFO << hits << " read hits in " << ops << " operations, "
                   << ops / time << " ops/ms" << std::endl;

            return EXIT_SUCCESS;
        }

private:
    eq::net::InstanceCache< uint16_t >& _cache;
    eq::base::RNG _rng;
};

int main( int argc, char **argv )
{
    eq::net::init( argc, argv );
    eq::net::CommandCache commandCache;

    eq::net::Command& command = commandCache.alloc( 0, 0, PACKET_SIZE );
    Reader** readers = static_cast< Reader** >( 
        alloca( N_READER * sizeof( Reader* )));

    eq::net::InstanceCache< uint16_t > cache;
    eq::base::RNG rng;

    size_t hits = 0;
    size_t ops = 0;

    for( size_t key = 0; key < 65536; ++key ) // Fill cache
        if( !cache.add( key, &command, false ))
            break;

    _clock.reset();
    for( size_t i = 0; i < N_READER; ++i )
    {
        readers[ i ] = new Reader( cache );
        readers[ i ]->start();
    }

    while( _clock.getTime64() < RUNTIME )
    {
        const uint16_t key = rng.get< uint16_t >();
        if( (ops % 10 ) == 0 )
        {
            if( cache.erase( key ))
            {
                TEST( cache.add( key, &command, false ));
                ++ops;
                hits += 2;
            }
        }
        else if( cache.add( key, &command, false ))
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

    for( size_t key = 0; key < 65536; ++key )
        cache.erase( key );

    for( size_t key = 0; key < 65536; ++key )
    {
        TEST( !cache.pin( key ));
    }

    EQINFO << cache << std::endl;

    TEST( cache.getSize() == 0 );
    TEST( command.isFree( ));
    return EXIT_SUCCESS;
}
