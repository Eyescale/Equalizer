
#include <eq/base/lock.h>
#include <eq/base/thread.h>
#include <iostream>

using namespace eqBase;
using namespace std;

#define MAXTHREADS 32

size_t   nThreads;
Barrier *barrier;
Clock    clock;

class Test : public Thread
{
public:
    Test() : Thread( Thread::PTHREAD ) {}
    virtual int run()
        {
            int nLoops = 1000000 / nThreads;
            const bool master = ( barrier->enter( nThreads ) == 0 );
            if( master )
                clock.reset();

            while( nLoops-- )
            {}

            barrier->enter( nThreads );
            if( master )
                cerr << nThreads << " threads, " << clock.getTime() << "ms"
                     << endl;
        }
}

int main( int argc, char **argv )
{
    barrier = new Barrier( Thread::PTHREAD );
    Thread threads[MAXTHREADS];

    for( nThreads = MAXTHREADS; nThreads>1; nThreads = nThreads>>1 )
    {
        for( size_t i=0; i<nThreads; i++ )
            
    }

    delete barrier;
}

