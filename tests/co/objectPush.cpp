
/* Copyright (c) 2011-2012, Stefan Eilemann <eile@eyescale.ch> 
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
#include <co/global.h>
#include <co/init.h>
#include <co/node.h>
#include <co/object.h>
#include <lunchbox/clock.h>
#include <lunchbox/monitor.h>
#include <lunchbox/rng.h>

#include <iostream>

using co::uint128_t;

namespace
{

lunchbox::Monitor< co::Object::ChangeType > monitor( co::Object::NONE ); 

static const std::string message =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Ut eget felis sed leo tincidunt dictum eu eu felis. Aenean aliquam augue nec elit tristique tempus. Pellentesque dignissim adipiscing tellus, ut porttitor nisl lacinia vel. Donec malesuada lobortis velit, nec lobortis metus consequat ac. Ut dictum rutrum dui. Pellentesque quis risus at lectus bibendum laoreet. Suspendisse tristique urna quis urna faucibus et auctor risus ultricies. Morbi vitae mi vitae nisi adipiscing ultricies ac in nulla. Nam mattis venenatis nulla, non posuere felis tempus eget. Cras dapibus ultrices arcu vel dapibus. Nam hendrerit lacinia consectetur. Donec ullamcorper nibh nisl, id aliquam nisl. Nunc at tortor a lacus tincidunt gravida vitae nec risus. Suspendisse potenti. Fusce tristique dapibus ipsum, sit amet posuere turpis fermentum nec. Nam nec ante dolor.";
}

class Object : public co::Object
{
public:
    Object( const ChangeType type ) : _type( type ){}
    Object( const ChangeType type, co::DataIStream& is )
            : _type( type )
        { applyInstanceData( is ); }

protected:
    virtual ChangeType getChangeType() const { return _type; }
    virtual void getInstanceData( co::DataOStream& os )
        { os << message << _type; }

    virtual void applyInstanceData( co::DataIStream& is )
        {
            std::string msg;
            ChangeType type;
            is >> msg >> type;
            TEST( message == msg );
            TEST( _type == type );
        }

private:
    const ChangeType _type;
};

class Server : public co::LocalNode
{
public:
    Server() : object( 0 ) {}

    co::Object* object;

protected:
    virtual void objectPush( const uint128_t& groupID, const uint128_t& typeID,
                             const uint128_t& objectID,
                             co::DataIStream& istream )
        {
            const co::Object::ChangeType type =
                co::Object::ChangeType( typeID.low( ));
            TESTINFO( istream.nRemainingBuffers() == 1 || // buffered
                      istream.nRemainingBuffers() == 2,   // unbuffered
                      istream.nRemainingBuffers( ));
            TEST( !object );
            object = new Object( type, istream );
            TESTINFO( !istream.hasData(), istream.nRemainingBuffers( ));
            monitor = type;
        }

private:
};

int main( int argc, char **argv )
{
    co::init( argc, argv );
    co::Global::setObjectBufferSize( 600 );

    lunchbox::RNG rng;
    const uint16_t port = (rng.get<uint16_t>() % 60000) + 1024;

    lunchbox::RefPtr< Server > server = new Server;
    co::ConnectionDescriptionPtr connDesc = 
        new co::ConnectionDescription;
    
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

    co::Nodes nodes;
    nodes.push_back( serverProxy );

    lunchbox::Clock clock;
    for( unsigned i = co::Object::NONE+1; i <= co::Object::UNBUFFERED; ++i )
    {
        const co::Object::ChangeType type = co::Object::ChangeType( i );
        Object object( type );
        TEST( client->registerObject( &object ));
        object.push( 42, i, nodes );

        monitor.waitEQ( type );
        TEST( server->mapObject( server->object, object.getID(),
                                 co::VERSION_NONE ));
        server->unmapObject( server->object );
        delete server->object;
        server->object = 0;

        client->deregisterObject( &object );
    }
    const float time = clock.getTimef();
    nodes.clear();

    std::cout << time << "ms for " << int( co::Object::UNBUFFERED )
              << " object types" << std::endl;

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
