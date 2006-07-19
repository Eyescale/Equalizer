
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <test.h>

#include <eq/net/barrier.h>
#include <eq/net/connection.h>
#include <eq/net/init.h>
#include <eq/net/node.h>
#include <eq/net/session.h>

#include <iostream>

using namespace eqBase;
using namespace eqNet;
using namespace std;

uint32_t barrierID = EQ_INVALID_ID;

class NodeThread : public Thread
{
public:
    NodeThread( const bool master ) : _master(master) {}

    virtual ssize_t run()
        {
            RefPtr<Connection>            connection = 
                Connection::create( Connection::TYPE_TCPIP );
            RefPtr<ConnectionDescription> connDesc   = 
                connection->getDescription();

            connDesc->TCPIP.port = _master ? 4242 : 4243;
            TEST( connection->listen( ))
            
            RefPtr<Node> node = new Node();
            TEST( node->listen( connection ));

            if( _master )
            {
                Session session;
                TEST( node->mapSession( node, &session, "foo" ));
                
                Barrier barrier( node, 2 );
                session.registerObject( &barrier, node );
                TEST( barrier.getID() != EQ_INVALID_ID );
                
                barrierID = barrier.getID();

                cerr << "Master enter" << endl;
                barrier.enter();
                cerr << "Master left" << endl;

                //session.deregisterObject( &barrier );
                //node->unmapSession( &session );
            }
            else
            {
                while( barrierID == EQ_INVALID_ID );

                RefPtr<Node>                  server     = new Node;
                RefPtr<ConnectionDescription> serverDesc = 
                    new ConnectionDescription;
                serverDesc->TCPIP.port = 4242;
                server->addConnectionDescription( serverDesc );

                Session session;
                TEST( node->mapSession( server, &session, "foo" ));
                
                RefPtr<eqNet::Object> object = session.getObject( barrierID);
                TEST( dynamic_cast<eqNet::Barrier*>(object.get()) );
                
                eqNet::Barrier* barrier = (eqNet::Barrier*)object.get();
                TEST( barrier );

                cerr << "Slave enter" << endl;
                barrier->enter();
                cerr << "Slave left" << endl;

                //session.deregisterObject( barrier );
                //node->unmapSession( &session );
            }

            node->stopListening();
            return EXIT_SUCCESS;
        }
            
private:
    bool _master;
};

int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    NodeThread server( true );
    NodeThread node( false );

    server.start();
    node.start();
    server.join();
    node.join();
}

