
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
            RefPtr<Connection> connection = Connection::create( TYPE_TCPIP );
            RefPtr<ConnectionDescription> connDesc = new ConnectionDescription;

            connDesc->TCPIP.port = _master ? 4242 : 4243;
            TEST( connection->listen( connDesc ))
            
            RefPtr<Node> node = new Node();
            TEST( node->listen( connection ));

            if( _master )
            {
                Session session;
                TEST( node->mapSession( node, &session, "foo" ));
                
                Barrier barrier(2);
                session.registerMobject( &barrier, node.get( ));
                TEST( barrier.getID() != EQ_INVALID_ID );
                
                barrierID = barrier.getID();

                cerr << "Master enter" << endl;
                barrier.enter();
                cerr << "Master left" << endl;

                //session.deregisterMobject( &barrier );
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
                
                Barrier *barrier = (Barrier*)session.getMobject( barrierID );
                TEST( barrier );

                cerr << "Slave enter" << endl;
                barrier->enter();
                cerr << "Slave left" << endl;

                //session.deregisterMobject( barrier );
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

