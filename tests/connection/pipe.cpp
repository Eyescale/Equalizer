
#include <test.h>

#include <eq/base/thread.h>
#include <eq/net/connection.h>
#include <eq/net/connectionDescription.h>
#include <eq/net/init.h>
#include <eq/net/pipeConnection.h>

#include <iostream>

using namespace eqBase;
using namespace eqNet;
using namespace std;

class Server : public eqBase::Thread
{
public:
    void start( RefPtr<Connection> connection )
        {
            _connection = connection;
            eqBase::Thread::start();
        }

protected:
    virtual void* run()
        {
            TEST( _connection.isValid( ));
            TEST( _connection->getState() == Connection::STATE_CONNECTED );

            char text[5];
            TEST( _connection->recv( &text, 5 ) == 5 );
            TEST( strcmp( "buh!", text ) == 0 );

            _connection->close();
            _connection = NULL;
            return EXIT_SUCCESS;
        }
private:
    RefPtr<Connection> _connection;
};

int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    RefPtr<Connection>  connection = new PipeConnection();
    TEST( connection->connect( ));

    Server server;
    server.start( connection );

    const char message[] = "buh!";
    const size_t nChars  = strlen( message ) + 1;

    TEST( connection->send( message, nChars ) == nChars );

    connection->close();
    connection = NULL;

    server.join();
    
    return EXIT_SUCCESS;
}
