
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
#include <co/init.h>
#include <co/localNode.h>
#include <co/zeroconf.h>
#include <lunchbox/rng.h>

#include <iostream>

using co::uint128_t;

int main( int argc, char **argv )
{
#ifdef CO_USE_SERVUS
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

    co::LocalNodePtr client = new co::LocalNode;
    TEST( client->listen( ));

    co::NodePtr serverProxy = client->connect( server->getNodeID( ));
    TEST( serverProxy );

    co::Zeroconf zeroconf = server->getZeroconf();
    zeroconf.set( "co_test_value", "42" );
    lunchbox::sleep( 500 ); // give it time to propagate
    zeroconf = client->getZeroconf(); // rediscover, use other peer for a change

    const co::Strings& instances = zeroconf.getInstances();
    bool found = false;
    TEST( instances.size() >= 1 );

    for( co::StringsCIter i = instances.begin(); i != instances.end(); ++i )
    {
        const std::string& instance = *i;
        const uint128_t nodeID( instance );
        TEST( nodeID != co::NodeID::ZERO );
        TEST( nodeID != client->getNodeID( ));

        if( nodeID != server->getNodeID( ))
            continue;

        TEST( zeroconf.get( instance, "co_numPorts" ) == "1" );
        TEST( zeroconf.get( instance, "co_test_value" ) == "42" );
        found = true;
    }

    TEST( found );
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
#endif
    return EXIT_SUCCESS;
}
