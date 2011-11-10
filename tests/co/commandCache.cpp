
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

#define EQ_TEST_RUNTIME 300
#include <test.h>

#include <co/command.h>
#include <co/commandCache.h>
#include <co/commandQueue.h>
#include <co/dispatcher.h>
#include <co/init.h>
#include <co/base/clock.h>

// Tests the functionality of the network packet cache

#define N_READER 13
#define RUNTIME 5000

struct Packet : public co::Packet
{
    Packet()
        {
            command        = 0;
            size           = sizeof( Packet ); 
        }
};


class Reader : public co::Dispatcher, public co::base::Thread
{
public:
    bool _cmd( co::Command& command ) { return true; }
    bool _cmdStop( co::Command& command ) { _running = false; return true; }

    Reader() : _running( false )
        {
            registerCommand( 0u,
                             co::CommandFunc< Reader >( this, &Reader::_cmd ),
                             &_queue );
            registerCommand( 1u,
                             co::CommandFunc<Reader>( this, &Reader::_cmdStop ),
                             &_queue );
        }
    virtual ~Reader(){}

protected:

    virtual void run()
        {
            _running = true;
            while( _running )
            {
                co::Command& command = *_queue.pop();
                TEST( command( ));
                command.release();
                yield(); // let writer run ahead a bit
            }
        }

private:
    co::CommandQueue _queue;
    bool _running;
};

int main( int argc, char **argv )
{
    co::init( argc, argv );
    {
        Reader readers[ N_READER ];
        for( size_t i = 0; i < N_READER; ++i )
            readers[i].start();

        co::CommandCache cache;
        co::LocalNodePtr node = new co::LocalNode;
        size_t nOps = 0;

        co::base::Clock clock;
        while( clock.getTime64() < RUNTIME )
        {
            co::Command& command = cache.alloc( node, node, sizeof( Packet ));
            Packet* packet = command.getModifiable< Packet >();
            *packet = Packet();

            readers[0].dispatchCommand( command );

            for( size_t i = 1; i < N_READER; ++i )
            {
                co::Command& clone = cache.clone( command );
                readers[i].dispatchCommand( clone );
            }
            ++nOps;
        }

        const uint64_t wTime = clock.getTime64();

        for( size_t i = 0; i < N_READER; ++i )
        {
            co::Command& command = cache.alloc( node, node, sizeof( Packet ));
            Packet* packet = command.getModifiable< Packet >();
            *packet = Packet();
            packet->command = 1;

            readers[i].dispatchCommand( command );
            readers[i].join();
        }

        const uint64_t rTime = clock.getTime64();
        std::cout << nOps / wTime << " write, " << N_READER * nOps / rTime
                  << " read ops/ms" << std::endl;
    }

    TEST( co::exit( ));
    return EXIT_SUCCESS;
}
