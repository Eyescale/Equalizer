// Tests network throughput
// Usage: see 'netPerf -h'

#include <pthread.h> // must come first!

#include <test.h>
#include <eq/base/monitor.h>
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
    vector< Receiver* > _receivers;
    eq::base::Lock      _freeThreadsLock;
    ConnectionSet       _connectionSet;
    eq::base::Atomic< long > _nClients;

    class Receiver : public eq::base::Thread
    {
    public:
        Receiver( const size_t packetSize ) { _buffer.resize( packetSize ); }
        virtual ~Receiver() {}

        bool readPacket( ConnectionPtr connection )
        {
            const float mBytesSec = _buffer.size / 1024.0f / 1024.0f * 1000.0f;

            _clock.reset();
            if( !connection->recv( _buffer.data, _buffer.size ))
                return false;

            const float time = _clock.getTimef();
            eq::net::ConnectionDescriptionPtr desc = 
                connection->getDescription();

            EQWARN << " Recv perf: " << mBytesSec / time << "MB/s ("
                   << time << "ms) from " << desc->getHostname() << ":"
                   << desc->TCPIP.port << endl;
            return true;
        }

        void executeReceive( ConnectionPtr connection )
        {
            TEST( _hasConnection == false );
            TEST( !_connection );

            _connection    = connection;
            _hasConnection = true;
        }

        virtual void* run()
        {
            while( true )
            {
                _hasConnection.waitEQ( true );

                ConnectionPtr connection = _connection;
                _connection    = 0;
                _hasConnection = false;

                if( readPacket( connection ))
                {
                    _connectionSet.addConnection( connection );

                    _freeThreadsLock.set();
                    _receivers.push_back( this );
                    _freeThreadsLock.unset();
                }
                else // dead connection
                    --_nClients;
            }
            return EXIT_SUCCESS;
        }

    private:
        Clock _clock;

        eq::base::Buffer< uint8_t > _buffer;
        eq::base::Monitor< bool >   _hasConnection;
        ConnectionPtr               _connection;
    };
}

int main( int argc, char **argv )
{
    TEST( eq::net::init( argc, argv ));

    ConnectionPtr connection = Connection::create( CONNECTIONTYPE_TCPIP );
    ConnectionDescriptionPtr connDesc = connection->getDescription();
    connDesc->TCPIP.port = 4242;

    bool isClient     = true;
    bool useThreads   = false;
    size_t packetSize = 1048576;

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
                                         false, 1048576, "unsigned", command );

        command.xorAdd( clientArg, serverArg );
        command.parse( argc, argv );

        if( clientArg.isSet( ))
            connDesc->fromString( clientArg.getValue( ));
        else if( serverArg.isSet( ))
        {
            isClient = false;
            connDesc->fromString( serverArg.getValue( ));
        }

        useThreads = threadedArg.isSet();

        if( sizeArg.isSet( ))
            packetSize = sizeArg.getValue();
    }
    catch( TCLAP::ArgException& exception )
    {
        EQERROR << "Command line parse error: " << exception.error() 
            << " for argument " << exception.argId() << endl;

        eq::net::exit();
        return EXIT_FAILURE;
    }

    // run
    if( isClient )
    {
        TEST( connection->connect( ));

        eq::base::Buffer< uint8_t > buffer;
        buffer.resize( packetSize );

        const float mBytesSec = buffer.size / 1024.0f / 1024.0f * 1000.0f;
        Clock       clock;
        size_t      packetNum = 0;

        while( true )
        {
            clock.reset();
            TEST( connection->send( buffer.data, buffer.size ));
            const float time = clock.getTimef();
            cout << ++packetNum << " Send perf: " << mBytesSec / time 
                 << "MB/s (" << time << "ms)" << endl;
        }
    }
    else
    {
        TEST( connection->listen( ));

        _connectionSet.addConnection( connection );

        // Get first client
        const ConnectionSet::Event event = _connectionSet.select();
        TEST( event == ConnectionSet::EVENT_CONNECT );

        ConnectionPtr resultConn = _connectionSet.getConnection();
        ConnectionPtr newConn    = resultConn->accept();
        TEST( resultConn == connection );
        TEST( newConn.isValid( ));

        _connectionSet.addConnection( newConn );

        if( !useThreads )
            _receivers.push_back( new Receiver( packetSize ));

        // Until all client have disconnected...
        _nClients = 1;
        while( _nClients > 0 )
        {
            switch( _connectionSet.select( )) // ...get next request
            {
                case ConnectionSet::EVENT_CONNECT: // new client
                    resultConn = _connectionSet.getConnection();
                    TEST( resultConn == connection );
                    newConn    = resultConn->accept();
                    TEST( newConn.isValid( ));

                    _connectionSet.addConnection( newConn );
                    ++_nClients;
                    break;

                case ConnectionSet::EVENT_DATA:  // new data
                    resultConn = _connectionSet.getConnection();
                    if( useThreads )
                    {
                        Receiver* receiver = 0;
                        _connectionSet.removeConnection( resultConn );

                        _freeThreadsLock.set();
                        if( _receivers.empty( ))
                        {
                            receiver = new Receiver( packetSize );
                            receiver->start();
                        }
                        else
                        {
                            receiver = _receivers.back();
                            _receivers.pop_back();
                        }
                        _freeThreadsLock.unset();    

                        receiver->executeReceive( resultConn );
                    }
                    else if( !_receivers[0]->readPacket( resultConn ))
                    {
                        // Connection dead?
                        _connectionSet.removeConnection( resultConn );
                        --_nClients;
                    }
                    break;

                case ConnectionSet::EVENT_DISCONNECT:
                case ConnectionSet::EVENT_INVALID_HANDLE:  // client done
                    resultConn = _connectionSet.getConnection();
                    _connectionSet.removeConnection( resultConn );
                    --_nClients;
                    break;

                case ConnectionSet::EVENT_INTERRUPT:
                    break;

                default:
                    TESTINFO( false, "Not reachable" );
            }
        }
    }

    // TODO thread tear down

    TESTINFO( connection->getRefCount() == 1, connection->getRefCount( ));
    connection->close();
    return EXIT_SUCCESS;
}
