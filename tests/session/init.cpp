
#include <eq/net/node.h>
#include <eq/net/session.h>

#include <eq/net/global.h>
#include <iostream>

using namespace eqNet;
using namespace std;


int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    Node server;
    Connection* connection = Connection::create( TYPE_PIPE );
    ConnectionDescription connDesc;
    connDesc.parameters.PIPE.entryFunc = "eqNet_Node_runServer";

    connection->connect(connDesc);

    sleep(1);
}

