
#include <test.h>

#include <eq/net/connection.h>
#include <eq/net/connectionDescription.h>
#include <eq/net/init.h>

#include <iostream>

using namespace eqNet;
using namespace eqBase;
using namespace std;

int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    RefPtr<Connection>            connection = 
        Connection::create( CONNECTIONTYPE_TCPIP );
    RefPtr<ConnectionDescription> connDesc   = connection->getDescription();

    connDesc->hostname = "localhost";
    connDesc->TCPIP.port = 0;
    TEST( connection->listen( ));

    RefPtr<Connection> client = connection->accept();
    cerr << "Server accepted connection" << endl;
    connection->close();

    char c;
    while( client->recv( &c, 1 ))
    {
        cerr << "Server recv: " << c << endl;
        TEST( client->send( &c, 1 ) == 1 );
    }

    return EXIT_SUCCESS;
}
