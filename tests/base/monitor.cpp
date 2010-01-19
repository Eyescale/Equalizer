
/* Copyright (c) 2010, Stefan Eilemann <eile@equalizergraphics.com>
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
#include <eq/base/clock.h>
#include <eq/base/monitor.h>
#include <eq/base/thread.h>
#include <iostream>

#define NLOOPS 200000

eq::base::Monitor< uint64_t > monitor;

class Thread : public eq::base::Thread
{
public:
    virtual ~Thread() {}
    virtual void* run()
        {
            int64_t nOps = NLOOPS;

            eq::base::Clock clock;
            while( nOps-- )
            {
                monitor.waitEQ( nOps );
                monitor = -nOps;
            }

            const float time = clock.getTimef();
            EQINFO << 2*NLOOPS/time << " ops/ms" << std::endl;
            return EXIT_SUCCESS;
        }
};

int main( int argc, char **argv )
{
    Thread waiter;
    uint64_t nOps = NLOOPS;
    
    TEST( waiter.start( ));
    eq::base::Clock clock;

    while( nOps-- )
    {
        monitor = nOps;
        monitor.waitEQ( -nOps );
    }

    const float time = clock.getTimef();

    TEST( waiter.join( ));
    EQINFO << 2*NLOOPS/time << " ops/ms" << std::endl;
    return EXIT_SUCCESS;
}

