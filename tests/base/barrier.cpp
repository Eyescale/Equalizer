
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#include <eq/base/barrier.h>
#include <eq/base/thread.h>
#include <iostream>

using namespace eq::base;
using namespace std;

#define MAXTHREADS 256

volatile size_t nThreads;
Barrier*        barrier;

class Test : public Thread
{
public:
    virtual void* run()
        {
            const size_t num = barrier->enter( nThreads );
            TEST( num < nThreads );
            return EXIT_SUCCESS;
        }
};

int main( int argc, char **argv )
{
    barrier = new Barrier;
    Test threads[MAXTHREADS];

    for( nThreads = MAXTHREADS; nThreads>1; nThreads = nThreads>>1 )
    {
        for( size_t i=0; i<nThreads; i++ )
            threads[i].start();

        for( size_t i=0; i<nThreads; i++ )
        {
            if( !threads[i].join( ))
            {
                cerr << "Could not join thread " << i << endl;
                exit(EXIT_FAILURE);
            }
        }
    }

    delete barrier;
    return EXIT_SUCCESS;
}

