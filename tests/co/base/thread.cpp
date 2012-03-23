
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

#include <test.h>
#include <lunchbox/clock.h>
#include <lunchbox/sleep.h>
#include <lunchbox/thread.h>
#include <iostream>

#define NTHREADS 256

class LoadThread : public lunchbox::Thread
{
public:
    virtual ~LoadThread() {}
    virtual void run() {}
};

class InitThread : public LoadThread
{
public:
    InitThread() : initLeft( false ) {}
    virtual ~InitThread() {}

    virtual bool init()
        {
            lunchbox::sleep( 10 );
            initLeft = true;
            return true;
        }
    
    bool initLeft;
};

class FailThread : public InitThread
{
public:
    virtual ~FailThread() {}

    virtual bool init()
        {
            return false;
        }
};

int main( int argc, char **argv )
{
    LoadThread loadThreads[NTHREADS];
    lunchbox::Clock clock;

    for( size_t i=0; i<NTHREADS; ++i )
        TEST( loadThreads[i].start( ));

    for( size_t i=0; i<NTHREADS; ++i )
        TEST( loadThreads[i].join( ));
    const float time = clock.getTimef();
    std::cout << "Spawned and joined " << NTHREADS << " loadThreads in "
              << time << " ms (" << (NTHREADS/time) << " threads/ms)" 
              << std::endl;

    for( size_t i=0; i<NTHREADS; ++i )
        TEST( loadThreads[i].isStopped( ));
    
    InitThread initThreads[NTHREADS];

    clock.reset();
    for( size_t i=0; i<NTHREADS; ++i )
    {
        TEST( initThreads[i].start( ));
        TEST( initThreads[i].initLeft == true );
    }
    TESTINFO( clock.getTimef() > NTHREADS * 10, clock.getTimef( ));

    for( size_t i=0; i<NTHREADS; ++i )
        TEST( initThreads[i].join( ));

    FailThread failThread;
    TEST( !failThread.start( ));
    TEST( !failThread.isRunning( ));
    TEST( failThread.isStopped( ));

    return EXIT_SUCCESS;
}

