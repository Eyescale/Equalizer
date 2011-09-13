
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com>
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
#include <co/base/clock.h>
#include <co/base/debug.h>
#include <co/base/lock.h>
#include <co/base/spinLock.h>
#include <co/base/timedLock.h>
#include <iostream>

#ifdef _MSC_VER
#  define MAXTHREADS 128
#else
#  define MAXTHREADS 256
#endif

#define NOPS       1000

volatile size_t nThreads;

template< class T > class Thread : public co::base::Thread
{
public:
    T* lock;

    virtual void run()
        {
            for( unsigned i = 0; i < NOPS; ++i )
            {
                lock->set();
#ifndef _MSC_VER
                TEST( lock->isSet( ));
#endif
                lock->unset();
            }
        }
};

template< class T > void _test()
{
    T* lock = new T;
    lock->set();

    Thread< T > threads[MAXTHREADS];
    for( nThreads = MAXTHREADS; nThreads > 0; nThreads = nThreads>>1 )
    {
        for( size_t i = 0; i < nThreads; ++i )
        {
            threads[i].lock = lock;
            TEST( threads[i].start( ));
        }

        co::base::Clock clock;
        lock->unset();

        for( size_t i=0; i<nThreads; i++ )
            TEST( threads[i].join( ));

        TEST( !lock->isSet( ));
        lock->set();

        const float time = clock.getTimef();
        std::cout << co::base::className( lock ) << ": "
                  << /*set, test, unset*/ 3 * NOPS * nThreads / time
                  << " lock ops/ms (" << nThreads << " threads)" << std::endl;
    }

    delete lock;
}

int main( int argc, char **argv )
{
    _test< co::base::Lock >();
    _test< co::base::SpinLock >();
    _test< co::base::TimedLock >();
    return EXIT_SUCCESS;
}
