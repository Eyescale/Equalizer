
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "test.h"
#include <co/base/atomic.h>
#include <co/base/lock.h>
#include <iostream>

#define MAXTHREADS 256
#define NOPS       1000

volatile size_t nThreads;
eq::base::Lock* lock;

class Thread : public eq::base::Thread
{
public:
    virtual void run()
        {
            for( unsigned i = 0; i < NOPS; ++i )
            {
                lock->set();
                TEST( lock->isSet( ));
                lock->unset();
            }
        }
};

int main( int argc, char **argv )
{
    lock = new eq::base::Lock;
    lock->set();

    Thread threads[MAXTHREADS];
    for( nThreads = MAXTHREADS; nThreads > 0; nThreads = nThreads>>1 )
    {
        for( size_t i = 0; i < nThreads; ++i )
            TEST( threads[i].start( ));

        eq::base::Clock clock;
        lock->unset();

        for( size_t i=0; i<nThreads; i++ )
            TEST( threads[i].join( ));

        TEST( !lock->isSet( ));
        lock->set();

        const float time = clock.getTimef();
        std::cout << 3 /*set, test, unset*/ * NOPS * nThreads / time
                  << " lock ops/ms (" << nThreads << " threads)" << std::endl;
    }

    delete lock;
    return EXIT_SUCCESS;
}

