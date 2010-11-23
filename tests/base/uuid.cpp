
/* Copyright (c) 2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

// Tests the functionality of universally unique identifiers

#include <test.h>
#include <eq/base/thread.h>
#include <eq/base/uuid.h>
#include <eq/base/init.h>

#define N_UUIDS 10000
#define N_THREADS 10

typedef eq::base::uint128_t uint128_t;

bool testConvertUint128ToUUID();
bool testOperationUint128();
bool testIncrement();

class Thread : public eq::base::Thread
{
public:
    virtual void run()
        {
            size_t i = N_UUIDS;

            while( i-- )
            {
                eq::base::UUID uuid( true );
        
                TESTINFO( hash.find( uuid ) == hash.end(),
                          "Iteration " << N_UUIDS - i );
                hash[ uuid ] = true;
            }
        }

    eq::base::uint128_tHash< bool > hash;
};

int main( int argc, char **argv )
{
    TEST( eq::base::init( argc, argv ));

    // basic tests
    eq::base::UUID id1( true );
    eq::base::UUID id2( true );

    TEST( id1 != eq::base::UUID::ZERO );
    TEST( id1 != id2 );

    id1 = id2;
    TEST( id1 == id2 );
    
    eq::base::UUID* id3 = new eq::base::UUID( id1 );
    eq::base::UUID* id4 = new eq::base::UUID( true );

    TEST( id1 == *id3 );
    TEST( *id4 != *id3 );
    
    *id4 = *id3;
    TEST( *id4 == *id3 );
    
    delete id3;
    delete id4;

    eq::base::UUID id5, id6;
    TEST( id5 == eq::base::UUID::ZERO );
    TEST( id5 == id6 );
    

    // Load tests
    Thread threads[ N_THREADS ];

    eq::base::Clock clock;
    for( size_t i = 0; i < N_THREADS; ++i )
        threads[ i ].start();
    for( size_t i = 0; i < N_THREADS; ++i )
        threads[ i ].join();
    
    EQINFO << N_UUIDS * N_THREADS /clock.getTimef() 
           << " UUID generations and hash ops / ms" << std::endl;

    eq::base::uint128_tHash< bool >& first = threads[0].hash;
    for( size_t i = 1; i < 0; ++i )
    {
        eq::base::uint128_tHash< bool >& current = threads[i].hash;

        for( eq::base::uint128_tHash< bool >::const_iterator j = current.begin();
             j != current.end(); ++j )
        {
            eq::base::UUID uuid = j->first;
            TESTINFO( uuid == j->first, j->first << " = " << uuid );

            std::ostringstream stream;
            stream << uuid;
            uuid = stream.str();
            TESTINFO( uuid == j->first,
                      j->first << " -> " << stream.str() << " -> " << uuid );

            TEST( first.find( uuid ) == first.end( ));
            first[ uuid ] = true;
        }

    }
    testConvertUint128ToUUID();
    testIncrement();
    TEST( eq::base::exit( ));
    return EXIT_SUCCESS;
}

bool testConvertUint128ToUUID()
{
    const uint64_t low = 1212;
    const uint64_t high = 2314;

    uint128_t test128( high, low );
    TEST( test128.getLow() == low && test128.getHigh() == high );

    eq::base::UUID testUUID;
    testUUID = test128;
    const uint128_t compare128 = testUUID;
    TEST( compare128 == test128 );

    return true;
}

bool testIncrement()
{
    {
        uint128_t test128( 0, 0 );
        ++test128;
        TEST( test128.getHigh() == 0 && test128.getLow() == 1 );
        --test128;
        TEST( test128.getHigh() == 0 && test128.getLow() == 0 );
        test128 = test128 + 1;
        TEST( test128.getHigh() == 0 && test128.getLow() == 1 );
        test128 = test128 - 1;
        TEST( test128.getHigh() == 0 && test128.getLow() == 0 );
    }

    {
        uint128_t test128( 0, 0xffffffffffffffffu );
        ++test128;
        TEST( test128.getHigh() == 1 && test128.getLow() == 0 );
        --test128;
        TEST( test128.getHigh() == 0  && 
              test128.getLow() == 0xffffffffffffffffu );
        test128 = test128 + 1;
        TEST( test128.getHigh() == 1 && test128.getLow() == 0 );
        test128 = test128 - 1;
        TEST( test128.getHigh() == 0  && 
              test128.getLow() == 0xffffffffffffffffu );
    }

    {
        eq::base::UUID test128( 0, 0 );
        ++test128;
        TEST( test128.getHigh() == 0 && test128.getLow() == 1 );
        --test128;
        TEST( test128.getHigh() == 0 && test128.getLow() == 0 );
        test128 = test128 + 1;
        TEST( test128.getHigh() == 0 && test128.getLow() == 1 );
        test128 = test128 - 1;
        TEST( test128.getHigh() == 0 && test128.getLow() == 0 );
    }

    {
        eq::base::UUID test128( 0, 0xffffffffffffffffu );
        ++test128;
        TEST( test128.getHigh() == 1 && test128.getLow() == 0 );
        --test128;
        TEST( test128.getHigh() == 0  && 
              test128.getLow() == 0xffffffffffffffffu );
        test128 = test128 + 1;
        TEST( test128.getHigh() == 1 && test128.getLow() == 0 );
        test128 = test128 - 1;
        TEST( test128.getHigh() == 0  && 
              test128.getLow() == 0xffffffffffffffffu );
    }

    return true;
}
