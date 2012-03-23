
/* Copyright (c) 2010-2011, Stefan Eilemann <eile@equalizergraphics.com>
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

#define EQ_TEST_RUNTIME 300 // seconds
#include "test.h"

#include <lunchbox/clock.h>
#include <lunchbox/monitor.h>
#include <lunchbox/thread.h>
#include <iostream>

#define NLOOPS 200000

lunchbox::Monitor< int64_t > monitor;

class Thread : public lunchbox::Thread
{
public:
    virtual ~Thread() {}
    virtual void run()
        {
            int64_t nOps = NLOOPS;

            lunchbox::Clock clock;
            while( nOps-- )
            {
                monitor.waitEQ( nOps );
                monitor = -nOps;
            }

            const float time = clock.getTimef();
            std::cout << 2*NLOOPS/time << " ops/ms" << std::endl;
        }
};

int main( int argc, char **argv )
{
    Thread waiter;
    int64_t nOps = NLOOPS;
    
    TEST( waiter.start( ));
    lunchbox::Clock clock;

    while( nOps-- )
    {
        monitor = nOps;
        monitor.waitEQ( -nOps );
    }

    const float time = clock.getTimef();

    TEST( waiter.join( ));
    std::cout << 2*NLOOPS/time << " ops/ms" << std::endl;
    return EXIT_SUCCESS;
}

