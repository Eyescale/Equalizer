
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
#include <eq/base/monitor.h>
#include <eq/net/connection.h>
#include <eq/net/connectionDescription.h>
#include <eq/net/connectionSet.h>
#include <eq/net/init.h>

#include <iostream>

#define PACKETSIZE (2048)

static eq::net::ConnectionType types[] =
{
    eq::net::CONNECTIONTYPE_TCPIP,
    eq::net::CONNECTIONTYPE_PIPE,
#if defined(EQ_USE_BOOST) || defined(EQ_PGM)
    eq::net::CONNECTIONTYPE_MCIP,
#endif
#ifdef EQ_USE_BOOST
    eq::net::CONNECTIONTYPE_RSP,
#endif
#ifdef WIN32
    eq::net::CONNECTIONTYPE_NAMEDPIPE,
#endif
#ifdef EQ_INFINIBAND
    eq::net::CONNECTIONTYPE_IB,
#endif
#ifdef EQ_PGM
    eq::net::CONNECTIONTYPE_PGM,
#endif

    eq::net::CONNECTIONTYPE_NONE // must be last
};


int main( int argc, char **argv )
{
    eq::net::init( argc, argv );

    for( size_t i = 0; types[i] != eq::net::CONNECTIONTYPE_NONE; ++i )
    {
        eq::net::ConnectionDescriptionPtr desc = 
            new eq::net::ConnectionDescription;
        desc->type = types[i];
        if( desc->type >= eq::net::CONNECTIONTYPE_MULTICAST )
            desc->setHostname( "239.255.12.34" );
        else
            desc->setHostname( "127.0.0.1" );

        eq::net::ConnectionPtr listener = eq::net::Connection::create( desc );
        eq::net::ConnectionPtr writer;
        eq::net::ConnectionPtr reader;

        switch( desc->type ) // different connections, different semantics..
        {
            case eq::net::CONNECTIONTYPE_PIPE:
                writer = listener;
                listener = 0;
                reader = writer;

                TEST( writer->connect( ));
                break;

            case eq::net::CONNECTIONTYPE_MCIP:
            case eq::net::CONNECTIONTYPE_RSP:
            case eq::net::CONNECTIONTYPE_PGM:
                TESTINFO( listener->listen(), desc );
                listener->acceptNB();

                writer = listener;
                reader = listener->acceptSync();
                break;

            default:
                TESTINFO( listener->listen(), desc );
                listener->acceptNB();

                writer = eq::net::Connection::create( desc );
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
