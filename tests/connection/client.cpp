
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
    description->port = 4242;

    ConnectionPtr            connection = Connection::create( description );
    TEST( connection->connect( ));

    const char     message[] = "buh!";
    const uint64_t nChars    = strlen( message ) + 1;
    char*          response  = static_cast<char*>( alloca( nChars ));

    TEST( connection->send( message, nChars ) == nChars );
    connection->recvNB( response, nChars );
    TEST( connection->recvSync( 0, 0 ) == nChars );
    cerr << "Client recv: " << response << endl;
    connection->close();

    return EXIT_SUCCESS;
}
