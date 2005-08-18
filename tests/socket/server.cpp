
#include <connection.h>
#include <connectionDescription.h>

#include <eq/net/global.h>
#include <iostream>

using namespace eqNet;
using namespace std;

int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    Connection *connection = Connection::create(TYPE_TCPIP);

    ConnectionDescription connDesc;
    sprintf( connDesc.hostname, "localhost" );
    connDesc.parameters.TCPIP.port = 4242;
    connection->listen(connDesc);

    Connection *client = connection->accept();
    cerr << "Server accepted connection" << endl;
    connection->close();

    char c;
    while( client->recv( &c, 1 ))
    {
        cerr << "Server recv: " << c << endl;
        client->send( &c, 1 );
    }

    return EXIT_SUCCESS;
}
