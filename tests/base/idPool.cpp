
#include <eq/base/idPool.h>

#include <stdlib.h>

using namespace eqBase;
using namespace std;

int main( int argc, char **argv )
{
    IDPool pool;
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

        if( id == 0 ) 
        {
            cout << "Failed to allocate identifiers after " << nLoops
                 << " allocations" << endl;
            return EXIT_FAILURE;
        }

        pool.freeIDs( id, range );
    }

    return EXIT_SUCCESS;
}

