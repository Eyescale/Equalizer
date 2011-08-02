
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

// Tests the functionality of the random number generator

#include <test.h>

#include <co/base/clock.h>
#include <co/base/init.h>
#include <co/base/rng.h>

#define MAXLOOPS 100000

#define TESTLOOP( type, min, max )                                      \
{                                                                       \
    size_t i = MAXLOOPS;                                                \
    while( --i )                                                        \
        if( rng.get< type >() <= ( min ))                               \
            break;                                                      \
    TESTINFO( i, "Did never get value below " << (min) << " for " << #type ); \
    i = MAXLOOPS;                                                       \
    while( --i )                                                        \
        if( rng.get< type >() >= ( max ))                               \
            break;                                                      \
    TESTINFO( i, "Did never get value above " << (max) << " for " << #type ); \
    {                                                                   \
        const type value = rng.get< type >();                           \
        i = MAXLOOPS;                                                   \
        while( --i )                                                    \
            if( rng.get< type >() != value )                            \
                break;                                                  \
        TESTINFO( i, "Always get the same value " << value << " for "   \
                  << #type );                                           \
    }                                                                   \
}

int main( int argc, char **argv )
{
    TEST( co::base::init( argc, argv ));

    co::base::RNG rng;

    TESTLOOP( uint8_t,  0,        255 );
    TESTLOOP( uint16_t, 50,       65000 );
    TESTLOOP( uint32_t, 1<<20,    1u<<12 );
    TESTLOOP( uint64_t, 1ull<<52, 1ull<<12 );

    TESTLOOP( int8_t,  -126,       127 );
    TESTLOOP( int16_t, -32000,     32000 );
    TESTLOOP( int32_t, -1<<5,      1<<5 );
    TESTLOOP( int64_t, -1<<10,     1<<10 );

    TESTLOOP( float,  0.1f, 0.9f );
    TESTLOOP( double, 0.1,  0.9 );

    co::base::Clock clock;
    for( size_t i = 0; i < MAXLOOPS; ++i )
        rng.get< uint64_t >();
    std::cout << float( MAXLOOPS ) / clock.getTimef() << " rng/ms"
              << std::endl;

    TEST( co::base::exit( ));
    return EXIT_SUCCESS;
}
