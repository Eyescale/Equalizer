
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <test.h>

#include <lunchbox/clock.h>
#include <lunchbox/timedLock.h>

#include <iostream>

using namespace lunchbox;
using namespace std;

int main( int argc, char **argv )
{
    TimedLock lock;

    TEST( lock.set( ));

    Clock clock;
    TEST( !lock.set( 1000 ));
    float time = clock.getTimef();

    TESTINFO( time > 999.0f, "was: " << time );
    TESTINFO( time < 1100.0f, "was: " << time );

    clock.reset();
    TEST( !lock.set( 100 ));
    time = clock.getTimef();

    TESTINFO( time > 99.0f, "was: " << time );
    TESTINFO( time < 200.0f, "was: " << time );

    return EXIT_SUCCESS;
}

