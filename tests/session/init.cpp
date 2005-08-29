
#include <test.h>
#include <eq/net/node.h>
#include <eq/net/pipeConnection.h>
#include <eq/net/session.h>

#include <iostream>

using namespace eqNet;
using namespace std;


int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    PipeConnection* connection = (PipeConnection*)Connection::create(TYPE_PIPE);
    ConnectionDescription connDesc;
    if( !connection->connect( connDesc ))
        exit( EXIT_FAILURE );

    Node node;
    Node server;
    Node nodeProxy;
    Node serverProxy;

    TEST( node.listen( ));
    TEST( server.listen( ));
    TEST( node.connect( &serverProxy, connection ));
    TEST( server.connect( &nodeProxy, connection->getChildEnd( )));

    Session session;
    TEST( node.mapSession( &serverProxy, &session, "foo" ));
    
    TEST( server.stop( ));
    sleep(1);
}

