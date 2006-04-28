
#include <eq/base/lock.h>
#include <eq/base/thread.h>
#include <eq/client/nodeFactory.h>
#include <iostream>

using namespace eqBase;
using namespace std;

#define MAXTHREADS 256

volatile size_t nThreads;
Lock*           lock;

eq::NodeFactory* eq::createNodeFactory() { return new eq::NodeFactory; }

class Test : public Thread
{
public:
    Test() : Thread( Thread::PTHREAD ) {}
    virtual ssize_t run()
        {
            lock->set();
            lock->unset();
            return EXIT_SUCCESS;
        }
};

int main( int argc, char **argv )
{
    lock = new Lock( Thread::PTHREAD );
    lock->set();

    Test threads[MAXTHREADS];
    for( nThreads = MAXTHREADS; nThreads>1; nThreads = nThreads>>1 )
    {
        cerr << nThreads << " threads" << endl;
        for( size_t i=0; i<nThreads; i++ )
            threads[i].start();

        lock->unset();

        for( size_t i=0; i<nThreads; i++ )
        {
            if( !threads[i].join( ))
            {
                cerr << "Could not join thread " << i << endl;
                exit(EXIT_FAILURE);
            }
        }
        lock->set();
    }

    delete lock;
    return EXIT_SUCCESS;
}

