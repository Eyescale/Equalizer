
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch>
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

#define NOPS       1000
#define TIME       100

co::base::Clock _clock;

template< class T > class WriteThread : public co::base::Thread
{
public:
    WriteThread() : nLoops( 0 ) {}

    T* lock;
    size_t nLoops;

    virtual void run()
        {
            while( _clock.getTime64() < TIME )
            {
                for( unsigned i = 0; i < NOPS; ++i )
                {
                    lock->set();
                    TEST( lock->isSetWrite( ));
                    lock->unset();
                }
                ++nLoops;
            }
        }
};

template< class T > class ReadThread : public co::base::Thread
{
public:
    ReadThread() : nLoops( 0 ) {}

    T* lock;
    size_t nLoops;

    virtual void run()
        {
            while( _clock.getTime64() < TIME )
            {
                for( unsigned i = 0; i < NOPS; ++i )
                {
                    lock->setRead();
                    TEST( lock->isSetRead( ));
                    lock->unsetRead();
                }
                ++nLoops;
            }
        }
};

template< class T > void _test()
{
    T* lock = new T;
    lock->set();

    WriteThread< T > writers[MAXTHREADS];
    ReadThread< T > readers[MAXTHREADS];

    std::cout << "               Class, write ops/ms,  read ops/ms, w threads, r threads"
              << std::endl;
    for( size_t nWrite = 1; nWrite <= MAXTHREADS; nWrite = nWrite<<1 )
    {
        for( size_t nThreads = 1; nThreads <= MAXTHREADS; nThreads=nThreads<<1 )
        {
            if( nThreads < nWrite )
                continue;

            const size_t nRead = nThreads - nWrite;
            _clock.reset();
            for( size_t i = 0; i < nWrite; ++i )
            {
                writers[i].lock = lock;
                writers[i].nLoops = 0;
                TEST( writers[i].start( ));
            }
            for( size_t i = 0; i < nRead; ++i )
            {
                readers[i].lock = lock;
                readers[i].nLoops = 0;
                TESTINFO( readers[i].start(), i );
            }
            co::base::sleep( 100 ); // let threads initialize

            _clock.reset();
            lock->unset();

            for( size_t i=0; i < nWrite; ++i )
                TEST( writers[i].join( ));
            for( size_t i=0; i < nRead; ++i )
                TEST( readers[i].join( ));
            const float time = _clock.getTimef();

            TEST( !lock->isSet( ));
            lock->set();

            size_t nWriteOps = 0;
            for( size_t i=0; i<nWrite; ++i )
                nWriteOps += writers[i].nLoops;

            size_t nReadOps = 0;
            for( size_t i=0; i<nRead; ++i )
                nReadOps += readers[i].nLoops;

            std::cout << std::setw(20)<< co::base::className( lock ) << ", "
                      << std::setw(12) << 3 * NOPS * nWriteOps * nWrite / time
                      << ", " << std::setw(12)
                      << 3 * NOPS * nReadOps * nRead / time << ", " 
                      << std::setw(9) << nWrite << ", " << std::setw(9)
                      << nRead << std::endl;
        }
        std::cout << std::endl;
    }

    delete lock;
}

int main( int argc, char **argv )
{
    TEST( co::base::init( argc, argv ));
    _test< co::base::SpinLock >();
    TEST( co::base::exit( ));
    return EXIT_SUCCESS;
}
