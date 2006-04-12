
#include <test.h>

#include <eq/net/connection.h>
#include <eq/net/connectionDescription.h>
#include <eq/net/init.h>
#include <eq/client/nodeFactory.h>

#include <iostream>

using namespace eqNet;
using namespace eqBase;
using namespace std;

eq::NodeFactory* eq::createNodeFactory() { return new eq::NodeFactory; }

int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    RefPtr<Connection> connection = Connection::create(Connection::TYPE_TCPIP);
    RefPtr<ConnectionDescription> connDesc   = new ConnectionDescription;

    connDesc->hostname = "localhost";
    connDesc->TCPIP.port = 0;
    TEST( connection->listen( connDesc ));

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
