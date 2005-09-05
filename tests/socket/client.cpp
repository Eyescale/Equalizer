
#include <eq/net/connection.h>
#include <eq/net/connectionDescription.h>
#include <eq/net/global.h>

#include <alloca.h>
#include <iostream>

using namespace eqNet;
using namespace std;

int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    Connection *connection = Connection::create( TYPE_TCPIP );

    ConnectionDescription connDesc;
    connDesc.hostname = "localhost";
    connDesc.parameters.TCPIP.port = 4242;
    connection->connect(connDesc);

    const char message[] = "buh!";
    int nChars = strlen( message ) + 1;
    const char *response = (const char*)alloca( nChars );

    connection->send( message, nChars );
    connection->recv( response, nChars );
    cerr << "Client recv: " << response << endl;
    connection->close();

    return EXIT_SUCCESS;
}
