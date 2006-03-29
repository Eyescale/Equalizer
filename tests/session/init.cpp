
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <test.h>
#include <eq/eq.h>

#include <iostream>

using namespace eqBase;
using namespace eqNet;
using namespace std;

RefPtr<eqNet::Connection> connection;

eq::NodeFactory* eq::createNodeFactory() { return new eq::NodeFactory; }

class NodeThread : public eqBase::Thread
{
protected:
    ssize_t run()
        {
            RefPtr<Node>        node      = new Node;
            RefPtr<eqNet::Node> nodeProxy = new eqNet::Node;

            TEST( node->listen( ));

            eqNet::PipeConnection* pipeConnection = 
                (eqNet::PipeConnection*)connection.get();

            TEST( node->connect( nodeProxy, pipeConnection->getChildEnd( )));
            TEST( nodeProxy->isConnected( ));
            
            Session session;
            TEST( node->mapSession( nodeProxy, &session, "foo" ));

            TEST( node->stopListening( ));
            return EXIT_SUCCESS;
        }
};

int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    connection = Connection::create(Connection::TYPE_PIPE);
    RefPtr<ConnectionDescription> connDesc = new ConnectionDescription;

    connDesc->type = Connection::TYPE_PIPE;

    TEST( connection->connect( connDesc ));

    NodeThread thread;
    thread.start();

    RefPtr<Node> node      = new Node;
    RefPtr<Node> nodeProxy = new Node;

    TEST( node->listen( ));
    TEST( node->connect( nodeProxy, connection ));
    TEST( nodeProxy->isConnected( ));

    Session session;
    TEST( node->mapSession( node, &session, "foo" ));
    
    TEST( node->stopListening( ));
    thread.join();
}

