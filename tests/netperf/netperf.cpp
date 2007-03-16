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

#define PACKETSIZE (400 * 1048576)
#define NPACKETS   10

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

            Clock    clock;
            for( unsigned i=0; i<NPACKETS; ++i )
            {
                clock.reset();
                TEST( _connection->send( buffer, PACKETSIZE ));
                EQINFO << "Send perf: " << mBytesSec / clock.getTimef() 
                       << "MB/s" << endl;
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
        //connDesc->hostname = "10.1.1.40";
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

    Clock clock;
    for( unsigned i=0; i<NPACKETS; ++i )
    {
        clock.reset();
        TEST( connection->recv( buffer, PACKETSIZE ));
        EQINFO << "Recv perf: " << mBytesSec / clock.getTimef() << "MB/s"
               << endl;
    }

    TEST( sender.join( ));
    connection->close();
    free( buffer );
    return EXIT_SUCCESS;
}
