
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

#include <test.h>
#include <eq/base/clock.h>
#include <eq/base/lfQueue.h>
#include <eq/base/thread.h>
#include <iostream>

#define RUNTIME 1000 /*ms*/

eq::base::LFQueue< uint64_t > queue(1024);

class ReadThread : public eq::base::Thread
{
public:
    virtual ~ReadThread() {}
    virtual void run()
        {
            uint64_t nOps = 0;
            uint64_t nEmpty = 0;
            uint64_t item = -1;

            eq::base::Clock clock;
            while( clock.getTime64() < RUNTIME )
            {
                if( queue.getFront( item ))
                {
                    TEST( item == nOps );
                    uint64_t item2 = -1;
                    TEST( queue.pop( item2 ));
                    TEST( item2 == item );
                    ++nOps;
                }
                TEST( item + 1 == nOps );
                ++nEmpty;
            }
            const float time = clock.getTimef();
            EQINFO << 2*nOps/time << " reads/ms, " << nEmpty/time << " empty/ms"
                   << std::endl;
        }
};

int main( int argc, char **argv )
{
    ReadThread reader;
    uint64_t nOps = 0;
    uint64_t nEmpty = 0;
    
    TEST( reader.start( ));

    eq::base::Clock clock;
    while( clock.getTime64() < RUNTIME )
    {
        while( queue.push( nOps ))
            ++nOps;
        ++nEmpty;
    }
    const float time = clock.getTimef();

    TEST( reader.join( ));
    EQINFO << nOps/time << " writes/ms, " << nEmpty/time << " full/ms"
           << std::endl;

    return EXIT_SUCCESS;
}

