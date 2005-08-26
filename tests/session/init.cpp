
//#include <eq/net/global.h>
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
    Node serverProxy;

    node.listen( connection );
    server.listen( connection->getChildEnd( ));
    node.connect( &serverProxy, connection );

    Session session;
    node.mapSession( &serverProxy, &session, "foo" );
    
    sleep(1);
    
    server.join();
}

