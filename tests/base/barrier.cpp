
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
    Test() : Thread( Thread::PTHREAD ) {}
    virtual ssize_t run()
        {
            const size_t num = barrier->enter( nThreads );
            cerr << " " << num;
            return EXIT_SUCCESS;
        }
};

int main( int argc, char **argv )
{
    barrier = new Barrier( Thread::PTHREAD );
    Test threads[MAXTHREADS];

    for( nThreads = MAXTHREADS; nThreads>1; nThreads = nThreads>>1 )
    {
        cerr << endl << nThreads << " threads";

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

