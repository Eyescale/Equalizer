// Tests pipe() throughput
// Usage: ./netperf

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
        Connection::create( CONNECTIONTYPE_PIPE );

    TEST( connection->connect( ));
    Sender sender( connection );
    TEST( sender.start( ));

    void* buffer = calloc( 1, PACKETSIZE );
    const float mBytesSec = PACKETSIZE / 1024.0f / 1024.0f * 1000.0f;

    Clock clock;
    for( unsigned i=0; i<NPACKETS; )
    {
        connection->waitForData();
        clock.reset();
        if( connection->recv( buffer, PACKETSIZE ))
        {
            EQINFO << "Recv perf: " << mBytesSec / clock.getTimef() << "MB/s"
                   << endl;
            ++i;
        }
    }

    TEST( sender.join( ));
    connection->close();
    free( buffer );
    return EXIT_SUCCESS;
}
