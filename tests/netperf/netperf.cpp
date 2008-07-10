// Tests network throughput
// Usage:
//   Server: ./netperf -s (<connectionDescription>)
//   Client: ./netperf -c <connectionDescription>
//     connectionDescription: hostname(:port)(:(SDP|TCPIP))

#include <test.h>
#include <eq/base/monitor.h>
#include <eq/net/connection.h>
#include <eq/net/connectionDescription.h>
#include <eq/net/connectionSet.h>
#include <eq/net/init.h>

#include <iostream>

using namespace eq::net;
using eq::base::Clock;
using namespace std;

#define PACKETSIZE (1048576)
#define NPACKETS   10000

int main( int argc, char **argv )
{
    if( argc != 2 && argc != 3 )
    {
        cout << "Usage: " << argv[0] << " (-s|-c) (connectionDescription)" 
             << endl;
        exit( EXIT_FAILURE );
    }

    eq::net::init( argc, argv );

    ConnectionPtr connection = Connection::create( CONNECTIONTYPE_TCPIP );
    ConnectionDescriptionPtr connDesc = connection->getDescription();
    connDesc->TCPIP.port = 4242;

    if( argc == 3 )
    {
        string desc = argv[2];
        connDesc->fromString( desc );
    }

    void*         buffer    = calloc( 1, PACKETSIZE );
    const float   mBytesSec = PACKETSIZE / 1024.0f / 1024.0f * 1000.0f;
    Clock         clock;

    if( argv[1] == string( "-c" )) // client
    {
        TEST( connection->connect( ));

        for( unsigned i=0; i<NPACKETS; ++i )
        {
            clock.reset();
            TEST( connection->send( buffer, PACKETSIZE ));
            const float time = clock.getTimef();
            cout << i << " Send perf: " << mBytesSec / time << "MB/s (" 
                 << time << "ms)" << endl;
        }
    }
    else
    {
        TEST( connection->listen( ));

        ConnectionSet connectionSet;
        connectionSet.addConnection( connection );

        const ConnectionSet::Event event = connectionSet.select();
        TEST( event == ConnectionSet::EVENT_CONNECT );

        ConnectionPtr resultConn = connectionSet.getConnection();
        ConnectionPtr newConn    = resultConn->accept();
        TEST( resultConn == connection );
        TEST( newConn.isValid( ));

        connectionSet.addConnection( newConn );

        // Until all client have disconnected...
        while( connectionSet.size() > 1 )
        {
            switch( connectionSet.select( )) // ...get next request
            {
                case ConnectionSet::EVENT_CONNECT: // new client
                    resultConn = connectionSet.getConnection();
                    TEST( resultConn == connection );
                    newConn    = resultConn->accept();
                    TEST( newConn.isValid( ));

                    connectionSet.addConnection( newConn );
                    break;

                case ConnectionSet::EVENT_DATA:  // new data
                    resultConn = connectionSet.getConnection();

                    clock.reset();
                    if( resultConn->recv( buffer, PACKETSIZE ))
                    {
                        const float time = clock.getTimef();
                        eq::net::ConnectionDescriptionPtr desc = 
                            resultConn->getDescription();

                        EQWARN << " Recv perf: " << mBytesSec / time << "MB/s ("
                            << time << "ms) from " << desc->getHostname() << ":"
                            << desc->TCPIP.port << endl;
                }
                    else // Connection dead?!
                        connectionSet.removeConnection( resultConn );
                break;

                case ConnectionSet::EVENT_DISCONNECT:
                case ConnectionSet::EVENT_INVALID_HANDLE:  // client done
                    resultConn = connectionSet.getConnection();
                    connectionSet.removeConnection( resultConn );
                    break;

                case ConnectionSet::EVENT_INTERRUPT:
                    break;

                default:
                    TESTINFO( false, "Not reachable" );
            }
        }
    }

    free( buffer );

    TESTINFO( connection->getRefCount() == 1, connection->getRefCount( ));
    connection->close();
    return EXIT_SUCCESS;
}
