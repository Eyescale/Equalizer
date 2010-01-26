
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

#include <pthread.h>
#include <test.h>
#include <eq/base/clock.h>
#include <eq/base/mtQueue.h>
#include <eq/base/thread.h>
#include <iostream>

#define NOPS 1000000

eq::base::MTQueue< uint64_t > queue;

class ReadThread : public eq::base::Thread
{
public:
    virtual ~ReadThread() {}
    virtual void* run()
        {
            uint64_t item = -1;

            eq::base::Clock clock;
            for( size_t i = 0 ; i < NOPS; ++i )
            {
                item = queue.pop();
#ifndef NDEBUG
                TEST( item == i );
#endif
            }
            const float time = clock.getTimef();

            TEST( queue.isEmpty( ));
            EQINFO << NOPS/time << " reads/ms" << std::endl;
            return EXIT_SUCCESS;
        }
};

int main( int argc, char **argv )
{
    ReadThread reader;
    uint64_t nOps = 0;
    
    TEST( reader.start( ));

    eq::base::Clock clock;
    for( size_t i = 0 ; i < NOPS; ++i )
    {
        queue.push( i );
    }
    const float time = clock.getTimef();

    TEST( reader.join( ));
    EQINFO << NOPS/time << " writes/ms" << std::endl;
    return EXIT_SUCCESS;
}

