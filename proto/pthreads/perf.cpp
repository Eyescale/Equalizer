
#include <eq/base/barrier.h>
#include <eq/base/clock.h>
#include <eq/base/thread.h>
#include <iostream>

using namespace eqBase;
using namespace std;

#define MAXTHREADS 2048

volatile size_t nThreads;
Barrier*        barrier;
Clock           timer;

class Test : public Thread
{
public:
    Test() : Thread( Thread::PTHREAD ) {}
    virtual ssize_t run()
        {
            int nLoops = MAXTHREADS / nThreads * 100000;
            const bool master = ( barrier->enter( nThreads ) == 0 );
            if( master )
                timer.reset();

            while( nLoops-- )
            {}

            barrier->enter( nThreads );
            if( master )
                cerr << nThreads << " threads, " << timer.getTimef() << "ms"
                     << endl;
            return EXIT_SUCCESS;
        }
};

int main( int argc, char **argv )
{
    barrier = new Barrier( Thread::PTHREAD );
    Test threads[MAXTHREADS];

    for( nThreads = MAXTHREADS; nThreads>1; nThreads = nThreads>>1 )
    {
        for( size_t i=0; i<nThreads; i++ )
            threads[i].start();
        for( size_t i=0; i<nThreads; i++ )
            threads[i].join();
    }

    delete barrier;
    return EXIT_SUCCESS;
}

