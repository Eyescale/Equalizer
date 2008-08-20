
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
    description->TCPIP.port = 0;

    ConnectionPtr            connection = Connection::create( description );
    TEST( connection->listen( ));

    RefPtr<Connection> client = connection->accept();
    cerr << "Server accepted connection" << endl;
    connection->close();

    char c;
    while( client->recv( &c, 1 ))
    {
        cerr << "Server recv: " << c << endl;
        TEST( client->send( &c, 1 ) == 1 );
    }

    return EXIT_SUCCESS;
}
