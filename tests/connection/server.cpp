
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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
    description->TCPIP.port = 0;

    ConnectionPtr            connection = Connection::create( description );
    TEST( connection->listen( ));

    RefPtr<Connection> client = connection->accept();
    cerr << "Server accepted connection" << endl;
    connection->close();

    char c;
    while( client->recv( &c, 1 ))
    {
        cerr << "Server recv: " << c << endl;
        TEST( client->send( &c, 1 ) == 1 );
    }

    return EXIT_SUCCESS;
}
