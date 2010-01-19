
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

class Thread : public eq::base::Thread
{
public:
    virtual void* run()
        {
            size_t i = N_UUIDS;

            while( i-- )
            {
                eq::base::UUID uuid( true );
        
                TESTINFO( hash.find( uuid ) == hash.end(),
                          "Iteration " << N_UUIDS - i );
                hash[ uuid ] = true;
            }
            return EXIT_SUCCESS;
        }

    eq::base::UUIDHash< bool > hash;
};

int main( int argc, char **argv )
{
    TEST( eq::base::init( ));

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

    eq::base::UUIDHash< bool >& first = threads[0].hash;
    for( size_t i = 1; i < N_THREADS; ++i )
    {
        eq::base::UUIDHash< bool >& current = threads[i].hash;

        for( eq::base::UUIDHash< bool >::const_iterator j = current.begin();
             j != current.end(); ++j )
        {
            eq::base::UUID uuid = j->first;
            TEST( uuid == j->first );

            std::ostringstream stream;
            stream << uuid;
            uuid = stream.str();
            TESTINFO( uuid == j->first,
                      j->first << " -> " << stream.str() << " -> " << uuid );

            TEST( first.find( uuid ) == first.end( ));
            first[ uuid ] = true;
        }
    }

    TEST( eq::base::exit( ));
    return EXIT_SUCCESS;
}
