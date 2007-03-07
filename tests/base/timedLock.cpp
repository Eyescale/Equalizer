
#include <test.h>

#include <eq/base/clock.h>
#include <eq/base/timedLock.h>

#include <iostream>

using namespace eqBase;
using namespace std;

int main( int argc, char **argv )
{
    TimedLock lock;

    TEST( lock.set( ));

    Clock clock;
    TEST( !lock.set( 1000.0f ));
    float time = clock.getTimef();

    TESTINFO( time > 999.0f, "was: " << time );
    TESTINFO( time < 1100.0f, "was: " << time );

    clock.reset();
    TEST( !lock.set( 100.0f ));
    time = clock.getTimef();

    TESTINFO( time > 99.0f, "was: " << time );
    TESTINFO( time < 200.0f, "was: " << time );

    return EXIT_SUCCESS;
}

