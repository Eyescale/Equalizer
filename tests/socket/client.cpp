
#include <connection.h>

#include <eq/net/connectionDescription.h>

#include <alloca.h>
#include <iostream>

using namespace eqNet;
using namespace eqNet::priv;
using namespace std;

int main( int argc, char **argv )
{
    Connection *connection = Connection::create( PROTO_TCPIP );

    ConnectionDescription connDesc;
    connDesc.parameters.TCPIP.address = "localhost:4242";
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
