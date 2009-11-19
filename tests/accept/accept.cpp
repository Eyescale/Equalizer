
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
// Tests multiple simultaneous connects
// Usage: see 'accept -h'

#include <pthread.h> // must come first!

#include <test.h>
#include <eq/base/monitor.h>
#include <eq/base/sleep.h>
#include <eq/net/connection.h>
#include <eq/net/connectionDescription.h>
#include <eq/net/connectionSet.h>
#include <eq/net/init.h>

#ifndef MIN
#  define MIN EQ_MIN
#endif
#include <tclap/CmdLine.h>

#include <iostream>

using namespace eq::net;

int main( int argc, char **argv )
{
    TEST( eq::net::init( argc, argv ));

    ConnectionDescriptionPtr description = new ConnectionDescription;
    description->type = CONNECTIONTYPE_TCPIP;
    description->port = 4242;

    bool  isClient  = true;
    size_t waitTime = 0;

    try // command line parsing
    {
        TCLAP::CmdLine command(
            "accept - Test case for simultaneous network connects\n");
        TCLAP::ValueArg<std::string> clientArg( "c", "client", "run as client", 
                                           true, "", "IP[:port]" );
        TCLAP::ValueArg<std::string> serverArg( "s", "server", "run as server", 
                                           true, "", "IP[:port]" );
        TCLAP::ValueArg<size_t> waitArg( "w", "wait", 
                              "wait time (ms) before disconnect (client only)", 
                                         false, 0, "unsigned", command );

        command.xorAdd( clientArg, serverArg );
        command.parse( argc, argv );

        if( clientArg.isSet( ))
            description->fromString( clientArg.getValue( ));
        else if( serverArg.isSet( ))
        {
            isClient = false;
            description->fromString( serverArg.getValue( ));
        }

        if( waitArg.isSet( ))
            waitTime = waitArg.getValue();
    }
    catch( TCLAP::ArgException& exception )
    {
        EQERROR << "Command line parse error: " << exception.error() 
                << " for argument " << exception.argId() << std::endl;
        
        eq::net::exit();
        return EXIT_FAILURE;
    }

    // run
    ConnectionPtr connection = Connection::create( description );
    eq::base::Clock clock;
    size_t nConnects = 0;
    if( isClient )
    {
        uint32_t data = 42;
        while( true )
        {
            TEST( connection->connect( ));
            TEST( connection->send( &data, 4, true ));

            connection->recvNB( &data, 4 );
            while( !connection->recvSync( 0, 0 ))
                OUTPUT << "Empty read" << std::endl;

            connection->close();

            if( waitTime > 0 )
                eq::base::sleep( waitTime );

            ++nConnects;
            const float time = clock.getTimef();
            if( time > 1000.0f )
            {
                OUTPUT << nConnects / time * 1000.f << " connects/s "
                       << std::endl;
                nConnects = 0;
                clock.reset();
            }
        }
    }
    else
    {
        TEST( connection->listen( ));
        connection->acceptNB();

        ConnectionSet connectionSet;
        connectionSet.addConnection( connection );

        while( true )
        {
            switch( connectionSet.select( )) // ...get next request
            {
                case ConnectionSet::EVENT_CONNECT: // new client
                {
                    connection = connectionSet.getConnection();
                    ConnectionPtr newConnection = connection->acceptSync();
                    connection->acceptNB();

                    TEST( newConnection.isValid( ));
                    connection->recvNB( new uint32_t, sizeof( uint32_t ));
                    connectionSet.addConnection( newConnection );
                    ++nConnects;
                    break;
                }
                case ConnectionSet::EVENT_ERROR:
                    OUTPUT << "Error on connection " << std::endl;
                    // no break;
                case ConnectionSet::EVENT_DISCONNECT:
                case ConnectionSet::EVENT_INVALID_HANDLE:  // client done
                {
                    connection = connectionSet.getConnection();
                    connectionSet.removeConnection( connection );

                    void* bufferPtr( 0 );
                    uint64_t bytes( 0 );
                    connection->getRecvData( &bufferPtr, &bytes );

                    TEST( bufferPtr );
                    TEST( bytes == sizeof( uint32_t ));
                    delete reinterpret_cast< uint32_t* >( bufferPtr );

                    TESTINFO( connection->getRefCount() == 1, 
                              connection->getRefCount( ));
                    break;
                }
                case ConnectionSet::EVENT_INTERRUPT:
                    break;

                case ConnectionSet::EVENT_SELECT_ERROR:
                    OUTPUT << "Error during select, " 
                           << connectionSet.getSize() << " connections open"
                           << std::endl;
                    break;

                case ConnectionSet::EVENT_DATA:
                {
                    connection = connectionSet.getConnection();
                    void* buffer( 0 );
                    uint64_t bytes( 0 );
                    
                    if( connection->recvSync( &buffer, &bytes ))
                    {
                        TEST( buffer );
                        TEST( bytes == 4 );
                        TEST( connection->send( buffer, bytes, true ));
                        connection->recvNB( buffer, bytes );
                    }
                    break;
                }
                default:
                    TESTINFO( false, "Not reachable" );
            }

            const float time = clock.getTimef();
            if( time > 1000.0f )
            {
                OUTPUT << nConnects / time * 1000.f << " accepts/s, "
                       << connectionSet.getSize() << " connections open"
                       << std::endl;
                nConnects = 0;
                clock.reset();
            }
        }
    }

    TESTINFO( connection->getRefCount() == 1, connection->getRefCount( ));
    connection->close();
    return EXIT_SUCCESS;
}
