
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


#include <test.h>
#include <co/base/monitor.h>
#include <co/connectionSet.h>
#include <co/init.h>

#include <iostream>

#include "libs/collage/pipeConnection.h" // private header

#define MAXPACKETSIZE (32 * 1048576)

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
            co::base::Clock clock;

            for( uint64_t packetSize = MAXPACKETSIZE; packetSize >= 1048576;
                 packetSize = packetSize >> 1 )
            {
                const float mBytes    = packetSize / 1024.0f / 1024.0f;
                const float mBytesSec = mBytes * 1000.0f;

                clock.reset();
                TEST( _connection->send( buffer, packetSize ));

                std::cerr << "Send perf: " << mBytesSec / clock.getTimef()
                          << "MB/s (" << mBytes << "MB)" << std::endl;
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

    for( uint64_t packetSize = MAXPACKETSIZE; packetSize >= 1048576;
         packetSize = packetSize >> 1 )
    {
        const float mBytes    = packetSize / 1024.0f / 1024.0f;
        const float mBytesSec = mBytes * 1000.0f;

        clock.reset();
        connection->recvNB( buffer, packetSize );
        TEST( connection->recvSync( 0, 0 ));

        std::cerr << "Recv perf: " << mBytesSec / clock.getTimef() << "MB/s ("
                  << mBytes << "MB)" << std::endl;
    }

    TEST( sender.join( ));
    connection->close();
    free( buffer );
    return EXIT_SUCCESS;
}
