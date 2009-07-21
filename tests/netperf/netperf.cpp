
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

// Tests network throughput
// Usage: see 'netPerf -h'

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
using eq::base::Clock;
using namespace std;

namespace
{
    class Receiver;
    ConnectionSet    _connectionSet;
    eq::base::mtLong _nClients;

    class Receiver : public eq::base::Thread
    {
    public:
        Receiver( const size_t packetSize, ConnectionPtr connection )
                : _connection( connection )
            { 
                _buffer.resize( packetSize );
                connection->recvNB( _buffer.getData(), _buffer.getSize() );
            }
        virtual ~Receiver() {}

        bool readPacket()
        {
            const float mBytesSec = _buffer.getSize() / 1024.f / 1024.f *1000.f;

            _clock.reset();
            if( !_connection->recvSync( 0, 0 ))
                return false;

            const float time = _clock.getTimef();
            _connection->recvNB( _buffer.getData(), _buffer.getSize() );

            eq::net::ConnectionDescriptionPtr desc = 
                _connection->getDescription();
            cerr << " Recv perf: " << mBytesSec / time << "MB/s ("
                 << time << "ms) from " << desc.get() << endl;
            return true;
        }

        void executeReceive()
        {
            TEST( _hasConnection == false );
            _hasConnection = true;
        }

        virtual void* run()
        {
            while( true )
            {
                _hasConnection.waitEQ( true );

                if( readPacket( ))
                {
                    _connectionSet.addConnection( _connection );
                }
                else // dead connection
                {
                    cerr << --_nClients << " clients" << endl;
                    _connectionSet.interrupt();
                }

                _hasConnection = false;
            }
            return EXIT_SUCCESS;
        }

    private:
        Clock _clock;

        eq::base::Bufferb _buffer;
        eq::base::Monitor< bool > _hasConnection;
        ConnectionPtr             _connection;
    };
}

int main( int argc, char **argv )
{
    TEST( eq::net::init( argc, argv ));

    ConnectionDescriptionPtr description = new ConnectionDescription;
    description->type       = CONNECTIONTYPE_TCPIP;
    description->TCPIP.port = 4242;

    bool isClient     = true;
    bool useThreads   = false;
    size_t packetSize = 1048576;
    size_t nPackets   = 0xffffffffu;
    size_t waitTime   = 0;

    try // command line parsing
    {
        TCLAP::CmdLine command( "netPerf - Equalizer network benchmark tool\n");
        TCLAP::ValueArg<string> clientArg( "c", "client", "run as client", 
                                           true, "", "IP[:port]" );
        TCLAP::ValueArg<string> serverArg( "s", "server", "run as server", 
                                           true, "", "IP[:port]" );
        TCLAP::SwitchArg threadedArg( "t", "threaded", 
                         "Run each receive in a separate thread (server only)", 
                                      command, false );
        TCLAP::ValueArg<size_t> sizeArg( "p", "packetSize", "packet size", 
                                         false, packetSize, "unsigned", 
                                         command );
        TCLAP::ValueArg<size_t> packetsArg( "n", "numPackets", 
                                            "number of packets to send", 
                                            false, nPackets, "unsigned",
                                            command );
        TCLAP::ValueArg<size_t> waitArg( "w", "wait", 
                                  "wait time (ms) between sends (client only)", 
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

        useThreads = threadedArg.isSet();

        if( sizeArg.isSet( ))
            packetSize = sizeArg.getValue();
        if( packetsArg.isSet( ))
            nPackets = packetsArg.getValue();
        if( waitArg.isSet( ))
            waitTime = waitArg.getValue();
    }
    catch( TCLAP::ArgException& exception )
    {
        EQERROR << "Command line parse error: " << exception.error() 
            << " for argument " << exception.argId() << endl;

        eq::net::exit();
        return EXIT_FAILURE;
    }

    // run
    ConnectionPtr connection = Connection::create( description );

    if( isClient )
    {
        TEST( connection->connect( ));

        eq::base::Buffer< uint8_t > buffer;
        buffer.resize( packetSize );

        const float mBytesSec = buffer.getSize() / 1024.0f / 1024.0f * 1000.0f;
        Clock       clock;
        size_t      packetNum = 0;

        while( nPackets-- )
        {
            clock.reset();
            TEST( connection->send( buffer.getData(), buffer.getSize() ));
            const float time = clock.getTimef();
            cerr << ++packetNum << " Send perf: " << mBytesSec / time 
                 << "MB/s (" << time << "ms)" << endl;
            if( waitTime > 0 )
                eq::base::sleep( waitTime );
        }
    }
    else
    {
        TEST( connection->listen( ));
        connection->acceptNB();

        _connectionSet.addConnection( connection );

        // Get first client
        const ConnectionSet::Event event = _connectionSet.select();
        TEST( event == ConnectionSet::EVENT_CONNECT );

        ConnectionPtr resultConn = _connectionSet.getConnection();
        ConnectionPtr newConn    = resultConn->acceptSync();
        resultConn->acceptNB();

        TEST( resultConn == connection );
        TEST( newConn.isValid( ));

        typedef std::pair< Receiver*, ConnectionPtr > RecvConn;
        vector< RecvConn > receivers;

        receivers.push_back( RecvConn( new Receiver( packetSize, newConn ),
                                       newConn ));

        _connectionSet.addConnection( newConn );

        // Until all client have disconnected...
        _nClients = 1;
        while( _nClients > 0 )
        {
            switch( _connectionSet.select( )) // ...get next request
            {
                case ConnectionSet::EVENT_CONNECT: // new client
                    resultConn = _connectionSet.getConnection();
                    TEST( resultConn == connection );

                    newConn = resultConn->acceptSync();
                    resultConn->acceptNB();
                    TEST( newConn.isValid( ));

                    receivers.push_back( 
                        RecvConn( new Receiver(packetSize, newConn), newConn ));

                    _connectionSet.addConnection( newConn );
                    cerr << ++_nClients << " clients" << endl;
                    break;

                case ConnectionSet::EVENT_DATA:  // new data
                {
                    resultConn = _connectionSet.getConnection();

                    Receiver* receiver = 0;
                    vector< RecvConn >::iterator i;
                    for( i = receivers.begin(); i != receivers.end(); ++i )
                    {
                        const RecvConn& candidate = *i;
                        if( candidate.second == resultConn )
                        {
                            receiver = candidate.first;
                            break;
                        }
                    }
                    TEST( receiver );

                    if( useThreads )
                    {
                        _connectionSet.removeConnection( resultConn );
                        receiver->executeReceive();
                    }
                    else if( !receiver->readPacket())
                    {
                        // Connection dead?
                        _connectionSet.removeConnection( resultConn );
                        delete receiver;
                        receivers.erase( i );
                        cerr << --_nClients << " clients" << endl;
                    }
                    break;
                }
                case ConnectionSet::EVENT_DISCONNECT:
                case ConnectionSet::EVENT_INVALID_HANDLE:  // client done
                    resultConn = _connectionSet.getConnection();
                    _connectionSet.removeConnection( resultConn );

                    for( vector< RecvConn >::iterator i = receivers.begin();
                         i != receivers.end(); ++i )
                    {
                        const RecvConn& candidate = *i;
                        if( candidate.second == resultConn )
                        {
                            delete candidate.first;
                            receivers.erase( i );
                            break;
                        }
                    }

                    cerr << --_nClients << " clients" << endl;
                    break;

                case ConnectionSet::EVENT_INTERRUPT:
                    break;

                default:
                    TESTINFO( false, "Not reachable" );
            }
        }

        TESTINFO( receivers.empty(), receivers.size() );
        TESTINFO( _connectionSet.size() == 1, _connectionSet.size() );
        _connectionSet.clear();
    }

    // TODO thread tear down
 
    TESTINFO( connection->getRefCount() == 1, connection->getRefCount( ));
    connection->close();
    return EXIT_SUCCESS;
}
