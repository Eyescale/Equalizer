
#include <test.h>
#include <eq/net/connection.h>
#include <eq/net/connectionDescription.h>
#include <eq/net/init.h>
#include <eq/nodeFactory.h>

#include <alloca.h>
#include <iostream>

using namespace eqNet;
using namespace eqBase;
using namespace std;

eq::NodeFactory* eq::createNodeFactory() { return new eq::NodeFactory; }

int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    RefPtr<Connection>            connection = Connection::create( TYPE_TCPIP );
    RefPtr<ConnectionDescription> connDesc   = new ConnectionDescription;

    connDesc->hostname = "localhost";
    connDesc->TCPIP.port = 4242;
    TEST( connection->connect( connDesc ));

    const char      message[] = "buh!";
    const uint64_t  nChars    = strlen( message ) + 1;
    const char     *response  = (const char*)alloca( nChars );

    TEST( connection->send( message, nChars ) == nChars );
    TEST( connection->recv( response, nChars ) == nChars );
    cerr << "Client recv: " << response << endl;
    connection->close();

    return EXIT_SUCCESS;
}
