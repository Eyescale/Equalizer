
#include <connection.h>

#include <eq/net/connectionDescription.h>

#include <alloca.h>

using namespace eqNet;

int main( int argc, char **argv )
{
    ConnectionDescription connDesc;
    connDesc.protocol      = Network::PROTO_TCPIP;
    connDesc.TCPIP.address = "localhost:4242";

    Connection *connection = Connection::create( connDesc );
    connection->connect();

    const char message[] = "buh!";
    int nChars = strlen( message ) + 1;
    const char *response = (const char*)alloca( nChars );

    connection->write( message, nChars );
    connection->read( response, nChars );
    fprintf( stderr, "%s\n", response );
    connection->close();

    return EXIT_SUCCESS;
}
