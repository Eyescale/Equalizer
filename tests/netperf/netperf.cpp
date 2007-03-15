// Tests network throughput
// Usage:
//   Server: ./netperf
//   Client: ./netperf <serverName>

#include <test.h>
#include <eq/net/connection.h>
#include <eq/net/connectionDescription.h>
#include <eq/net/connectionSet.h>
#include <eq/net/init.h>

#include <iostream>

using namespace eqNet;
using namespace eqBase;
using namespace std;

#define PACKETSIZE (16 * 1048576)

class Sender : public Thread
{
public:
    Sender( RefPtr<Connection> connection )
            : Thread(), 
              _connection( connection )
        {}
    virtual ~Sender(){}

protected:
    virtual void* run()
        {
            void* buffer = calloc( 1, PACKETSIZE );
            const float mBytesSec = PACKETSIZE / 1024.0f / 1024.0f * 1000.0f;

            Clock clock;
            while( _connection->send( buffer, PACKETSIZE ))
            {
                EQINFO << "Send perf: " << mBytesSec / clock.getTimef() 
                       << "MB/s" << endl;
                clock.reset();
            }

            free( buffer );
            return EXIT_SUCCESS;
        }

private:
    RefPtr<Connection> _connection;
};

int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    RefPtr<Connection>            connection = 
        Connection::create( CONNECTIONTYPE_TCPIP );
    RefPtr<ConnectionDescription> connDesc   = connection->getDescription();
    connDesc->TCPIP.port = 4242;

    if( argc == 2 )
    {
        connDesc->hostname = argv[1];
        TEST( connection->connect( ));
    }
    else
    {
        //connDesc->hostname = "localhost";
        TEST( connection->listen( ));

        ConnectionSet set;
        set.addConnection( connection );
        set.select();

        RefPtr<Connection> client = connection->accept();
        EQINFO << "accepted connection" << endl;
        connection->close();
        connection = client;
    }

    Sender sender( connection );
    TEST( sender.start( ));

    void* buffer = calloc( 1, PACKETSIZE );
    const float mBytesSec = PACKETSIZE / 1024.0f / 1024.0f * 1000.0f;

    unsigned nLoops = 10;

    Clock clock;
    while( nLoops-- )
    {
        TEST( connection->recv( buffer, PACKETSIZE ))
        {
            EQINFO << "Recv perf: " << mBytesSec / clock.getTimef() << "MB/s"
                   << endl;
                clock.reset();
        }
    }

    connection->close();
    TEST( sender.join( ));
    free( buffer );
    return EXIT_SUCCESS;
}
