
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

// Tests pipe() throughput
// Usage: ./pipeperf


#include <test.h>
#include <eq/base/monitor.h>
#include <eq/net/connection.h>
#include <eq/net/connectionDescription.h>
#include <eq/net/connectionSet.h>
#include <eq/net/init.h>

#include <iostream>

using namespace eq::net;
using namespace eq::base;
using namespace std;

#define PACKETSIZE (100 * 1048576)
#define NPACKETS   10

class Sender : public Thread
{
public:
    Sender( ConnectionPtr connection )
            : Thread(),
              _connection( connection )
        {}
    virtual ~Sender(){}

protected:
    virtual void* run()
        {
            void* buffer = calloc( 1, PACKETSIZE );
            const float mBytesSec = PACKETSIZE / 1024.0f / 1024.0f * 1000.0f;

            Clock    clock;
            for( unsigned i=0; i<NPACKETS; ++i )
            {
                clock.reset();
                TEST( _connection->send( buffer, PACKETSIZE ));
                cout << "Send perf: " << mBytesSec / clock.getTimef() << "MB/s"
                     << endl;
            }

            free( buffer );
            return EXIT_SUCCESS;
        }

private:
    ConnectionPtr _connection;
};

int main( int argc, char **argv )
{
    eq::net::init( argc, argv );

    ConnectionPtr connection = new PipeConnection;

    TEST( connection->connect( ));
    Sender sender( connection );
    TEST( sender.start( ));

    void* buffer = calloc( 1, PACKETSIZE );
    const float mBytesSec = PACKETSIZE / 1024.0f / 1024.0f * 1000.0f;

    Clock clock;
    for( unsigned i=0; i<NPACKETS; )
    {
        clock.reset();
        connection->recvNB( buffer, PACKETSIZE );
        if( connection->recvSync( 0, 0 ))
        {
            cout << "Recv perf: " << mBytesSec / clock.getTimef() << "MB/s"
                 << endl;
            ++i;
        }
    }

    TEST( sender.join( ));
    connection->close();
    free( buffer );
    return EXIT_SUCCESS;
}
