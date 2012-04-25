
/* Copyright (c) 2012, Stefan Eilemann <eile@eyescale.ch> 
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

#include <co/connection.h>
#include <co/connectionDescription.h>
#include <co/dataIStream.h>
#include <co/dataOStream.h>
#include <co/init.h>
#include <co/node.h>
#include <co/object.h>
#include <lunchbox/clock.h>
#include <lunchbox/monitor.h>
#include <lunchbox/rng.h>
#include <lunchbox/thread.h>

#include <iostream>

using co::uint128_t;

namespace
{
static const std::string message =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Ut eget felis sed leo tincidunt dictum eu eu felis. Aenean aliquam augue nec elit tristique tempus. Pellentesque dignissim adipiscing tellus, ut porttitor nisl lacinia vel. Donec malesuada lobortis velit, nec lobortis metus consequat ac. Ut dictum rutrum dui. Pellentesque quis risus at lectus bibendum laoreet. Suspendisse tristique urna quis urna faucibus et auctor risus ultricies. Morbi vitae mi vitae nisi adipiscing ultricies ac in nulla. Nam mattis venenatis nulla, non posuere felis tempus eget. Cras dapibus ultrices arcu vel dapibus. Nam hendrerit lacinia consectetur. Donec ullamcorper nibh nisl, id aliquam nisl. Nunc at tortor a lacus tincidunt gravida vitae nec risus. Suspendisse potenti. Fusce tristique dapibus ipsum, sit amet posuere turpis fermentum nec. Nam nec ante dolor.";

class Object : public co::Object
{
public:
    Object() {}

protected:
    virtual ChangeType getChangeType() const { return UNBUFFERED; }
    virtual uint64_t getMaxVersions() const { return 1; }

    virtual void getInstanceData( co::DataOStream& os ) { os << message; }

    virtual void applyInstanceData( co::DataIStream& is )
        {
            std::string msg;
            is >> msg;
            TEST( message == msg );
        }
};

class Thread : public lunchbox::Thread
{
public:
    Thread( Object& object ) : _object( object ) {}

    virtual void run()
        {
            lunchbox::sleep( 150 );
            _object.sync( 3 );
            TESTINFO( _object.getVersion() == 3, _object.getVersion( ));
        }

private:
    Object& _object;
};
}

int main( int argc, char **argv )
{
    co::init( argc, argv );
    lunchbox::RNG rng;
    const uint16_t port = (rng.get<uint16_t>() % 60000) + 1024;

    co::LocalNodePtr server = new co::LocalNode;
    co::ConnectionDescriptionPtr connDesc = new co::ConnectionDescription;
    
    connDesc->type = co::CONNECTIONTYPE_TCPIP;
    connDesc->port = port;
    connDesc->setHostname( "localhost" );

    server->addConnectionDescription( connDesc );
    TEST( server->listen( ));

    co::NodePtr serverProxy = new co::Node;
    serverProxy->addConnectionDescription( connDesc );

    connDesc = new co::ConnectionDescription;
    connDesc->type = co::CONNECTIONTYPE_TCPIP;
    connDesc->setHostname( "localhost" );

    co::LocalNodePtr client = new co::LocalNode;
    client->addConnectionDescription( connDesc );
    TEST( client->listen( ));
    TEST( client->connect( serverProxy ));

    Object master;
    TEST( client->registerObject( &master ));

    Object slave;
    TEST( server->mapObject( &slave, master.getID( )));
    
    Thread thread( slave );
    TEST( thread.start( ));

    lunchbox::Clock clock;
    master.commit();
    master.commit(); // should block
    const float time = clock.getTimef();

    TESTINFO( master.getVersion() == 3, master.getVersion( ));
    TESTINFO( time > 100.f, time );

    server->unmapObject( &slave );
    client->deregisterObject( &master );

    TEST( client->disconnect( serverProxy ));
    TEST( client->close( ));
    TEST( server->close( ));

    serverProxy->printHolders( std::cerr );
    TESTINFO( serverProxy->getRefCount() == 1, serverProxy->getRefCount( ));
    TESTINFO( client->getRefCount() == 1, client->getRefCount( ));
    TESTINFO( server->getRefCount() == 1, server->getRefCount( ));

    serverProxy = 0;
    client      = 0;
    server      = 0;

    return EXIT_SUCCESS;
}
