
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <test.h>
#include <eq/eq.h>

#include <iostream>

using namespace eqBase;
using namespace eqNet;
using namespace std;

RefPtr<eqNet::Connection> connection;

class NodeThread : public eqBase::Thread
{
protected:
    virtual void* run()
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

    connection = new PipeConnection();
    TEST( connection->connect( ));

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
    node = NULL;
    nodeProxy = NULL;
}

