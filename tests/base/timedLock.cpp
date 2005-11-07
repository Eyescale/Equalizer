
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
    TEST( clock.getTime() > 1000. );

    delete lock;
    return EXIT_SUCCESS;
}

