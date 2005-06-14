
#include <connection.h>

using namespace eqNet;

int main( int argc, char **argv )
{
    ConnectionDescription connDesc;
    connDesc.protocol      = Network::PROTO_TCPIP;
    connDesc.TCPIP.address = "localhost:4242";

    Connection *connection = Connection::create(connDesc);

    connection->listen();
    Connection *client = connection->accept();
    fprintf( stderr, "Server accepted connection\n" );
    connection->close();

    char c;
    while( client->read( &c, 1 ))
    {
        fprintf( stderr, "Server recv: '%c'\n", c );
        client->write( &c, 1 );
    }

    return EXIT_SUCCESS;
}
