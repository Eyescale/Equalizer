
/* Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <test.h>

#include <eq/net/connection.h>
#include <eq/net/init.h>
#include <eq/net/pipeConnection.h>
#include <eq/net/session.h>
#include <eq/net/object.h>
#include <eq/client/nodeFactory.h>

#include <iostream>

using namespace eq::base;
using namespace std;

eq::NodeFactory* eq::createNodeFactory() { return new eq::NodeFactory; }

class TestObject : public eq::net::Object
{
public:
    TestObject() 
            : Object( eq::net::Object::TYPE_MANAGED_CUSTOM ), 
              value(42) {}

protected:
    int value;

    const void* getInstanceData( uint64_t* size )
        {
            *size = sizeof( value );
            return &value;
        }
};

RefPtr<eq::net::Connection> connection;
volatile uint32_t         testID = EQ_ID_INVALID;

class ServerThread : public eq::base::Thread
{
protected:
    virtual void* run()
        {
            RefPtr<eq::net::Node> server    = new eq::net::Node;
            RefPtr<eq::net::Node> nodeProxy = new eq::net::Node;

            TEST( server->listen( ));

            eq::net::PipeConnection* pipeConnection = 
                (eq::net::PipeConnection*)connection.get();

            TEST( server->connect( nodeProxy, pipeConnection->getChildEnd( )));
            TEST( nodeProxy->isConnected( ));
                        
            eq::net::Session session;
            TEST( server->mapSession( server, &session, "foo" ));

            while( testID == EQ_ID_INVALID ); // spin for test object
            
            RefPtr<eq::net::Object> object = session.getObject( testID );
            TEST( dynamic_cast<TestObject*>(object.get()) );
                
            TestObject* testObject = (TestObject*) object.get();
            TEST( testObject );
            TEST( testObject->getID() == testID );

            TEST( server->stopListening( ));
            return EXIT_SUCCESS;
        }
};

int main( int argc, char **argv )
{
    eq::net::init( argc, argv );

    connection = new eq::net::PipeConnection();
    TEST( connection->connect( ));

    ServerThread server;
    server.start();

    RefPtr<eq::net::Node> node        = new eq::net::Node;
    RefPtr<eq::net::Node> serverProxy = new eq::net::Node;

    TEST( node->listen( ));
    TEST( node->connect( serverProxy, connection ));
    TEST( serverProxy->isConnected( ));

    eq::net::Session session;
    TEST( node->mapSession( serverProxy, &session, "foo" ));
    
    TestObject obj;
    session.registerObject( &obj );

    testID = obj.getID();
    TEST( testID != EQ_ID_INVALID );

    TEST( node->stopListening( ));
    TEST( server.join( ));
}
