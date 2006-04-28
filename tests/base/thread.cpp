
#include <eq/base/thread.h>
#include <eq/client/nodeFactory.h>
#include <iostream>

using namespace eqBase;
using namespace std;

#define MAXTHREADS 256

volatile size_t nThreads;
Thread*         thread;

eq::NodeFactory* eq::createNodeFactory() { return new eq::NodeFactory; }

class TestThread : public Thread
{
public:
    TestThread( Thread::Type type ) : Thread( type ) {}
    virtual ssize_t run()
        {
            return EXIT_SUCCESS;
        }
};

class PThread : public TestThread
{
public:
    PThread() : TestThread( Thread::PTHREAD ) {}
};

class Fork : public TestThread
{
public:
    Fork() : TestThread( Thread::FORK ) {}
};


int main( int argc, char **argv )
{
    PThread pthreads[MAXTHREADS];
    Fork    procs[MAXTHREADS];

    for( size_t i=0; i<nThreads; i++ )
    {
        pthreads[i].start();
        procs[i].start();
    }

    for( size_t i=0; i<nThreads; i++ )
    {
        if( !pthreads[i].join( ))
        {
            cerr << "Could not join pthread " << i << endl;
            return EXIT_FAILURE;
        }
        if( !procs[i].join( ))
        {
            cerr << "Could not join process " << i << endl;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

