
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

#include <pthread.h>
#include <test.h>

#include <co/base/atomic.h>
#include <co/base/perThread.h>
#include <co/base/rng.h>

#define NTHREADS 1024

co::base::a_int32_t _threads;

void destructor( size_t* data ) { --_threads; }

co::base::PerThread< size_t, destructor > _tls1;
co::base::PerThread< size_t > _tls2;
co::base::PerThread< size_t, destructor > _tls3;


class Thread : public co::base::Thread
{
public:
    virtual ~Thread() {}

    virtual void run()
        {
            ++_threads;
            co::base::RNG rng;

            size_t i = rng.get< size_t >();
            size_t* data = new size_t( i );
            _tls1 = data;
            _tls2 = data;
            _tls3 = _tls1;

            TEST( _tls1.get() == data );
            TEST( *_tls1 == i );
            TEST( _tls2.get() == data );
            TEST( *_tls2 == i );
            TEST( *_tls1 == *_tls2 );
            TEST( *_tls1 == *_tls3 );
           
            _tls3 = 0;
            TEST( _tls3 == 0 );
        }
};


int main( int argc, char **argv )
{
    TEST( co::base::init( argc, argv ));

    Thread threads[NTHREADS];

    for( size_t i=0; i<NTHREADS; ++i )
        TEST( threads[i].start( ));
    for( size_t i=0; i<NTHREADS; ++i )
        TEST( threads[i].join( ));

    TEST( _threads == 0 );
    TEST( co::base::exit( ));
    return EXIT_SUCCESS;
}

