
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com>
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
#include <eq/net/connectionDescription.h>
#include <eq/net/init.h>

#include <iostream>

using namespace eq::net;
using namespace eq::base;
using namespace std;

int main( int argc, char **argv )
{
    eq::net::init( argc, argv );

    ConnectionDescriptionPtr description = new ConnectionDescription;
    description->type = CONNECTIONTYPE_TCPIP;
    description->setHostname( "localhost" );
    description->port = 0;

    ConnectionPtr            connection = Connection::create( description );
    TEST( connection->listen( ));

    connection->acceptNB();
    RefPtr<Connection> client = connection->acceptSync();
    cerr << "Server accepted connection" << endl;
    connection->close();

    char c;
    client->recvNB( &c, 1 );
    while( client->recvSync( 0, 0 ))
    {
        cerr << "Server recv: " << c << endl;
        TEST( client->send( &c, 1 ) == 1 );
        client->recvNB( &c, 1 );
    }

    return EXIT_SUCCESS;
}
