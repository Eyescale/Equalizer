
#include <connection.h>

using namespace eqNet;

int main( int argc, char **argv )
{
    Connection *conn = new Connection();
    conn->listen( ":4242" );

    Connection *client = conn->accept();
    fprintf( stderr, "Server accepted connection\n" );
    conn->close();

    char c;
    while( client->read( &c, 1))
    {
        fprintf( stderr, "Server recv: %c\n", c );
        client->write( &c, 1 );
    }

    return 0;
}
