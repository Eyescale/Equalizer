
#include <test.h>
#include <eq/net/connection.h>
#include <eq/net/connectionDescription.h>
#include <eq/net/init.h>

#include <iostream>

using namespace eq::net;
using namespace eq::base;
using namespace std;

int main( int argc, char **argv )
{
    eq::net::init( argc, argv );

    ConnectionDescriptionPtr description = new ConnectionDescription;
    description->type = CONNECTIONTYPE_TCPIP;
    description->setHostname( "localhost" );
    description->TCPIP.port = 4242;

    ConnectionPtr            connection = Connection::create( description );
    TEST( connection->connect( ));

    const char     message[] = "buh!";
    const uint64_t nChars    = strlen( message ) + 1;
    char*          response  = static_cast<char*>( alloca( nChars ));

    TEST( connection->send( message, nChars ) == nChars );
    TEST( connection->recv( response, nChars ) == nChars );
    cerr << "Client recv: " << response << endl;
    connection->close();

    return EXIT_SUCCESS;
}
