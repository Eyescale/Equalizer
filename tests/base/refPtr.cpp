
#include <test.h>
#include <eq/base/referenced.h>
#include <eq/base/refPtr.h>
#include <eq/base/thread.h>
#include <iostream>

using namespace eq::base;
using namespace std;

#define NTHREADS 16
#define NREFS    100000

class Foo : public Referenced
{
public:
    Foo() {}
private:
    virtual ~Foo() {}
};

typedef RefPtr<Foo> FooPtr;
FooPtr foo;

class TestThread : public Thread
{
public:
    virtual void* run()
        {
            FooPtr myFoo;
            for( size_t i = 0; i<NREFS; ++i )
            {
                myFoo = foo;
                foo   = myFoo;
                myFoo = 0;
            }

            return EXIT_SUCCESS;
        }
};

int main( int argc, char **argv )
{
    TestThread threads[NTHREADS];

    Clock clock;
    foo = new Foo;
    for( size_t i=0; i<NTHREADS; ++i )
        TEST( threads[i].start( ));

    for( size_t i=0; i<NTHREADS; ++i )
        TEST( threads[i].join( ));

    const float time = clock.getTimef();
    cout << time << " ms for " << 3*NREFS << " reference operations in " 
         << NTHREADS << " threads (" << time/(3*NREFS*NTHREADS)*1000000
         << "ns/op)" 
         << endl;

    TEST( foo->getRefCount() == 1 );
    foo = 0;

    return EXIT_SUCCESS;
}

