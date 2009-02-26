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
    description->type       = CONNECTIONTYPE_TCPIP;
    description->TCPIP.port = 4242;

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
        while( true )
        {
            TEST( connection->connect( ));
            if( waitTime > 0 )
                eq::base::sleep( waitTime );
            connection->close();

            ++nConnects;
            const float time = clock.getTimef();
            if( time > 1000.0f )
            {
                std::cout << nConnects / time * 1000.f << " connects/s "
                          << std::endl;
                nConnects = 0;
                clock.reset();
            }
        }
    }
    else
    {
        TEST( connection->listen( ));

        ConnectionSet connectionSet;
        connectionSet.addConnection( connection );

        ConnectionPtr resultConn;
        while( true )
        {
            switch( connectionSet.select( )) // ...get next request
            {
                case ConnectionSet::EVENT_CONNECT: // new client
                    resultConn = connectionSet.getConnection();
                    TEST( resultConn == connection );

                    resultConn = resultConn->accept();
                    TEST( resultConn.isValid( ));
                    
                    connectionSet.addConnection( resultConn );
                    ++nConnects;
                    break;

                case ConnectionSet::EVENT_DISCONNECT:
                case ConnectionSet::EVENT_INVALID_HANDLE:  // client done
                    resultConn = connectionSet.getConnection();
                    connectionSet.removeConnection( resultConn );
                    break;

                case ConnectionSet::EVENT_INTERRUPT:
                    break;

                default:
                    TESTINFO( false, "Not reachable" );
            }

            const float time = clock.getTimef();
            if( time > 1000.0f )
            {
                std::cout << nConnects / time * 1000.f << " accepts/s ("
                          << connectionSet.size() << " connections open)"
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
