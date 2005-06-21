
#include <connection.h>

#include <eq/net/connectionDescription.h>

#include <alloca.h>

using namespace eqNet;
using namespace eqNetInternal;

int main( int argc, char **argv )
{
    Connection *connection = Connection::create( Network::PROTO_TCPIP );

    ConnectionDescription connDesc;
    connDesc.TCPIP.address = "localhost:4242";
    connection->connect(connDesc);

    const char message[] = "buh!";
    int nChars = strlen( message ) + 1;
    const char *response = (const char*)alloca( nChars );

    connection->send( message, nChars );
    connection->recv( response, nChars );
    fprintf( stderr, "%s\n", response );
    connection->close();

    return EXIT_SUCCESS;
}
