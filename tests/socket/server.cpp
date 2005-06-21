
#include <connection.h>

using namespace eqNet;
using namespace eqNetInternal;

int main( int argc, char **argv )
{
    Connection *connection = Connection::create(Network::PROTO_TCPIP);

    ConnectionDescription connDesc;
    connDesc.TCPIP.address = "localhost:4242";
    connection->listen(connDesc);

    Connection *client = connection->accept();
    fprintf( stderr, "Server accepted connection\n" );
    connection->close();

    char c;
    while( client->recv( &c, 1 ))
    {
        fprintf( stderr, "Server recv: '%c'\n", c );
        client->send( &c, 1 );
    }

    return EXIT_SUCCESS;
}
