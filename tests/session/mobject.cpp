
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <test.h>

#include <eq/net/connection.h>
#include <eq/net/init.h>
#include <eq/net/pipeConnection.h>
#include <eq/net/session.h>
#include <eq/net/versionedObject.h>
#include <eq/client/nodeFactory.h>

#include <iostream>

using namespace eqBase;
using namespace std;

eq::NodeFactory* eq::createNodeFactory() { return new eq::NodeFactory; }

class TestMobject : public eqNet::Mobject
{
public:
    TestMobject() : Mobject( eqNet::MOBJECT_CUSTOM ), value(42) {}

protected:
    int value;

    const void* getInstanceData( uint64_t* size )
        {
            *size = sizeof( value );
            return &value;
        }
};

class Session : public eqNet::Session
{
    eqNet::Mobject* instanciateMobject( const uint32_t type, const void* data, 
                                        const uint64_t dataSize )
        {
            if( type == eqNet::MOBJECT_CUSTOM )
            {
                TEST( *(int*)data == 42 );
                return new TestMobject();
            }
            return eqNet::Session::instanciateMobject( type, data, dataSize );
        }
};

RefPtr<eqNet::Connection> connection;
volatile uint32_t         testID = EQ_INVALID_ID;

class ServerThread : public eqBase::Thread
{
protected:
    ssize_t run()
        {
            RefPtr<eqNet::Node> server    = new eqNet::Node;
            RefPtr<eqNet::Node> nodeProxy = new eqNet::Node;

            TEST( server->listen( ));

            eqNet::PipeConnection* pipeConnection = 
                (eqNet::PipeConnection*)connection.get();

            TEST( server->connect( nodeProxy, pipeConnection->getChildEnd( )));
            TEST( nodeProxy->isConnected( ));
                        
            Session session;
            TEST( server->mapSession( server, &session, "foo" ));

            while( testID == EQ_INVALID_ID ); // spin for test object
            
            TestMobject* object = (TestMobject*) session.getMobject( testID );
            TEST( object );
            TEST( object->getID() == testID );

            TEST( server->stopListening( ));
            return EXIT_SUCCESS;
        }
};

int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    connection = eqNet::Connection::create(eqNet::Connection::TYPE_PIPE);
    RefPtr<eqNet::ConnectionDescription> connDesc = 
        new eqNet::ConnectionDescription;

    connDesc->type = eqNet::Connection::TYPE_PIPE;

    TEST( connection->connect( connDesc ));

    ServerThread server;
    server.start();

    RefPtr<eqNet::Node> node        = new eqNet::Node;
    RefPtr<eqNet::Node> serverProxy = new eqNet::Node;

    TEST( node->listen( ));
    TEST( node->connect( serverProxy, connection ));
    TEST( serverProxy->isConnected( ));

    Session session;
    TEST( node->mapSession( serverProxy, &session, "foo" ));
    
    TestMobject obj;
    session.registerMobject( &obj, serverProxy.get( ));

    testID = obj.getID();
    TEST( testID != EQ_INVALID_ID );

    TEST( node->stopListening( ));
    TEST( server.join( ));
}
