
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

// Simple test case for bug:
// http://sourceforge.net/tracker/index.php?func=detail&aid=1964341&group_id=170962&atid=856209

// Compile with:
//  g++-4.2 test.cpp -g -fopenmp -lgomp -lpthread -o test

#define NTHREADS 4
#define LOOPSIZE 100000

#include <pthread.h>

#ifdef EQ_USE_OPENMP
#  include <omp.h>
#endif

void* runChild( void* arg )
{
    unsigned char data[ LOOPSIZE ];

#  pragma omp parallel for
    for( int i = 0; i < LOOPSIZE; ++i )
    {
        data[i] = static_cast< unsigned char >( i % 256 );
    }

    return 0;
}

int main( int argc, char **argv )
{
//    omp_set_nested( 1 );

    pthread_attr_t attributes;
    pthread_attr_init( &attributes );
    pthread_attr_setscope( &attributes, PTHREAD_SCOPE_SYSTEM );

	pthread_t threadIDs[NTHREADS];

    for( int i = 0; i < NTHREADS; ++i )
        pthread_create( &threadIDs[i], &attributes, runChild, 0 );

    for( int i = 0; i < NTHREADS; ++i )
        pthread_join( threadIDs[i], 0 );
}
