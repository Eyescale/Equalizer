
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

#define EQ_TEST_RUNTIME 600 // seconds, needed for NighlyMemoryCheck
#include "test.h"

#include <co/base/atomic.h>
#include <co/base/clock.h>
#include <co/base/debug.h>
#include <co/base/init.h>
#include <co/base/lock.h>
#include <co/base/spinLock.h>
#include <co/base/timedLock.h>

#include <iostream>

#ifdef _MSC_VER
#  define MAXTHREADS 128
#else
#  define MAXTHREADS 256
#endif

#define TIME       500  // ms

co::base::Clock _clock;
bool _running = false;

template< class T > class Thread : public co::base::Thread
{
public:
    Thread() : ops( 0 ) {}

    T* lock;
    size_t ops;

    virtual void run()
        {
            ops = 0;
            while( CO_LIKELY( _running ))
            {
                lock->set();
#ifndef _MSC_VER
                TEST( lock->isSet( ));
#endif
                lock->unset();
                ++ops;
            }
        }
};

template< class T > void _test()
{
    T* lock = new T;
    lock->set();

    Thread< T > threads[MAXTHREADS];
    for( size_t nThreads = 1; nThreads <= MAXTHREADS; nThreads = nThreads<<1 )
    {
        _running = true;
        for( size_t i = 0; i < nThreads; ++i )
        {
            threads[i].lock = lock;
            TEST( threads[i].start( ));
        }
        co::base::sleep( 10 ); // let threads initialize

        _clock.reset();
        lock->unset();
        co::base::sleep( TIME ); // let threads run
        _running = false;

        for( size_t i=0; i<nThreads; ++i )
            TEST( threads[i].join( ));
        const float time = _clock.getTimef();

        TEST( !lock->isSet( ));
        lock->set();

        size_t ops = 0;
        for( size_t i=0; i<nThreads; ++i )
            ops += threads[i].ops;

        std::cout << std::setw(20) << co::base::className( lock ) << ", "
                  << std::setw(12) << /*set, test, unset*/ 3 * ops / time
                  << ", " << std::setw(3) << nThreads << std::endl;
    }

    delete lock;
}

int main( int argc, char **argv )
{
    TEST( co::base::init( argc, argv ));

    std::cout << "               Class,       ops/ms, threads" << std::endl;
    _test< co::base::SpinLock >();
    std::cout << std::endl;

    _test< co::base::Lock >();
    std::cout << std::endl;

    _test< co::base::TimedLock >();
    TEST( co::base::exit( ));

    return EXIT_SUCCESS;
}
