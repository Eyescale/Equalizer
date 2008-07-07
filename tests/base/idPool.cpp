
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <test.h>
#include <eq/base/idPool.h>
#include <eq/base/rng.h>

#include <stdlib.h>
#include <iostream>

using namespace eq::base;
using namespace std;

int main( int argc, char **argv )
{
    IDPool pool( IDPool::MAX_CAPACITY );
    size_t nLoops = 10000;

    while( nLoops-- )
    {
        RNG rng;
        uint32_t range = static_cast<uint32_t>( rng.get<uint32_t>() * .0001f );
        uint32_t id    = pool.genIDs( range );
        
        TESTINFO( id != EQ_ID_INVALID,
                  "Failed to allocate after " << nLoops << " allocations" );
        
        pool.freeIDs( id, range );
    }

    return EXIT_SUCCESS;
}

