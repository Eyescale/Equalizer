
/* Copyright (c) 2006-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#define EQ_RELEASE_ASSERT

#include <co/co.h>

#ifndef MIN
#  define MIN LB_MIN
#endif
#include <tclap/CmdLine.h>

#include <iostream>

namespace
{
co::ConnectionSet   _connectionSet;
lunchbox::a_int32_t _nClients;
lunchbox::Lock      _mutexPrint;
uint32_t _delay = 0;
enum
{
    SEQUENCE,
    DATA // must be last
};

class Receiver : public lunchbox::Thread
{
public:
    Receiver( const size_t packetSize, co::ConnectionPtr connection )
            : _connection( connection )
            , _mBytesSec( packetSize / 1024.f / 1024.f * 1000.f )
            , _nSamples( 0 )
            , _lastPacket( 0 )
        { 
            _buffer.resize( packetSize );
            connection->recvNB( _buffer.getData(), _buffer.getSize() );
        }

    virtual ~Receiver() {}

    bool readPacket()
        {
            if( !_connection->recvSync( 0, 0 ))
                return false;

            EQASSERTINFO( _lastPacket == 0 || _lastPacket - 1 ==
                          _buffer[ SEQUENCE ],
                          static_cast< int >( _lastPacket ) << ", " <<
                          static_cast< int >( _buffer[ SEQUENCE ] ));
            _lastPacket = _buffer[SEQUENCE];

            _connection->recvNB( _buffer.getData(), _buffer.getSize() );
            const float time = _clock.getTimef();
            ++_nSamples;

            const size_t probe = (_rng.get< size_t >() %
                                  ( _buffer.getSize() - DATA )) + DATA;
            EQASSERTINFO( _buffer[probe] == static_cast< uint8_t >( probe ),
                          (int)_buffer[probe] << " != " << (probe&0xff) );

            if( _delay > 0 )
                lunchbox::sleep( _delay );

            if( time < 1000.f )
                return true;

            _clock.reset();
            co::ConnectionDescriptionPtr desc = 
                _connection->getDescription();
            const lunchbox::ScopedMutex<> mutex( _mutexPrint );
            std::cerr << "Recv perf: " << _mBytesSec / time * _nSamples
                      << "MB/s (" << _nSamples / time * 1000.f  << "pps) from "
                      << desc->toString() << std::endl;
            _nSamples = 0;
            return true;
        }

    void executeReceive()
        {
            EQASSERT( _hasConnection == false );
            _hasConnection = true;
        }

    void stop()
        {
            EQASSERT( _hasConnection == false );
            _connection = 0;
            _hasConnection = true;
        }

    virtual void run()
        {
            _hasConnection.waitEQ( true );
            while( _connection.isValid( ))
            {
                if( !readPacket( )) // dead connection
                {
                    std::cerr << --_nClients << " clients" << std::endl;
                    _connectionSet.interrupt();
                    _connection = 0;
                    return;
                }

                _hasConnection = false;
                _connectionSet.addConnection( _connection );
                _hasConnection.waitEQ( true );
            }
        }
        
private:
    lunchbox::Clock _clock;
    lunchbox::RNG _rng;

    lunchbox::Bufferb _buffer;
    lunchbox::Monitor< bool > _hasConnection;
    co::ConnectionPtr _connection;
    const float _mBytesSec;
    size_t      _nSamples;
    uint8_t     _lastPacket;
};

class Selector : public lunchbox::Thread
{
public:
    Selector( co::ConnectionPtr connection, const size_t packetSize,
              const bool useThreads )
            : _connection( connection )
            , _packetSize( packetSize )
            , _useThreads( useThreads ) {}

    virtual bool init()
        {
            EQCHECK( _connection->listen( ));
            _connection->acceptNB();
            _connectionSet.addConnection( _connection );

            // Get first client
            const co::ConnectionSet::Event event = _connectionSet.select();
            EQASSERT( event == co::ConnectionSet::EVENT_CONNECT );

            co::ConnectionPtr resultConn = _connectionSet.getConnection();
            co::ConnectionPtr newConn    = resultConn->acceptSync();
            resultConn->acceptNB();
        
            EQASSERT( resultConn == _connection );
            EQASSERT( newConn.isValid( ));

            _receivers.push_back( RecvConn( new Receiver( _packetSize, newConn),
                                            newConn ));
            if( _useThreads )
                _receivers.back().first->start();

            _connectionSet.addConnection( newConn );
            // Until all client have disconnected...
            _nClients = 1;
            return true;
        }

    virtual void run()
        {
            co::ConnectionPtr resultConn;
            co::ConnectionPtr newConn;
            const bool multicast = _connection->getDescription()->type >=
                                   co::CONNECTIONTYPE_MULTICAST;

            while( _nClients > 0 )
            {
                switch( _connectionSet.select( )) // ...get next request
                {
                    case co::ConnectionSet::EVENT_CONNECT: // new client
                    
                        resultConn = _connectionSet.getConnection();
                        newConn = resultConn->acceptSync();
                        resultConn->acceptNB();

                        EQASSERT( newConn.isValid( ));

                        _receivers.push_back( 
                            RecvConn( new Receiver( _packetSize, newConn ),
                                      newConn ));
                        if( _useThreads )
                            _receivers.back().first->start();

                        _connectionSet.addConnection( newConn );
                        std::cerr << ++_nClients << " clients" << std::endl;
                        break;

                    case co::ConnectionSet::EVENT_DATA:  // new data
                    {
                        resultConn = _connectionSet.getConnection();
                        if( resultConn == _connection )
                        {
                            // really a close event of the listener
                            EQASSERT( resultConn->isClosed( ));
                            _connectionSet.removeConnection( resultConn );
                            std::cerr << "listener closed" << std::endl;
                            break;
                        }

                        Receiver* receiver = 0;
                        std::vector< RecvConn >::iterator i;
                        for( i = _receivers.begin(); i != _receivers.end(); ++i)
                        {
                            const RecvConn& candidate = *i;
                            if( candidate.second == resultConn )
                            {
                                receiver = candidate.first;
                                break;
                            }
                        }
                        EQASSERT( receiver );

                        if( _useThreads )
                        {
                            _connectionSet.removeConnection( resultConn );
                            receiver->executeReceive();
                        }
                        else if( !receiver->readPacket())
                        {
                            // Connection dead?
                            _connectionSet.removeConnection( resultConn );
                            delete receiver;
                            _receivers.erase( i );

                            if( multicast )
                                _connection->close();
                            std::cerr << --_nClients << " clients" << std::endl;
                        }
                        break;
                    }
                    case co::ConnectionSet::EVENT_DISCONNECT:
                    case co::ConnectionSet::EVENT_INVALID_HANDLE: // client done
                        resultConn = _connectionSet.getConnection();
                        _connectionSet.removeConnection( resultConn );

                        for( std::vector< RecvConn >::iterator i =
                                 _receivers.begin(); i !=_receivers.end(); ++i )
                        {
                            const RecvConn& candidate = *i;
                            if( candidate.second == resultConn )
                            {
                                Receiver* receiver = candidate.first;
                                _receivers.erase( i );
                                if( _useThreads )
                                {
                                    receiver->stop();
                                    receiver->join();
                                }
                                delete receiver;
                                break;
                            }
                        }

                        if( multicast )
                            _connection->close();
                        std::cerr << --_nClients << " clients" << std::endl;
                        break;

                    case co::ConnectionSet::EVENT_INTERRUPT:
                        break;

                    default:
                        EQASSERTINFO( false, "Not reachable" );
                }
            }
            EQASSERTINFO( _receivers.empty(), _receivers.size() );
            EQASSERTINFO( _connectionSet.getSize() <= 1,
                          _connectionSet.getSize( ));
            _connectionSet.clear();
        }

private:
    typedef std::pair< Receiver*, co::ConnectionPtr > RecvConn;
    co::ConnectionPtr    _connection;
    std::vector< RecvConn > _receivers;
    size_t _packetSize;
    bool _useThreads;
    lunchbox::Bufferb _buffer;
};

}

int main( int argc, char **argv )
{
    EQCHECK( co::init( argc, argv ));

    co::ConnectionDescriptionPtr description = new co::ConnectionDescription;
    description->type = co::CONNECTIONTYPE_TCPIP;
    description->port = 4242;

    bool isClient     = true;
    bool useThreads   = false;
    size_t packetSize = 1048576;
    size_t nPackets   = 0xffffffffu;
    uint32_t waitTime = 0;

    try // command line parsing
    {
        TCLAP::CmdLine command( "netPerf - Equalizer network benchmark tool\n");
        TCLAP::ValueArg< std::string > clientArg( "c", "client",
                                                  "run as client", true, "",
                                                  "IP[:port][:protocol]" );
        TCLAP::ValueArg< std::string > serverArg( "s", "server",
                                                  "run as server", true, "",
                                                  "IP[:port][:protocol]" );
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
        TCLAP::ValueArg<uint32_t> waitArg( "w", "wait", 
                                   "wait time (ms) between sends (client only)",
                                         false, 0, "unsigned", command );
        TCLAP::ValueArg<uint32_t> delayArg( "d", "delay", 
                                "wait time (ms) between receives (server only)",
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
        if( delayArg.isSet( ))
            _delay = delayArg.getValue();
    }
    catch( TCLAP::ArgException& exception )
    {
        EQERROR << "Command line parse error: " << exception.error() 
                << " for argument " << exception.argId() << std::endl;

        co::exit();
        return EXIT_FAILURE;
    }

    // run
    co::ConnectionPtr connection = co::Connection::create( description );
    if( !connection )
    {
        EQWARN << "Unsupported connection: " << description << std::endl;
        co::exit();
        return EXIT_FAILURE;
    }

    Selector* selector = 0;
    if( isClient )
    {
        if( description->type == co::CONNECTIONTYPE_RSP )
        {
            selector = new Selector( connection, packetSize, useThreads );
            selector->start();
        }
        else if( !connection->connect( ))
            ::exit( EXIT_FAILURE );

        lunchbox::Buffer< uint8_t > buffer;
        buffer.resize( packetSize );
        for( size_t i = 0; i<packetSize; ++i )
            buffer[i] = static_cast< uint8_t >( i );

        const float mBytesSec = buffer.getSize() / 1024.0f / 1024.0f * 1000.0f;
        lunchbox::Clock clock;
        size_t lastOutput = nPackets;

        clock.reset();
        while( nPackets-- )
        {
            buffer[SEQUENCE] = uint8_t( nPackets );
            EQCHECK( connection->send( buffer.getData(), buffer.getSize() ));
            const float time = clock.getTimef();
            if( time > 1000.f )
            {
                const lunchbox::ScopedMutex<> mutex( _mutexPrint );
                const size_t nSamples = lastOutput - nPackets;
                std::cerr << "Send perf: " << mBytesSec / time * nSamples 
                          << "MB/s (" << nSamples / time * 1000.f  << "pps)"
                          << std::endl;

                lastOutput = nPackets;
                clock.reset();
            }
            if( waitTime > 0 )
                lunchbox::sleep( waitTime );
        }
        const float time = clock.getTimef();
        const size_t nSamples = lastOutput - nPackets;
        if( nSamples != 0 )
        {
            const lunchbox::ScopedMutex<> mutex( _mutexPrint );
            std::cerr << "Send perf: " << mBytesSec / time * nSamples 
                      << "MB/s (" << nSamples / time * 1000.f  << "pps)"
                      << std::endl;
        }
        if ( selector )
        {
            connection->close();
            selector->join();
        }
    }
    else
    {
        selector = new Selector( connection, packetSize, useThreads );
        selector->start();
        
        EQASSERTINFO( connection->getRefCount()>=1, connection->getRefCount( ));
    
        if ( selector )
            selector->join();
    }

    
    delete selector;
    EQASSERTINFO( connection->getRefCount() == 1, connection->getRefCount( ));
    connection = 0;
    EQCHECK( co::exit( ));
    return EXIT_SUCCESS;
}

