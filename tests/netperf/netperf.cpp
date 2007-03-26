// Tests network throughput
// Usage:
//   Server: ./netperf
//   Client: ./netperf <serverName>

#include <test.h>
#include <eq/base/monitor.h>
#include <eq/net/connection.h>
#include <eq/net/connectionDescription.h>
#include <eq/net/connectionSet.h>
#include <eq/net/init.h>

#include <iostream>

using namespace eqNet;
using namespace eqBase;
using namespace std;

#define PACKETSIZE (100 * 1048576)
#define NPACKETS   10

int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    RefPtr<Connection>            connection = 
        Connection::create( CONNECTIONTYPE_TCPIP );
    RefPtr<ConnectionDescription> connDesc   = connection->getDescription();
    connDesc->TCPIP.port = 4242;

    void*         buffer    = calloc( 1, PACKETSIZE );
    const float   mBytesSec = PACKETSIZE / 1024.0f / 1024.0f * 1000.0f;
    Clock         clock;

    if( argc == 2 )
    {
        connDesc->hostname = argv[1];
        TEST( connection->connect( ));

        for( unsigned i=0; i<NPACKETS; )
        {
            clock.reset();
            if( connection->recv( buffer, PACKETSIZE ))
            {
                EQINFO << "Recv perf: " << mBytesSec / clock.getTimef()
                       << "MB/s" << endl;
                ++i;
            }
        }
    }
    else
    {
        //connDesc->hostname = "10.2.1.2";
        TEST( connection->listen( ));

        RefPtr<Connection> client = connection->accept();
        EQINFO << "accepted connection" << endl;
        connection->close();
        connection = client;

        for( unsigned i=0; i<NPACKETS; ++i )
        {
            clock.reset();
            TEST( connection->send( buffer, PACKETSIZE ));
            EQINFO << "Send perf: " << mBytesSec / clock.getTimef() 
                   << "MB/s" << endl;
        }
    }

    free( buffer );
    connection->close();
    return EXIT_SUCCESS;
}
