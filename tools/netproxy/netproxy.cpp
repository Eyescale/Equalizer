
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <eq/net/connection.h>
#include <eq/net/connectionDescription.h>
#include <eq/net/connectionSet.h>
#include <eq/net/global.h>
#include <eq/net/init.h>

#include <iostream>

#define BUFFERSIZE 1024

using namespace eq::net;

int main( int argc, char **argv )
{
    if( argc < 3 )
    {
        EQINFO << "Usage: " << argv[0] << " input output" << std::endl;
        return EXIT_FAILURE;

    }

    eq::net::init( argc, argv );

    ConnectionDescriptionPtr listen = new ConnectionDescription;
    listen->type = CONNECTIONTYPE_TCPIP;
    listen->port = Global::getDefaultPort();
    std::string listenArg( argv[1] );
    listen->fromString( listenArg );

    ConnectionDescriptionPtr forward = new ConnectionDescription;
    forward->type = CONNECTIONTYPE_TCPIP;
    forward->port = Global::getDefaultPort() + 1;
    std::string forwardArg( argv[2] );
    forward->fromString( forwardArg );

    // wait for input connection
    ConnectionPtr connection = Connection::create( listen );
    if( !connection->listen( ))
    {
        EQERROR << "Can't open listening socket " << listen << std::endl;
        eq::net::exit();
        return EXIT_FAILURE;
    }
    connection->acceptNB();

    ConnectionSet connections;
    connections.addConnection( connection );
    connections.select();

    // remove listener, add input connection
    connection = connections.getConnection();
    connections.removeConnection( connection );

    ConnectionPtr input = connection->acceptSync();
    uint8_t inputBuffer[ BUFFERSIZE ];
    input->readNB( inputBuffer, BUFFERSIZE );
    connections.addConnection( input );

    // connect forwarding socket
    ConnectionPtr output = Connection::create( forward );
    if( !output->connect( ))
    {
        EQERROR << "Can't connect forwarding socket " << forward << std::endl;
        eq::net::exit();
        return EXIT_FAILURE;
    }

    uint8_t outputBuffer[ BUFFERSIZE ];
    output->readNB( outputBuffer, BUFFERSIZE );
    connections.addConnection( output );

    while( true )
    {
        switch( connections.select( )) // ...get next request
        {
            case ConnectionSet::EVENT_DATA:  // new data
            {
                connection = connections.getConnection();
                const bool isInput = (connection == input);
                uint8_t* buffer = isInput ? inputBuffer : outputBuffer;
                const int64_t read = connection->readSync( buffer, BUFFERSIZE,
                                                           true );
                if( read < 0 ) // error
                {
                    EQINFO << "Socket disconnected" << std::endl;
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

            case ConnectionSet::EVENT_DISCONNECT:
            case ConnectionSet::EVENT_INVALID_HANDLE:
                EQINFO << "Socket disconnected" << std::endl;
                return EXIT_SUCCESS;

            case ConnectionSet::EVENT_INTERRUPT:
                break;

            case ConnectionSet::EVENT_CONNECT:
            default:
                assert( 0 );
                break;
        }
    }

    return EXIT_SUCCESS;
}
