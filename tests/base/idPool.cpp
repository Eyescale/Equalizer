
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <eq/base/idPool.h>

#include <stdlib.h>

using namespace eqBase;
using namespace std;

int main( int argc, char **argv )
{
    IDPool pool( IDPool::MAX_CAPACITY );
    size_t nLoops = 1000000;

    while( nLoops-- )
    {
#ifdef __linux__
        srandom( getpid( ));
#else
        srandomdev();
#endif
        uint range = (uint)(random() * .0001);
        uint id    = pool.genIDs( range );
        
        //cout << "id: " << id << " range " << range << endl;

        if( id == EQ_INVALID_ID ) 
        {
            cout << "Failed to allocate identifiers after " << nLoops
                 << " allocations" << endl;
            return EXIT_FAILURE;
        }

        pool.freeIDs( id, range );
    }

    return EXIT_SUCCESS;
}

