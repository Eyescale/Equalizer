
/* Copyright (c) 2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include <test.h>
#include <eq/eq.h>

#include <iostream>

using namespace eqBase;
using namespace std;

class TestMobject : public eqNet::Mobject
{
    void getInstanceInfo( uint32_t* typeID, std::string& data ) 
        {
            *typeID = eqNet::MOBJECT_CUSTOM;
            data = "42";
        }

};

class Session : public eqNet::Session
{
    virtual eqNet::Mobject* instanciateMobject( const uint32_t type, 
                                                const char* data )
        {
            if( type == eqNet::MOBJECT_CUSTOM )
            {
                TEST( strcmp( data, "42" ) == 0 );
                return new TestMobject();
            }
            return eqNet::Session::instanciateMobject( type, data );
        }
};

class Node : public eqNet::Node
{
    virtual eqNet::Session* createSession() 
    { return new Session(); }
};

int main( int argc, char **argv )
{
    eqNet::init( argc, argv );

    RefPtr<eqNet::Connection> connection = 
        eqNet::Connection::create(eqNet::TYPE_PIPE);
    RefPtr<eqNet::ConnectionDescription> connDesc = 
        new eqNet::ConnectionDescription;

    connDesc->type = eqNet::TYPE_PIPE;

    if( !connection->connect( connDesc ))
        exit( EXIT_FAILURE );

    RefPtr<Node> node        = new Node;
    RefPtr<Node> server      = new Node;
    RefPtr<Node> nodeProxy   = new Node;
    RefPtr<Node> serverProxy = new Node;

    TEST( node->listen( ));
    TEST( server->listen( ));
    TEST( node->connect( RefPtr_static_cast<eqNet::Node, Node>(serverProxy),
                         connection ));
    TEST( serverProxy->isConnected( ));

    eqNet::PipeConnection* pipeConnection = 
        (eqNet::PipeConnection*)connection.get();

    TEST( server->connect( RefPtr_static_cast<eqNet::Node, Node>(nodeProxy),
                           pipeConnection->getChildEnd( )));
    TEST( nodeProxy->isConnected( ));

    Session session;
    TEST( node->mapSession( RefPtr_static_cast<eqNet::Node, Node>(serverProxy),
                            &session, "foo" ));
    
    Session *sessionMaster = dynamic_cast<Session*>(server->findSession("foo"));
    TEST( sessionMaster );

    TestMobject obj;
    session.registerMobject( &obj, serverProxy.get( ));
    const uint32_t id = obj.getID();
    TEST( id != INVALID_ID );

    TestMobject* objProxy = (TestMobject*)sessionMaster->getMobject( id );
    TEST( objProxy );
    TEST( objProxy->getID() == id );

    sleep(1);

    TEST( server->stopListening( ));
    TEST( node->stopListening( ));

    TEST( connection->getRefCount() == 1 );
    TEST( node->getRefCount() == 1 );
    TEST( server->getRefCount() == 1 );
    TEST( nodeProxy->getRefCount() == 1 );
    TEST( serverProxy->getRefCount() == 1 );
}

