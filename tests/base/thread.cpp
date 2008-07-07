
#include <test.h>
#include <eq/base/thread.h>
#include <iostream>

using namespace eq::base;
using namespace std;

#define NTHREADS 256

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
    TestThread threads[NTHREADS];

    for( size_t i=0; i<NTHREADS; ++i )
        TEST( threads[i].start( ));

    for( size_t i=0; i<NTHREADS; ++i )
        TEST( threads[i].join( ));

    return EXIT_SUCCESS;
}

