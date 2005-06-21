
#include <connection.h>
#include <connectionDescription.h>

#include <alloca.h>
#include <iostream>

using namespace eqNet;
using namespace eqNet::priv;
using namespace std;

extern "C" int testPipeServer( Connection* connection )
{
    cerr << "Server up" << endl;
    char c;
    while( connection->recv( &c, 1 ))
    {
        cerr << "Server recv: " << c << endl;
        connection->send( &c, 1 );
    }

    return EXIT_SUCCESS;
}

int main( int argc, char **argv )
{
    Connection *connection = Connection::create(PROTO_PIPE);

    ConnectionDescription connDesc;
    connDesc.PIPE.entryFunc = "testPipeServer";

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
