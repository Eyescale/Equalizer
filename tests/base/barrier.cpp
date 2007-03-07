
#include <test.h>

#include <eq/base/barrier.h>
#include <eq/base/thread.h>
#include <iostream>

using namespace eqBase;
using namespace std;

#define MAXTHREADS 256

volatile size_t nThreads;
Barrier*        barrier;

class Test : public Thread
{
public:
    virtual void* run()
        {
            const size_t num = barrier->enter( nThreads );
            TEST( num < nThreads );
            return EXIT_SUCCESS;
        }
};

int main( int argc, char **argv )
{
    barrier = new Barrier;
    Test threads[MAXTHREADS];

    for( nThreads = MAXTHREADS; nThreads>1; nThreads = nThreads>>1 )
    {
        for( size_t i=0; i<nThreads; i++ )
            threads[i].start();

        for( size_t i=0; i<nThreads; i++ )
        {
            if( !threads[i].join( ))
            {
                cerr << "Could not join thread " << i << endl;
                exit(EXIT_FAILURE);
            }
        }
    }

    delete barrier;
    return EXIT_SUCCESS;
}

