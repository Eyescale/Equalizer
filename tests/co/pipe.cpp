
/* Copyright (c) 2006-2011, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <lunchbox/thread.h>
#include <co/init.h>

#include <iostream>

#include <co/pipeConnection.h> // private header

class Server : public lunchbox::Thread
{
public:
    void start( co::ConnectionPtr connection )
        {
            _connection = connection;
            lunchbox::Thread::start();
        }

protected:
    virtual void run()
        {
            TEST( _connection.isValid( ));
            TEST( _connection->getState() == 
                  co::Connection::STATE_CONNECTED );

            char text[5];
            _connection->recvNB( &text, 5 );
            TEST( _connection->recvSync( 0, 0 ));
            TEST( strcmp( "buh!", text ) == 0 );

            _connection->close();
            _connection = 0;
        }
private:
    co::ConnectionPtr _connection;
};

int main( int argc, char **argv )
{
    co::init( argc, argv );
    co::PipeConnectionPtr connection = new co::PipeConnection;
    TEST( connection->connect( ));

    Server server;
    server.start( connection->acceptSync( ));

    const char message[] = "buh!";
    const size_t nChars  = strlen( message ) + 1;

    TEST( connection->send( message, nChars ));

    server.join();

    connection->close();
    connection = 0;
    
    return EXIT_SUCCESS;
}
