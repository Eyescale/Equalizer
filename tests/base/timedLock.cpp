
#include <test.h>

#include <eq/base/clock.h>
#include <eq/base/timedLock.h>

#include <iostream>

using namespace eqBase;
using namespace std;

int main( int argc, char **argv )
{
    TimedLock* lock = new TimedLock( Thread::PTHREAD );

    TEST( lock->set( ));

    Clock clock;
    TEST( !lock->set( 1000 ));
    float time = clock.getTimef();

    TEST( time > 1000. );
    TEST( time < 1100. );

    clock.reset();
    TEST( !lock->set( 100 ));
    time = clock.getTimef();

    TEST( time > 100. );
    TEST( time < 200. );

    delete lock;
    return EXIT_SUCCESS;
}

