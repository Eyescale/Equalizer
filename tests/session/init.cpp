
#include <test.h>
#include <eq/net/node.h>
#include <eq/net/pipeConnection.h>
#include <eq/net/session.h>

#include <iostream>

using namespace eqBase;
using namespace eqNet;
using namespace std;


int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    eqBase::RefPtr<Connection> connection =
        (PipeConnection*)Connection::create(TYPE_PIPE);

    RefPtr<ConnectionDescription> connDesc = new ConnectionDescription;
    if( !connection->connect( connDesc ))
        exit( EXIT_FAILURE );

    Node node;
    Node server;
    Node nodeProxy;
    Node serverProxy;

    TEST( node.listen( ));
    TEST( server.listen( ));
    TEST( node.connect( &serverProxy, connection ));

    PipeConnection* pipeConnection = (PipeConnection*)connection.get();
    TEST( server.connect( &nodeProxy, pipeConnection->getChildEnd( )));

    Session session;
    TEST( node.mapSession( &serverProxy, &session, "foo" ));
    
    TEST( server.stopListening( ));
    TEST( node.stopListening( ));

    cerr << connection->getRefCount() << endl;
    TEST( connection->getRefCount() == 1 );

    sleep(1);
}

