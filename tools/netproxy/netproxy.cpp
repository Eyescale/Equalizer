
/* Copyright (c) 2009-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

// Proxy forwarding between two sockets
// Usage: see 'netPerf -h'

#include <co/connection.h>
#include <co/connectionDescription.h>
#include <co/connectionSet.h>
#include <co/global.h>
#include <co/init.h>

#include <iostream>

#define BUFFERSIZE 1024

int main( int argc, char **argv )
{
    if( argc < 3 )
    {
        LBINFO << "Usage: " << argv[0] << " input output" << std::endl;
        return EXIT_FAILURE;
    }

    co::init( argc, argv );

    co::ConnectionDescriptionPtr listen = new co::ConnectionDescription;
    listen->type = co::CONNECTIONTYPE_TCPIP;
    listen->port = co::Global::getDefaultPort();
    std::string listenArg( argv[1] );
    listen->fromString( listenArg );

    co::ConnectionDescriptionPtr forward = new co::ConnectionDescription;
    forward->type = co::CONNECTIONTYPE_TCPIP;
    forward->port = co::Global::getDefaultPort() + 1;
    std::string forwardArg( argv[2] );
    forward->fromString( forwardArg );

    // wait for input connection
    co::ConnectionPtr connection = co::Connection::create( listen );
    if( !connection )
    {
        LBWARN << "Unsupported connection: " << listen << std::endl;
        co::exit();
        return EXIT_FAILURE;
    }

    if( !connection->listen( ))
    {
        LBERROR << "Can't open listening socket " << listen << std::endl;
        co::exit();
        return EXIT_FAILURE;
    }
    connection->acceptNB();

    co::ConnectionSet connections;
    connections.addConnection( connection );
    connections.select();

    // remove listener, add input connection
    connection = connections.getConnection();
    connections.removeConnection( connection );

    co::ConnectionPtr input = connection->acceptSync();
    uint8_t inputBuffer[ BUFFERSIZE ];
    input->readNB( inputBuffer, BUFFERSIZE );
    connections.addConnection( input );

    // connect forwarding socket
    co::ConnectionPtr output = co::Connection::create( forward );
    if( !output->connect( ))
    {
        LBERROR << "Can't connect forwarding socket " << forward << std::endl;
        co::exit();
        return EXIT_FAILURE;
    }

    uint8_t outputBuffer[ BUFFERSIZE ];
    output->readNB( outputBuffer, BUFFERSIZE );
    connections.addConnection( output );

    while( true )
    {
        switch( connections.select( )) // ...get next request
        {
            case co::ConnectionSet::EVENT_DATA:  // new data
            {
                connection = connections.getConnection();
                const bool isInput = (connection == input);
                uint8_t* buffer = isInput ? inputBuffer : outputBuffer;
                const int64_t read = connection->readSync( buffer, BUFFERSIZE,
                                                           true );
                if( read < 0 ) // error
                {
                    LBINFO << "Socket disconnected" << std::endl;
                    return EXIT_SUCCESS;
                }
                else if( read )
                {
                    if( isInput )
                        output->send( buffer, read );
                    else
                        input->send( buffer, read );
                }
                connection->readNB( buffer, BUFFERSIZE );
                break;
            }

            case co::ConnectionSet::EVENT_DISCONNECT:
            case co::ConnectionSet::EVENT_INVALID_HANDLE:
                LBINFO << "Socket disconnected" << std::endl;
                return EXIT_SUCCESS;

            case co::ConnectionSet::EVENT_INTERRUPT:
                break;

            case co::ConnectionSet::EVENT_CONNECT:
            default:
                assert( 0 );
                break;
        }
    }

    return EXIT_SUCCESS;
}
