
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

#pragma warning(push)
// attributes not present on previous declaration.
// for details: http://www.softwareverify.com/software-verify-blog/?p=671
#pragma warning(disable: 4985)                     
#include <limits>
#pragma warning(pop)

#define MAXTHREADS 128
#define TIME       500  // ms

co::base::Clock _clock;
bool _running = false;

template< class T, uint32_t hold > class WriteThread : public co::base::Thread
{
public:
    WriteThread() : ops( 0 ) {}

    T* lock;
    size_t ops;
    double sTime;

    virtual void run()
        {
            ops = 0;
            sTime = 0.;
            while( CO_LIKELY( _running ))
            {
                lock->set();
                TEST( lock->isSetWrite( ));
                if( hold > 0 ) // static, optimized out
                {
                    const double begin = _clock.getTimed();
                    co::base::sleep( hold );
                    sTime += _clock.getTimef() - begin;
                }
                lock->unset();

                ++ops;
            }
        }
};

template< class T, uint32_t hold > class ReadThread : public co::base::Thread
{
public:
    ReadThread() : ops( 0 ) {}

    T* lock;
    size_t ops;
    double sTime;

    virtual void run()
        {
            ops = 0;
            sTime = 0.;
            while( CO_LIKELY( _running ))
            {
                lock->setRead();
                TEST( lock->isSetRead( ));
                if( hold > 0 ) // static, optimized out
                {
                    const double begin = _clock.getTimed();
                    co::base::sleep( hold );
                    sTime += _clock.getTimef() - begin;
                }
                lock->unsetRead();

                ++ops;
            }
        }
};

template< class T, uint32_t hold > void _test()
{
    T* lock = new T;
    lock->set();

    WriteThread< T, hold > writers[MAXTHREADS];
    ReadThread< T, hold > readers[MAXTHREADS];

    std::cout << "               Class, write ops/ms,  read ops/ms, w threads, "
              << "r threads" << std::endl;
    for( size_t nWrite = 0; nWrite <= MAXTHREADS;
         nWrite = (nWrite == 0) ? 1 : nWrite<<1 )
    {
        for( size_t nThreads = 1; nThreads <= MAXTHREADS; nThreads=nThreads<<1 )
        {
            if( nThreads < nWrite )
                continue;

            const size_t nRead = nThreads - nWrite;
            _running = true;
            for( size_t i = 0; i < nWrite; ++i )
            {
                writers[i].lock = lock;
                TEST( writers[i].start( ));
            }
            for( size_t i = 0; i < nRead; ++i )
            {
                readers[i].lock = lock;
                TESTINFO( readers[i].start(), i );
            }
            co::base::sleep( 10 ); // let threads initialize

            _clock.reset();
            lock->unset();
            co::base::sleep( TIME ); // let threads run
            _running = false;

            for( size_t i=0; i < nWrite; ++i )
                TEST( writers[i].join( ));
            for( size_t i=0; i < nRead; ++i )
                TEST( readers[i].join( ));
            const double time = _clock.getTimed();

            TEST( !lock->isSet( ));
            lock->set();

            size_t nWriteOps = 0;
            double wTime = time * double( nWrite );
            for( size_t i=0; i<nWrite; ++i )
            {
                nWriteOps += writers[i].ops;
                wTime -= writers[i].sTime;
            }
            if( nWrite > 0 )
                wTime /= double( nWrite );
            if( wTime == 0.f )
                wTime = std::numeric_limits< double >::epsilon();


            size_t nReadOps = 0;
            double rTime = time * double( nRead );
            for( size_t i=0; i<nRead; ++i )
            {
                nReadOps += readers[i].ops;
                rTime -= readers[i].sTime;
            }
            if( nRead > 0 )
                rTime /= double( nRead );
            if( rTime == 0.f )
                rTime = std::numeric_limits< double >::epsilon();

            std::cout << std::setw(20)<< co::base::className( lock ) << ", "
                      << std::setw(12) << 3 * nWriteOps / wTime << ", "
                      << std::setw(12) << 3 * nReadOps / rTime << ", " 
                      << std::setw(9) << nWrite << ", " << std::setw(9) << nRead
                      << std::endl;
        }
    }

    delete lock;
}

int main( int argc, char **argv )
{
    TEST( co::base::init( argc, argv ));
//    co::base::sleep( 5000 );

    std::cerr << "0 ms in locked region" << std::endl;
    _test< co::base::SpinLock, 0 >();
#if 0 // time collection not yet correct
    std::cerr << "1 ms in locked region" << std::endl;
    _test< co::base::SpinLock, 1 >();
    std::cerr << "2 ms in locked region" << std::endl;
    _test< co::base::SpinLock, 2 >();
    std::cerr << "4 ms in locked region" << std::endl;
    _test< co::base::SpinLock, 4 >();
#endif

    TEST( co::base::exit( ));
    return EXIT_SUCCESS;
}
