
#include <eq/base/thread.h>
#include <iostream>

using namespace eqBase;
using namespace std;

#define MAXTHREADS 256

volatile size_t nThreads;
Thread*         thread;

class TestThread : public Thread
{
public:
    virtual void* run()
        {
            return EXIT_SUCCESS;
        }
};

int main( int argc, char **argv )
{
    TestThread threads[MAXTHREADS];

    for( size_t i=0; i<nThreads; i++ )
    {
        threads[i].start();
    }

    for( size_t i=0; i<nThreads; i++ )
    {
        if( !threads[i].join( ))
        {
            cerr << "Could not join thread " << i << endl;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

