
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

#include "test.h"
#include <co/base/spinLock.h>
#include <iostream>

#define MAXTHREADS 128
#define NOPS       100000

volatile size_t nThreads;
co::base::SpinLock* lock;

class Thread : public co::base::Thread
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
    lock = new co::base::SpinLock;
    lock->set();

    Thread threads[MAXTHREADS];
    for( nThreads = MAXTHREADS; nThreads > 0; nThreads = nThreads>>1 )
    {
        for( size_t i = 0; i < nThreads; ++i )
            TEST( threads[i].start( ));

        co::base::Clock clock;
        lock->unset();

        for( size_t i=0; i<nThreads; i++ )
            TEST( threads[i].join( ));

        TEST( !lock->isSet( ));
        lock->set();

        const float time = clock.getTimef();
        std::cout << 3 /*set, test, unset*/ * NOPS * nThreads / time
                  << " spin lock ops/ms (" << nThreads << " threads)"
                  << std::endl;
    }

    delete lock;
    return EXIT_SUCCESS;
}

