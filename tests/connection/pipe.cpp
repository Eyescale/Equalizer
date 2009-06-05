
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

#include <eq/base/thread.h>
#include <eq/net/connection.h>
#include <eq/net/connectionDescription.h>
#include <eq/net/init.h>
#include <eq/net/pipeConnection.h>

#include <iostream>

using namespace eq::base;
using namespace eq::net;
using namespace std;

class Server : public eq::base::Thread
{
public:
    void start( RefPtr<Connection> connection )
        {
            _connection = connection;
            eq::base::Thread::start();
        }

protected:
    virtual void* run()
        {
            TEST( _connection.isValid( ));
            TEST( _connection->getState() == Connection::STATE_CONNECTED );

            char text[5];
            _connection->recvNB(  &text, 5 );
            TEST( _connection->recvSync( 0, 0 ));
            TEST( strcmp( "buh!", text ) == 0 );

            _connection->close();
            _connection = NULL;
            return EXIT_SUCCESS;
        }
private:
    RefPtr<Connection> _connection;
};

int main( int argc, char **argv )
{
    eq::net::init( argc, argv );

    RefPtr<Connection>  connection = new PipeConnection();
    TEST( connection->connect( ));

    Server server;
    server.start( connection );

    const char message[] = "buh!";
    const size_t nChars  = strlen( message ) + 1;

    TEST( connection->send( message, nChars ) == nChars );

    connection->close();
    connection = NULL;

    server.join();
    
    return EXIT_SUCCESS;
}
