
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com>
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

#include <co/base/thread.h>
#include <co/connection.h>
#include <co/connectionDescription.h>
#include <co/init.h>

#include <iostream>

#define SIZE EQ_64MB
void* buffer = malloc( SIZE );

class Server : public co::base::Thread
{
public:
    void start( co::ConnectionPtr connection )
        {
            _connection = connection;
            co::base::Thread::start();
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

            co::base::Clock _clock;
            _connection->recvNB( buffer, SIZE );
            TEST( _connection->recvSync( 0, 0 ));
            std::cout << "Recv perf: " << SIZE / _clock.getTimef() / 1024 << "MB/s" << std::endl;

            _connection->close();
            _connection = 0;
        }
private:
    co::ConnectionPtr _connection;
};

int main( int argc, char **argv )
{
    co::init( argc, argv );

    co::ConnectionDescriptionPtr desc = new co::ConnectionDescription;
    desc->type = co::CONNECTIONTYPE_PIPE;
    co::ConnectionPtr connection = co::Connection::create( desc );
    TEST( connection->connect( ));

    Server server;
    server.start( connection );

    const char message[] = "buh!";
    const size_t nChars  = strlen( message ) + 1;

    TEST( connection->send( message, nChars ));

    co::base::Clock _clock;
    TEST( connection->send( buffer, SIZE ));
    std::cout << "Send perf: " << SIZE / _clock.getTimef() / 1024 << "MB/s" << std::endl;

    server.join();

    connection->close();
    connection = 0;
    
    return EXIT_SUCCESS;
}
