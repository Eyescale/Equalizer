
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <test.h>
#include <eq/base/idPool.h>
#include <eq/base/rng.h>

#include <stdlib.h>
#include <iostream>

using namespace eqBase;
using namespace std;

int main( int argc, char **argv )
{
    IDPool pool( IDPool::MAX_CAPACITY );
    size_t nLoops = 10000;

    while( nLoops-- )
    {
        RNG rng;
        uint range = static_cast<uint>( rng.get<uint>() * .0001f );
        uint id    = pool.genIDs( range );
        
        TESTINFO( id != EQ_ID_INVALID,
                  "Failed to allocate after " << nLoops << " allocations" );
        
        pool.freeIDs( id, range );
    }

    return EXIT_SUCCESS;
}

