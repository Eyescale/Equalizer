
/* Copyright (c) 2008-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

// Tests PipeConnection throughput
// Usage: ./pipeperf


#define EQ_TEST_RUNTIME 600 // seconds, needed for NighlyMemoryCheck
#include <test.h>
#include <co/base/clock.h>
#include <co/base/monitor.h>
#include <co/connectionSet.h>
#include <co/init.h>

#include <iostream>

#include "libs/collage/pipeConnection.h" // private header

#define MAXPACKETSIZE EQ_64MB

static co::base::Monitor< unsigned > _nextStage;

class Sender : public co::base::Thread
{
public:
    Sender( co::ConnectionPtr connection )
            : co::base::Thread()
            , _connection( connection )
        {}
    virtual ~Sender(){}

protected:
    virtual void run()
        {
            void* buffer = calloc( 1, MAXPACKETSIZE );

            unsigned stage = 2;
            for( uint64_t packetSize = MAXPACKETSIZE; packetSize > 0;
                 packetSize = packetSize >> 1 )
            {
                uint32_t nPackets = 10 * MAXPACKETSIZE / packetSize;
                if( nPackets > 10000 )
                    nPackets = 10000;
                uint32_t i = nPackets + 1;

                while( --i )
                {
                    TEST( _connection->send( buffer, packetSize ));
                }
                ++_nextStage;
                _nextStage.waitGE( stage );
                stage += 2;
            }
            free( buffer );
        }

private:
    co::ConnectionPtr _connection;
};

int main( int argc, char **argv )
{
    co::init( argc, argv );

    co::PipeConnectionPtr connection = new co::PipeConnection;

    TEST( connection->connect( ));
    Sender sender( connection->acceptSync( ));
    TEST( sender.start( ));

    void* buffer = calloc( 1, MAXPACKETSIZE );
    co::base::Clock clock;

    unsigned stage = 2;
    for( uint64_t packetSize = MAXPACKETSIZE; packetSize > 0;
         packetSize = packetSize >> 1 )
    {
        const float mBytes    = packetSize / 1024.0f / 1024.0f;
        const float mBytesSec = mBytes * 1000.0f;
        uint32_t nPackets = 10 * MAXPACKETSIZE / packetSize;
        if( nPackets > 10000 )
            nPackets = 10000;
        uint32_t i = nPackets + 1;

        clock.reset();
        while( --i )
        {
            connection->recvNB( buffer, packetSize );
            TEST( connection->recvSync( 0, 0 ));
        }
        const float time = clock.getTimef();
        if( mBytes > 0.2f )
            std::cerr << nPackets * mBytesSec / time << "MB/s, "
                      << nPackets / time << "p/ms (" << mBytes << "MB)"
                      << std::endl;
        else
            std::cerr << nPackets * mBytesSec / time << "MB/s, "
                      << nPackets / time << "p/ms (" << packetSize << "B)"
                      << std::endl;

        ++_nextStage;
        _nextStage.waitGE( stage );
        stage += 2;
    }

    TEST( sender.join( ));
    connection->close();
    free( buffer );
    return EXIT_SUCCESS;
}
