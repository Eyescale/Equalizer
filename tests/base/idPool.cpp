
#include <eq/base/idPool.h>
#include <eq/client/nodeFactory.h>

#include <stdlib.h>

using namespace eqBase;
using namespace std;

eq::NodeFactory* eq::createNodeFactory() { return new eq::NodeFactory; }

int main( int argc, char **argv )
{
    IDPool pool( IDPool::getMaxCapacity( ));
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

