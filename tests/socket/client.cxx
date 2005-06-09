
#include <connection.h>

using namespace eqNet;

int main( int argc, char **argv )
{
    Connection *conn = new Connection();
    conn->connect( "localhost:22" );

    char c;
    while( conn->read( &c, 1))
        fprintf( stderr, "%c", c );

    return 0;
}
