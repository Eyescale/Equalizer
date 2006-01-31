
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <test.h>
#include <eq/eq.h>

#include <iostream>

using namespace eqBase;
using namespace eqNet;
using namespace std;


int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    RefPtr<Connection>            connection = Connection::create(TYPE_PIPE);
    RefPtr<ConnectionDescription> connDesc   = new ConnectionDescription;

    if( !connection->connect( connDesc ))
        exit( EXIT_FAILURE );

    RefPtr<Node> node        = new Node;
    RefPtr<Node> server      = new Node;
    RefPtr<Node> nodeProxy   = new Node;
    RefPtr<Node> serverProxy = new Node;

    TEST( node->listen( ));
    TEST( server->listen( ));
    TEST( node->connect( serverProxy, connection ));
    TEST( serverProxy->isConnected( ));

    PipeConnection* pipeConnection = (PipeConnection*)connection.get();
    TEST( server->connect( nodeProxy, pipeConnection->getChildEnd( )));
    TEST( nodeProxy->isConnected( ));

    Session session;
    TEST( node->mapSession( serverProxy, &session, "foo" ));
    
    TEST( server->stopListening( ));
    TEST( node->stopListening( ));

    sleep(1);

    TEST( connection->getRefCount() == 1 );
    TEST( node->getRefCount() == 1 );
    TEST( server->getRefCount() == 1 );
    TEST( nodeProxy->getRefCount() == 1 );
    TEST( serverProxy->getRefCount() == 1 );
}

