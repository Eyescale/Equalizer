
/* Copyright (c) 2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

// Tests basic connection functionality
#include <test.h>
#include <co/base/monitor.h>
#include <co/connection.h>
#include <co/connectionDescription.h>
#include <co/connectionSet.h>
#include <co/init.h>

#include <iostream>

#define PACKETSIZE (2048)

static co::ConnectionType types[] =
{
    co::CONNECTIONTYPE_TCPIP,
    co::CONNECTIONTYPE_PIPE,
#ifdef EQ_USE_BOOST
    co::CONNECTIONTYPE_RSP,
#endif
#ifdef WIN32
    co::CONNECTIONTYPE_NAMEDPIPE,
#endif
#ifdef EQ_INFINIBAND
    co::CONNECTIONTYPE_IB,
#endif
    co::CONNECTIONTYPE_NONE // must be last
};


int main( int argc, char **argv )
{
    co::init( argc, argv );

    for( size_t i = 0; types[i] != co::CONNECTIONTYPE_NONE; ++i )
    {
        co::ConnectionDescriptionPtr desc = 
            new co::ConnectionDescription;
        desc->type = types[i];
        if( desc->type >= co::CONNECTIONTYPE_MULTICAST )
            desc->setHostname( "239.255.12.34" );
        else
            desc->setHostname( "127.0.0.1" );

        co::ConnectionPtr listener = co::Connection::create( desc );
        co::ConnectionPtr writer;
        co::ConnectionPtr reader;

        switch( desc->type ) // different connections, different semantics..
        {
            case co::CONNECTIONTYPE_PIPE:
                writer = listener;
                listener = 0;
                reader = writer;

                TEST( writer->connect( ));
                break;

            case co::CONNECTIONTYPE_MCIP:
            case co::CONNECTIONTYPE_RSP:
                TESTINFO( listener->listen(), desc );
                listener->acceptNB();

                writer = listener;
                reader = listener->acceptSync();
                break;
            default:
                TESTINFO( listener->listen(), desc );
                listener->acceptNB();

                writer = co::Connection::create( desc );
                TEST( writer->connect( ));

                reader = listener->acceptSync();
                break;
        }
        TEST( writer.isValid( ));
        TEST( reader.isValid( ));

        uint8_t in[ PACKETSIZE ];
        reader->recvNB( in, PACKETSIZE );

        uint8_t out[ PACKETSIZE ];
        TEST( writer->send( out, PACKETSIZE ));
        TEST( reader->recvSync( 0, 0 ));

        writer->close();
        reader->recvNB( in, PACKETSIZE );
        TEST( !reader->recvSync( 0, 0 ));
        TEST( reader->isClosed( ));

        if( listener == writer )
            listener = 0;
        if( reader == writer )
            reader = 0;

        if( listener.isValid( ))
            TEST( listener->getRefCount() == 1 );
        if( reader.isValid( ))
            TEST( reader->getRefCount() == 1 );
        TEST( writer->getRefCount() == 1 );
    }

    return EXIT_SUCCESS;
}
