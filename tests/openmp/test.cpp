
/* Copyright (c) 2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

// Simple test case for bug:
// http://sourceforge.net/tracker/index.php?func=detail&aid=1964341&group_id=170962&atid=856209

// Compile with:
//  g++-4.2 test.cpp -g -fopenmp -lgomp -lpthread -o test

#define NTHREADS 4
#define LOOPSIZE 100000

#include <pthread.h>
#include <omp.h>

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
